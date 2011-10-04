/*
* Copyright (C) 2010-2011 Toni Gundogdu.
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

#include "config.h"

#include <QMainWindow>
#include <QCloseEvent>
#include <QFileDialog>
#include <QRegExp>
#ifdef _0
#include <QDebug>
#endif

#include <NAboutDialog>
#include <NFeedDialog>
#include <NLang>

#include "util.h"
#include "Recent.h"
// UI
#include "Preferences.h"
#include "MainWindow.h"

#define QSETTINGS_GROUP             "MainWindow"

// main.cpp

extern bool have_quvi_feature_query_formats;
extern SharedPreferences shPrefs;
extern Recent recent;

// Modes.

enum { StreamVideo=0, DownloadVideo };

// Ctor.

MainWindow::MainWindow()
{
  setupUi (this);

  readSettings ();

  // Stay on top?

  if (shPrefs.get (SharedPreferences::StayOnTop).toBool ())
    setWindowFlags (windowFlags () | Qt::WindowStaysOnTopHint);

  // Create Video instance.

  video  = new Video;

  // Create context menu.

  createContextMenu ();

  // Create system tray icon.

  createTray ();

  // Create Download dialog.

  download = new DownloadDialog (this);

  // Create Process Progress dialog for quvi.

  proc = new ProcessProgressDialog (this);

  connect (proc, SIGNAL (finished (QString)),
           this, SLOT (onProcFinished (QString)));

  QHash<QString,QRegExp> h;

#define _wrap(s,r) \
    do { h[s] = QRegExp(r); } while (0)

  _wrap(tr("Checking..."),    "^:: Check");
  _wrap(tr("Fetching..."),    "^:: Fetch");
  _wrap(tr("Verifying..."),   "^:: Verify");

#undef _wrap

  proc->setLabelRegExp (h);

  // Custom program icon.

  if (shPrefs.get (SharedPreferences::CustomProgramIcon).toBool ())
    changeProgramIcon ();

  NomNom::convert_old_umph_path(this);
  NomNom::check_for_umph_feature("--all");
}

void MainWindow::createContextMenu()
{
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

  creat_a (tr("Address..."),  onAddress,  false);
  creat_a (tr("Feed..."),     onFeed,     false);
  creat_a (tr("Recent..."),   onRecent,   false);
  add_s;
  creat_a (tr("Preferences..."), onPreferences, false);
  add_s;
  creat_a (tr("About..."),    onAbout,    false);
  creat_a (tr("Quit"),        close,      false);

#undef add_s
#undef creat_a

  // Add key shortcuts.

#define _wrap(s,k) \
    do { actions[s]->setShortcut (QKeySequence(k)); } while (0)

  _wrap (tr("Address..."),    "Ctrl+A");
  _wrap (tr("Feed..."),       "Ctrl+F");
  _wrap (tr("Recent..."),     "Ctrl+R");
  // --
  _wrap (tr("Preferences..."),"Ctrl+E");
  // --
  _wrap (tr("Quit"),          "Ctrl+Q");
#undef _wrap

  // Add the context menu.

  textBrowser->setContextMenuPolicy (Qt::ActionsContextMenu);
}

// Create tray icon.

void MainWindow::createTray()
{
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

  trayIcon = new QSystemTrayIcon (this);
  trayIcon->setContextMenu (trayMenu);
  trayIcon->setIcon (QIcon (":img/nomnom.png"));

  connect(trayIcon, SIGNAL( activated(QSystemTrayIcon::ActivationReason) ),
          this, SLOT( onTrayActivated(QSystemTrayIcon::ActivationReason) ) );
}

// Show event.

void MainWindow::showEvent(QShowEvent *e)
{
  trayIcon->hide();
  e->accept();
}

// Hide event.

void MainWindow::hideEvent(QHideEvent *e)
{
  if (QSystemTrayIcon::isSystemTrayAvailable()
      && shPrefs.get(SharedPreferences::MinToTray).toBool())
    {
      trayIcon->show();
      if (!isMinimized())
        hide();
    }
  e->accept();
}

// Close event.

void MainWindow::closeEvent(QCloseEvent *e)
{
  QSettings s;
  NomNom::save_size (s, this, QSETTINGS_GROUP);
  NomNom::save_pos (s, this, QSETTINGS_GROUP);

  s.beginGroup (QSETTINGS_GROUP);
  s.setValue ("modeCBox", modeCBox->currentIndex ());
  s.endGroup ();

  recent.write ();

  e->accept ();
}

// Read.

void MainWindow::readSettings()
{
  QSettings s;

  NomNom::restore_size(s, this, QSETTINGS_GROUP, QSize(130,150));
  NomNom::restore_pos(s, this, QSETTINGS_GROUP);

  s.beginGroup(QSETTINGS_GROUP);
  modeCBox->setCurrentIndex(s.value("modeCBox").toInt());
  s.endGroup();

  shPrefs.read();
}

// Handle (dropped) URL.

void MainWindow::handleURL(const QString& url)
{
  // Check paths.

  const QString quviPath =
    shPrefs.get (SharedPreferences::QuviPath).toString ().simplified ();

  if (quviPath.isEmpty ())
    {
      NomNom::crit (this, tr ("You must specify path to the quvi command."));
      onPreferences ();
      return;
    }

  NomNom::check_query_formats(quviPath);

  const QString playerPath =
    shPrefs.get (SharedPreferences::PlayerPath).toString ().simplified ();

  if (playerPath.isEmpty ())
    {

      NomNom::crit(this,
                   tr ("You must specify path to a stream-capable media "
                       "player command."));
      onPreferences();
      return;
    }

  // Recent.

  recent << url;

  // quvi args.

  QStringList args =
    quviPath.split (" ").replaceInStrings ("%u", url);

  // Choose format.
#ifdef _0
  qDebug() << __PRETTY_FUNCTION__ << have_quvi_feature_query_formats;
#endif

  QStringList formats;
  if (have_quvi_feature_query_formats)
    {
      bool failed = false;

      if (!queryFormats(formats, quviPath, url, failed))
        formats.clear();

      if (failed)
        {
          NomNom::crit(this, proc->errmsg());
          return;
        }

      if (proc->canceled())
        return;
    }

  json.clear();

  QString fmt;
  if (!selectFormat(formats, fmt))
    return;

  args << "-f" << fmt;

  proc->setLabelText(tr ("Checking ..."));
  proc->setMaximum(0);
  proc->start(args);

  if (proc->canceled())
    return;

  QString errmsg;
  if (parseOK(errmsg))
    {
      if (modeCBox->currentIndex() == StreamVideo)
        streamVideo();
      else
        downloadVideo();
    }
  else
    NomNom::crit(this, errmsg);
}

bool MainWindow::queryFormats(QStringList& formats,
                              const QString& quviPath,
                              const QString& url,
                              bool& failed)
{
  QStringList args =
    QStringList()
    << quviPath.split(" ").replaceInStrings("%u", url)
    << "-F";

  json.clear();

  proc->setLabelText(tr("Checking ..."));
  proc->setMaximum(0);
  proc->start(args);
#ifdef _0
  qDebug() << __PRETTY_FUNCTION__;
#endif

  failed = proc->failed();
  if (failed)
    return false;

#ifdef _0
  qDebug() <<  __PRETTY_FUNCTION__ << __LINE__ << failed;
#endif

  QStringList lns = json.split("\n");
  lns.removeDuplicates();

  const QRegExp rx("^(.*)\\s+:\\s+");
  foreach (const QString s, lns)
  {
    if (rx.indexIn(s) != -1)
      {
        formats = QStringList()
                  << "default"
                  << "best"
                  << rx.cap(1).simplified().split("|");
#ifdef _0
        qDebug() << __PRETTY_FUNCTION__ << formats;
#endif
        return true;
      }
  }
  return false;
}

bool MainWindow::selectFormat(QStringList& formats, QString& fmt)
{
  // Prompt only if count exceeds 1 ("default)".
  if (formats.size() < 2)
    {
      fmt = "default"; // Do not translate. quvi understands only this.
      return true;
    }

  bool ok = false;

  fmt = QInputDialog::getItem (
          this,
          tr ("Choose format"),
          tr ("Format:"),
          formats << tr("Enter your own"),
          0,
          false,
          &ok
        );
  if (!ok)
    return false; // Cancel

  if (fmt == tr("Enter your own"))
    {
      fmt = QInputDialog::getText(
              this,
              tr("Enter format"),
              tr("Format:"),
              QLineEdit::Normal,
              "default",
              &ok
            );
    }
  return ok && !fmt.isEmpty();
}

// View video (stream).

void MainWindow::streamVideo()
{
  QStringList args =
    shPrefs.get (SharedPreferences::PlayerPath)
    .toString ().simplified ().split (" ");

  args = args.replaceInStrings ("%u", video->get (Video::Link).toString ());

  const bool ok = QProcess::startDetached (args.takeFirst (), args);

  if (!ok)
    {
      NomNom::crit (this,
                    tr ("Unable to start player command, check the Preferences."));
    }
}

// Download video (to a file).

void MainWindow::downloadVideo()
{
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

  const bool dont_prompt =
    shPrefs.get (SharedPreferences::DontPromptFilename).toBool ();

  if (!dont_prompt)
    {

      const QFileDialog::Options opts =
        QFileDialog::DontConfirmOverwrite;

      fpath = QFileDialog::getSaveFileName (
                this,
                tr ("Save video as"),
                fpath,
                suffix, // Filter.
                0,      // Selected filter.
                opts
              );

      if (fpath.isEmpty ())
        return;
    }

  const qint64 expected_bytes =
    video->get (Video::Length).toLongLong ();

  if (QFileInfo (fpath).size () < expected_bytes)
    {

      const QString cmd =
        shPrefs.get (SharedPreferences::CurlPath).toString ().simplified ();

      download->start (cmd, fpath, video);
    }

  const bool completeFile =
    QFileInfo (fpath).size () >= expected_bytes;

  const bool playWhenDone =
    shPrefs.get (SharedPreferences::PlayWhenDone).toBool ();

#ifdef _0
  qDebug ()
      << QFileInfo (fpath).size ()
      << expected_bytes
      << completeFile
      << playWhenDone;
#endif

  if (completeFile && playWhenDone)
    {

      QStringList args =
        shPrefs.get (SharedPreferences::PlayerPath)
        .toString ().simplified ().split (" ");

      args = args.replaceInStrings ("%u", fpath);

      const bool ok = QProcess::startDetached (args.takeFirst (), args);

      if (!ok)
        {
          NomNom::crit (this, tr ("Unable to start player command, "
                                  "check the Preferences."));
        }

    }

}

// Change program icon.

void MainWindow::changeProgramIcon()
{
  const bool customProgramIcon =
    shPrefs.get (SharedPreferences::CustomProgramIcon).toBool ();

  QString html = textBrowser->toHtml ();

  const QString iconPath =
    customProgramIcon
    ? shPrefs.get (SharedPreferences::ProgramIconPath).toString ()
    : ":img/nomnom.png";

  html.replace (QRegExp ("img src=\".*\""), "img src=\"" +iconPath+ "\"");

  textBrowser->setHtml (html);

  QIcon icon = QIcon (iconPath);

  setWindowIcon     (icon);
  trayIcon->setIcon (icon);
}

// Parse JSON data returned by quvi.

bool MainWindow::parseOK(QString& errmsg)
{
  if (proc->failed())
    {
      errmsg = proc->errmsg();
      return false;
    }

  const int n = json.indexOf("{");
  if (n == -1)
    {
      errmsg = tr("quvi returned unexpected data");
      return false;
    }

  return video->fromJSON(json.mid(n), errmsg);
}

// Slot: Icon activated.

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason r)
{
  switch (r)
    {
    case QSystemTrayIcon::Trigger:
      showNormal();
      break;
    default:
      break;
    }
}

// Slot: Preferences.

void MainWindow::onPreferences()
{
  const bool icon_state =
    shPrefs.get (SharedPreferences::CustomProgramIcon).toBool ();

  const QString icon_path =
    shPrefs.get (SharedPreferences::ProgramIconPath).toString ();

  Preferences *d = new Preferences(this);

  if (d->exec() == QDialog::Accepted)
    {
      NomNom::convert_old_umph_path(this);
      NomNom::check_for_umph_feature("--all");

      // User requested app restart.

      if (d->restartAfter())
        {
          close ();
          QProcess::startDetached (QCoreApplication::applicationFilePath ());
          return;
        }

      Qt::WindowFlags flags = windowFlags ();

      // Update window flags: Stay on top.

      if (shPrefs.get (SharedPreferences::StayOnTop).toBool () )
        {
          // Set flag.
          if (! (flags & Qt::WindowStaysOnTopHint) )
            flags |= Qt::WindowStaysOnTopHint;
        }
      else
        {
          // Remove flag.
          if (flags & Qt::WindowStaysOnTopHint)
            flags &= ~Qt::WindowStaysOnTopHint;
        }

      if (flags != windowFlags ())
        {
          setWindowFlags (flags);
          show ();
        }

      // Update program icon?

      if (icon_state !=
          shPrefs.get (SharedPreferences::CustomProgramIcon).toBool ()
          || icon_path !=
          shPrefs.get (SharedPreferences::ProgramIconPath).toString ())
        {
          changeProgramIcon ();
        }
    }
}

// Slot: About.

#define WWW "http://nomnom.sourceforge.net/"

void MainWindow::onAbout()
{
  nn::NAboutDialog *d = new nn::NAboutDialog(VERSION, WWW, this);
  d->exec();
}

#undef WWW

// Slot: Recent.

void MainWindow::onRecent()
{
  const QStringList lst = recent.toStringList ();

  if (lst.size () == 0)
    {
      NomNom::info (this, tr ("No record of recently visited URLs found."));
      return;
    }

  bool ok = false;

  const QString s = QInputDialog::getItem (
                      this,
                      tr ("Recent URLs"),
                      tr ("Select URL (most recent first):"),
                      lst,
                      0,
                      false,
                      &ok
                    );

  if (!ok)
    return;

  handleURL (s);
}

// Slot: Download.

void MainWindow::onAddress()
{
  const QString url =
    QInputDialog::getText (this, tr ("Address"), tr ("Video URL:"));

  if (url.isEmpty ())
    return;

  handleURL (url);
}

// Slot: on feed.

void MainWindow::onFeed()
{
  const QString path =
    shPrefs.get(SharedPreferences::UmphPath).toString();

  nn::NFeedDialog *d = new nn::NFeedDialog(this, path);
  if (d->exec() == QDialog::Accepted)
    handleURL(d->selected());
}

// Slot: quvi finished.

void MainWindow::onProcFinished(QString output)
{
  json = output;
}

// Event: DragEnter.

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
  QUrl url(e->mimeData()->text());
  if (url.isValid() && url.scheme().toLower() == "http")
    e->acceptProposedAction();
}

// Event: Drop.

void MainWindow::dropEvent(QDropEvent *e)
{
  handleURL (e->mimeData ()->text ().simplified ());
  e->acceptProposedAction ();
}

// vim: set ts=2 sw=2 tw=72 expandtab:
