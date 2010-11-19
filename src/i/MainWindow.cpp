/* 
* Copyright (C) 2010 Toni Gundogdu.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QMainWindow>
#include <QCloseEvent>
#include <QFileDialog>
#include <QRegExp>
#include <QDebug>

#include "util.h"
#include "Log.h"
#include "History.h"
// UI
#include "About.h"
#include "Preferences.h"
#include "LogView.h"
#include "Reminder.h"
#include "YoutubeFeed.h"
#include "MainWindow.h"

#define QSETTINGS_GROUP             "MainWindow"
#define QSETTINGS_REMINDER_SHOWTIPS "showTips"

// main.cpp

extern QMap<QString,QStringList> hosts;
extern SharedPreferences shPrefs;
extern History history;
extern Log log;

// Modes.

enum { StreamVideo=0, DownloadVideo };

// Ctor.

MainWindow::MainWindow  ()
    : canceled (false)
{
    setupUi(this);

    readSettings();

#define _wrap(sig,slot) \
    do { \
        connect(&proc, SIGNAL(sig), this, SLOT(slot)); \
    } while (0)

    _wrap(started(), onQuviStarted());

    _wrap(error(QProcess::ProcessError),
        onQuviError(QProcess::ProcessError));

    _wrap(readyRead(), onQuviReadyRead());

    _wrap(finished(int,QProcess::ExitStatus),
        onQuviFinished(int,QProcess::ExitStatus));

#undef _wrap

    // Stay on top?

    if (shPrefs.get(SharedPreferences::StayOnTop).toBool())
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    // Create Video instance.

    video  = new Video;

    // Create context menu.

    createContextMenu();

    // Create system tray icon.

    createTray();

    // Create Download dialog.

    download = new Download (this);
#ifdef _0
    download->setLabelText (tr ("Copying..."));
#endif
    download->setCancelButtonText (tr ("&Abort"));

    connect (download, SIGNAL (error (QString)),
        this, SLOT (onDownloadError (QString)));

    proc.setProcessChannelMode (QProcess::MergedChannels);

    log << tr ("Program started.") + "\n";
}

void
MainWindow::createContextMenu () {

#define creat_a(t,f,c) \
    do { \
        QAction *a = new QAction (t, textBrowser); \
        if (c) { \
            a->setCheckable (c); \
            /*connect (a, SIGNAL(toggled (bool)), SLOT(f (bool)));*/ \
        } \
        else \
            connect (a, SIGNAL(triggered ()), SLOT(f ())); \
        textBrowser->addAction (a); \
        actions[t] = a; \
    } while (0)

#define add_s \
    do { \
        QAction *a = new QAction (textBrowser); \
        a->setSeparator (true); \
        textBrowser->addAction (a); \
    } while (0)

    creat_a (tr("Address..."), onAddress,   false);
    creat_a (tr("Feed..."), onFeed, false);
#ifdef _0
    creat_a (tr("Rake..."), onRake, false);
#endif
    creat_a (tr("History..."), onHistory, false);
    add_s;
    creat_a (tr("Overwrite"), _,      true);
    creat_a (tr("Stop"),      onStop, false);
    add_s;
    creat_a (tr("Log..."),         onLog,         false);
    creat_a (tr("Preferences..."), onPreferences, false);
    add_s;
    creat_a (tr("About..."), onAbout, false);
    creat_a (tr("Quit"),     close,   false );

#undef add_s
#undef creat_a

    // Add key shortcuts.

    actions[tr("Address...")]->setShortcut  (QKeySequence(tr("Ctrl+A")));
    actions[tr("Feed...")]->setShortcut (QKeySequence(tr("Ctrl+F")));
#ifdef _0
    actions[tr("Rake...")]->setShortcut (QKeySequence(tr("Ctrl+C")));
#endif
    actions[tr("Overwrite")]->setShortcut(QKeySequence(tr("Ctrl+W")));
    actions[tr("Stop")]->setShortcut    (QKeySequence(tr("Ctrl+S")));
    actions[tr("Log...")]->setShortcut  (QKeySequence(tr("Ctrl+L")));
    actions[tr("History...")]->setShortcut     (QKeySequence(tr("Ctrl+R")));
    actions[tr("Preferences...")]->setShortcut (QKeySequence(tr("Ctrl+E")));
    actions[tr("Quit")]->setShortcut    (QKeySequence(tr("Ctrl+Q")));

    // Add the context menu.

    textBrowser->setContextMenuPolicy (Qt::ActionsContextMenu);
}

// Create tray icon.

void
MainWindow::createTray () {

    trayMenu = new QMenu(this);

#define creat_a(t,f) \
    do { \
        QAction *a = new QAction(t, this); \
        connect(a, SIGNAL(triggered()), this, SLOT(f)); \
        actions[t] = a; \
        trayMenu->addAction(a); \
    } while (0)

    creat_a ( tr("Open"), showNormal() );
    trayMenu->addSeparator();
    creat_a ( tr("Quit"), close() );

#undef creat_a

    trayIcon = new QSystemTrayIcon(this);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->setIcon(QIcon(":img/nomnom.png"));

    connect(trayIcon, SIGNAL( activated(QSystemTrayIcon::ActivationReason) ),
        this, SLOT( onTrayActivated(QSystemTrayIcon::ActivationReason) ) );
}

// Show event.

void
MainWindow::showEvent (QShowEvent *e) {
    trayIcon->hide();
    e->accept();
}

// Hide event.

void
MainWindow::hideEvent (QHideEvent *e) {

    if (QSystemTrayIcon::isSystemTrayAvailable()
        && shPrefs.get(SharedPreferences::MinToTray).toBool())
    {
        trayIcon->show();
        if (!isMinimized())
            hide();
    }
    e->accept();
}

static void
still_running (QWidget *p) {
    NomNom::crit (p,
        QObject::tr ("Stop the running quvi(1) process first, and try again."));
}

// Close event.

void
MainWindow::closeEvent (QCloseEvent *e) {

    int rc = QMessageBox::Yes;

    if (proc.state() != QProcess::NotRunning) {
        rc = NomNom::ask(
            this,
            tr("quvi is still running, really quit?")
        );
    }

    if (rc == QMessageBox::Yes)
        proc.kill();
    else {
        e->ignore();
        return;
    }

    QSettings s;
    NomNom::save_size(s, this, QSETTINGS_GROUP);
    NomNom::save_pos(s, this, QSETTINGS_GROUP);

    s.beginGroup(QSETTINGS_GROUP);
    s.setValue("modeCBox", modeCBox->currentIndex());
    s.endGroup();

    history.write();

    e->accept();
}

// Read.

void
MainWindow::readSettings () {

    QSettings s;

    NomNom::restore_size(s, this, QSETTINGS_GROUP, QSize(130,150));
    NomNom::restore_pos(s, this, QSETTINGS_GROUP);

    s.beginGroup(QSETTINGS_GROUP);
    modeCBox->setCurrentIndex(s.value("modeCBox").toInt());
    s.endGroup();

    Reminder (this, "SharedPreferences").conditionalExec ();

    // Re-read (now updated) "showReminder" value. This was previously done
    // in main.cpp, but has to be done here if we expect the Preferences
    // "show tips" box value to match the one set in the Reminder dialog.

    shPrefs.read ();
}

// Handle (dropped) URL.

bool
MainWindow::handleURL (const QString& url) {

    if (proc.state() != QProcess::NotRunning) {
        still_running (this);
        return false;
    }

    // Check paths.

    const QString quviPath =
        shPrefs.get (SharedPreferences::QuviPath).toString ().simplified ();

    if (quviPath.isEmpty ()) {

        NomNom::crit (this, tr ("You must specify path to the quvi command."));

        onPreferences ();

        return false;
    }

    if (hosts.isEmpty ()) {
        const bool ok = NomNom::parse_quvi_support (this, quviPath);
        if (!ok) return false;
    }

    const QString playerPath =
        shPrefs.get (SharedPreferences::PlayerPath).toString ().simplified ();

    if (playerPath.isEmpty ()) {

        NomNom::crit (this,
            tr("You must specify path to a stream-capable media "
                "player command."));

        onPreferences ();

        return false;
    }

    // History.

    history << url;

    // quvi args.

    QStringList args =
        quviPath.split (" ").replaceInStrings ("%u", url);

    // Choose format.

    QStringList formats;

    foreach (QString key, hosts.keys ()) {
        QRegExp re (key);
        if (re.indexIn (url) != -1) {
            formats = hosts.value (key);
            break;
        }
    }

    if (formats.size () > 1) { // Assume "default" is always present.
        bool ok = false;
        QString s = QInputDialog::getItem (
            this,
            tr ("Choose format"),
            tr ("Format:"),
            formats,
            0,
            false,
            &ok
        );
        if (ok)
            args << "-f" << s;
        else
            return false;
    }

    log << args.join (" ") + "\n";

    json.clear ();
    canceled = false;

    proc.start (args.takeFirst (), args);

    return true;
}

// View video (stream).

void
MainWindow::streamVideo () {

    QStringList args =
        shPrefs.get (SharedPreferences::PlayerPath)
            .toString ().simplified ().split (" ");

    args = args.replaceInStrings ("%u", video->get (Video::Link).toString ());

    log  << args.join(" ") + "\n";

    const bool ok = QProcess::startDetached (args.takeFirst (), args);

    if (!ok) {
        NomNom::crit (this,
            tr ("Unable to start player command, check the Preferences."));
    }
}

// Download video (to a file).

void
MainWindow::downloadVideo () {

    QString fname =
        shPrefs.get (SharedPreferences::FilenameFormat).toString ();

    const QString suffix =
        video->get (Video::Suffix).toString ().simplified ();

    bool ok = NomNom::format_filename (
        this,
        shPrefs.get (SharedPreferences::Regexp).toString (),
        video->get (Video::Title).toString ().simplified (),
        suffix,
        video->get (Video::Host).toString ().simplified (),
        video->get (Video::ID).toString ().simplified (),
        fname
    );

    if (!ok)
        return;

    QString fpath =
        shPrefs.get (SharedPreferences::SaveDir).toString ();

    if (fpath.isEmpty ())
        fpath = QDir::currentPath ();

    fpath = QDir::toNativeSeparators (fpath +"/"+ fname);

    QFileDialog::Options opts =
        QFileDialog::DontConfirmOverwrite;

    fpath = QFileDialog::getSaveFileName(
        this, 
        tr("Save video as"),
        fpath,
        suffix, // Filter.
        0,      // Selected filter.
        opts
    );

    if (fpath.isEmpty ())
        return;

    if (actions[tr("Overwrite")]->isChecked ())
        QDir ().remove (fpath);

    const qint64 expected_bytes =
        video->get (Video::Length).toLongLong ();

    if (QFileInfo (fpath).size () != expected_bytes) {

        const QString cmd =
            shPrefs.get (SharedPreferences::CurlPath).toString ().simplified ();

        download->start (cmd, fpath, video);
    }

    const bool completeFile =
        QFileInfo (fpath).size () == expected_bytes;

    const bool playWhenDone = 
        shPrefs.get (SharedPreferences::PlayWhenDone).toBool ();

    if (completeFile && playWhenDone) {

        QStringList args =
            shPrefs.get (SharedPreferences::PlayerPath)
                .toString ().simplified ().split (" ");

        args = args.replaceInStrings ("%u", fpath);

        log  << args.join(" ") + "\n";

        const bool ok = QProcess::startDetached (args.takeFirst (), args);

        if (!ok) {
            NomNom::crit (this,
                tr ("Unable to start player command, check the Preferences."));
        }

    }

}

// Parse JSON data returned by quvi.

bool
MainWindow::parseOK () {

    QString error = tr ("Could not find the JSON data in quvi output.");
    const int n   = json.indexOf ("{");

    if (n != -1) {
        if (video->fromJSON (json.mid (n), error))
            return true;
    }

    NomNom::crit (this, error);

    return false;
}

// Parse error from data returned by quvi.

void
MainWindow::parseError () {

    QRegExp re ("error:\\s+(.*)$");

    QString error = tr ("Unknown error");

    if (re.indexIn (json) != -1)
        error = re.cap (1);

    NomNom::crit (this, error.simplified ());
}

// Slot: Icon activated.

void
MainWindow::onTrayActivated (QSystemTrayIcon::ActivationReason r) {
    switch (r) {
    case QSystemTrayIcon::DoubleClick: showNormal(); break;
    default: break;
    }
}

// Slot: Preferences.

void
MainWindow::onPreferences () {

    Preferences dlg (this);

    if (dlg.exec () == QDialog::Accepted) {

        // User requested app restart.

        if (dlg.restartAfter ()) {
            close ();
            QProcess::startDetached (QCoreApplication::applicationFilePath ());
            return;
        }

        Qt::WindowFlags flags = windowFlags ();

        // Update window flags: Stay on top.

        if (shPrefs.get (SharedPreferences::StayOnTop).toBool () ) {
            // Set flag.
            if (! (flags & Qt::WindowStaysOnTopHint) )
                flags |= Qt::WindowStaysOnTopHint;
        }
        else {
            // Remove flag.
            if (flags & Qt::WindowStaysOnTopHint)
                flags &= ~Qt::WindowStaysOnTopHint;
        }

        if (flags != windowFlags ()) {
            setWindowFlags (flags);
            show ();
        }

    }

}

// Slot: About.

void
MainWindow::onAbout ()
    { About(this).exec(); }

// Slot: History.

void
MainWindow::onHistory () {

    const QStringList lst = history.toStringList();

    if (lst.size() == 0) {
        NomNom::info (this, tr ("No record of previously visited URLs found."));
        return;
    }

    if (proc.state() != QProcess::NotRunning) {
        still_running (this);
        return;
    }

    bool ok = false;

    const QString s = QInputDialog::getItem(
        this,
        tr("History"),
        tr("Visited URL:"),
        lst,
        0,
        false,
        &ok
    );

    if (!ok)
        return;

    handleURL(s);
}

// Slot: Log.

void
MainWindow::onLog ()
    { LogView (this).exec (); }

// Slot: Download.

void
MainWindow::onAddress() {

    if (proc.state() != QProcess::NotRunning) {
        still_running (this);
        return;
    }

    const QString url =
        QInputDialog::getText (this, tr ("Address"), tr ("Video URL:"));

    if (url.isEmpty ())
        return;

    handleURL (url);
}

// Slot: Stop quvi process.

void
MainWindow::onStop () {
    if (proc.state() != QProcess::NotRunning) {
        canceled = true;
        proc.kill();
    }
}

// main.cpp
extern QHash<QString,QString> feed;

// Slot: on feed.

void
MainWindow::onFeed () {

    const QString path =
        shPrefs.get (SharedPreferences::UmphPath).toString ();

    if (path.isEmpty ()) {
        NomNom::crit (this,
            tr ("Specify path to the umph(1) command in the Preferences."));
        return;
    }

    if (proc.state() != QProcess::NotRunning) {
        still_running (this);
        return;
    }

    QString url;

    if (!feed.empty ()) {

        const int rc = NomNom::ask (this, tr ("Choose from old results?"));

        if (rc == QMessageBox::Yes) {
            if (!NomNom::choose_from_feed (this, url))
                return;
        }

    }

    if (url.isEmpty ()) {

        YoutubeFeed d (this);

        if (d.exec () != QDialog::Accepted || !d.gotItems ())
            return;

        if (!NomNom::choose_from_feed (this, url))
            return;

    }

    handleURL (url);
}

// Slot: on scan.

void
MainWindow::onRake () {
}

// Slot: Process started.

void
MainWindow::onQuviStarted ()
    { statusLabel->setText (tr ("Starting parser ...")); }

// Slot: Process error.

void
MainWindow::onQuviError (QProcess::ProcessError n) {
    if (!canceled)
        NomNom::crit (this, NomNom::to_process_errmsg (n));
}

// Slot: Ready read. Note that we use merged channels (stdout + stderr).

void
MainWindow::onQuviReadyRead () {

    static char data[1024];

    while (proc.readLine (data, sizeof(data))) {

        QString ln = QString::fromLocal8Bit (data);

        log << ln;
        json += ln;
        ln.remove ("\n");

        if (ln.startsWith (":: Check"))
            statusLabel->setText (tr ("Checking..."));

        else if (ln.startsWith (":: Fetch"))
            statusLabel->setText (tr ("Fetching..."));

        else if (ln.startsWith (":: Verify"))
            statusLabel->setText (tr ("Verifying..."));

        // Leave JSON and error parsing to finished slot.
    }
}

// Slot: Process finished.

void
MainWindow::onQuviFinished (int exitCode, QProcess::ExitStatus exitStatus) {

    statusLabel->setText (tr ("Ready."));

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        if (parseOK ()) {
            if (modeCBox->currentIndex() == StreamVideo)
                streamVideo();
            else
                downloadVideo();
        }
    }
    else {
        if (!canceled)
            parseError  ();
    }
}

// Slot: download error.

void
MainWindow::onDownloadError (QString e)
    { NomNom::crit (this, e); }

// Event: DragEnter.

void
MainWindow::dragEnterEvent (QDragEnterEvent *e) {

    QUrl url(e->mimeData()->text());

    if (url.isValid() && url.scheme().toLower() == "http")
        e->acceptProposedAction();
}

// Event: Drop.

void
MainWindow::dropEvent (QDropEvent *e) {

    const QString url = e->mimeData()->text().simplified();

    if (handleURL(url))
        e->acceptProposedAction();
    else
        e->ignore();
}


