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

#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include "util.h"
#include "History.h"
#include "Preferences.h"

#define QSETTINGS_GROUP "Preferences"

// main.cpp

extern SharedPreferences shPrefs;

// Ctor.

Preferences::Preferences (QWidget *parent)
    : QDialog (parent), _restartAfter (false)
{
    setupUi (this);

    QSettings s;
    NomNom::restore_size (s, this, QSETTINGS_GROUP, QSize (600,200));

    configEdit->setText (s.fileName ());

    quviPathEdit->setText (
        shPrefs.get (SharedPreferences::QuviPath).toString ());

    if (quviPathEdit->text ().isEmpty ())
        quviPathEdit->setText ("quvi %u");

    curlPathEdit->setText (
        shPrefs.get (SharedPreferences::CurlPath).toString ());

    if (curlPathEdit->text ().isEmpty ())
        curlPathEdit->setText ("curl -L -C - -o %f %u");

    playerPathEdit->setText (
        shPrefs.get (SharedPreferences::PlayerPath).toString ());

    if (playerPathEdit->text ().isEmpty ())
        playerPathEdit->setText ("vlc --quiet %u");

    umphPathEdit->setText (
        shPrefs.get (SharedPreferences::UmphPath).toString ());

#ifdef _0
    if (umphPathEdit->text ().isEmpty ())
        umphPathEdit->setText ("umph");
#endif

    grakePathEdit->setText (
        shPrefs.get (SharedPreferences::GrakePath).toString ());

    if (grakePathEdit->text ().isEmpty ())
        grakePathEdit->setText ("grake");

    saveDirEdit->setText (
        shPrefs.get (SharedPreferences::SaveDir).toString ());

    filenameFormatEdit->setText (
        shPrefs.get (SharedPreferences::FilenameFormat).toString ());

    if (filenameFormatEdit->text ().isEmpty ())
        onFilenameFormatChanged (0);

    regexpEdit->setText (
        shPrefs.get (SharedPreferences::Regexp).toString ());

    if (regexpEdit->text ().isEmpty ())
        onRegexpChanged (0);

    minToTrayBox->setChecked (
        shPrefs.get (SharedPreferences::MinToTray).toBool ()
            ? Qt::Checked
            : Qt::Unchecked
    );

    stayOnTopBox->setChecked (
        shPrefs.get (SharedPreferences::StayOnTop).toBool ()
            ? Qt::Checked
            : Qt::Unchecked
    );

    playWhenDoneBox->setChecked (
        shPrefs.get (SharedPreferences::PlayWhenDone).toBool ()
            ? Qt::Checked
            : Qt::Unchecked
    );

    showReminderBox->setChecked (
        shPrefs.get (SharedPreferences::ShowReminder).toBool ()
            ? Qt::Checked
            : Qt::Unchecked
    );

    dontPromptFilenameBox->setChecked (
        shPrefs.get (SharedPreferences::DontPromptFilename).toBool ()
            ? Qt::Checked
            : Qt::Unchecked
    );

    onRefresh ();
}

bool Preferences::restartAfter () const { return _restartAfter; }

static void
browse (QWidget *p, QLineEdit *w) {

    const QString s =
        QFileDialog::getOpenFileName (p, QObject::tr ("Browse"));

    if (!s.isEmpty ())
        w->setText (s);
}

// Slot: browse player.

void
Preferences::onBrowsePlayer ()
    { browse (this, playerPathEdit); }

// Slot: browse quvi.

void
Preferences::onBrowseQuvi ()
    { browse (this, quviPathEdit); }

// Slot: browse curl.

void
Preferences::onBrowseCurl()
    { browse (this, curlPathEdit); }

// Slot: browse umph.

void
Preferences::onBrowseUmph ()
    { browse (this, umphPathEdit); }

// Slot: browse grake.

void
Preferences::onBrowseGrake ()
    { browse (this, grakePathEdit); }

// Slot: browse save dir.

void
Preferences::onBrowseSaveDir () {

    const QString dir = QFileDialog::getExistingDirectory (
        this,
        tr ("Open directory"),
        QDir::homePath (),
        QFileDialog::ShowDirsOnly
            | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty ())
        saveDirEdit->setText (dir);
}


// Slot: filename format (current index) changed.

void
Preferences::onFilenameFormatChanged (int n) {

    static const QString fmts[] = {
        "%t.%s",    // Default
        "%t %i.%s",
        "%t %i %h.%s"
    };

    static const int m =
        sizeof (fmts) / sizeof (QString);

    if (n > m)
        n = 0;

    filenameFormatEdit->setText (fmts[n]);

    onRefresh ();
}

// Slot: regexp (current index) changed.

void
Preferences::onRegexpChanged (int n) {

    static const QString regexps[] = {
        "/(\\w|\\s)/g", // Default
        "/(\\w)/g",
        "/(\\d)/g",
        "/(\\W)/g",
        "/(\\D)/g",
        "/(\\S)/g"
    };

    static const int m =
        sizeof (regexps) / sizeof (QString);

    if (n > m)
        n = 0;

    regexpEdit->setText (regexps[n]);

    onRefresh ();
}

// Slot: filename format editing finished.

void
Preferences::onFilenameFormatEditingFinished ()
    { onRefresh (); }


// Slot: regexp editing finished.

void
Preferences::onRegexpEditingFinished ()
    { onRefresh (); }

// Slot: refresh clicked.

void
Preferences::onRefresh () {

    QString title   = "Foo bar";
    QString host    = "baz";
    QString id      = "1234";
    QString suffix  = "flv";
    QString preview = filenameFormatEdit->text ().simplified ();

    const bool ok = NomNom::format_filename (
        this,
        regexpEdit->text ().simplified (),
        title,
        suffix,
        host,
        id,
        preview
    );

    if (ok)
        previewEdit->setText (preview);

}

// main.cpp

extern History history;

// Slot: clear history.

void
Preferences::onClearHistory ()
    { history.clear (); }

// Slot: language.

void
Preferences::onLanguage () {

    QString langName;

    if (!NomNom::choose_lang (this, langName))
        return;

    QSettings s;
    s.setValue ("language", langName);

    if (NomNom::ask (this,
        tr ("The language will be changed after you restart NomNom. "
        "Restart NomNom?"))
        == QMessageBox::No)
    {
        return;
    }

    _restartAfter = true;

    done (QDialog::Accepted);
}

static QString
append (QLineEdit *w, const QStringList& l = QStringList() << "%u") {
    QString s = w->text ();
    foreach (QString w, l) {
        if (!s.contains (w))
            s += " " + w;
    }
    return s;
}

// Done. QDialog and closeEvent design glitch workaround.

void
Preferences::done (int r) {

    QSettings s;
    NomNom::save_size (s, this, QSETTINGS_GROUP);

    if (r == QDialog::Accepted) {

        shPrefs.set (SharedPreferences::QuviPath,
            append (quviPathEdit));

        shPrefs.set (SharedPreferences::CurlPath,
            curlPathEdit->text ());

        shPrefs.set (SharedPreferences::PlayerPath,
            append (playerPathEdit));

        shPrefs.set (SharedPreferences::UmphPath,
            umphPathEdit->text ());

        shPrefs.set (SharedPreferences::GrakePath,
            grakePathEdit->text ());

        shPrefs.set (SharedPreferences::SaveDir,
            saveDirEdit->text ());

        shPrefs.set (SharedPreferences::FilenameFormat,
            filenameFormatEdit->text ());

        shPrefs.set (SharedPreferences::Regexp,
            regexpEdit->text ());

        shPrefs.set (SharedPreferences::MinToTray,
            minToTrayBox->isChecked ());

        shPrefs.set (SharedPreferences::StayOnTop,
            stayOnTopBox->isChecked ());

        shPrefs.set (SharedPreferences::PlayWhenDone,
            playWhenDoneBox->isChecked ());

        shPrefs.set (SharedPreferences::ShowReminder,
            showReminderBox->isChecked ());

        shPrefs.set (SharedPreferences::DontPromptFilename,
            dontPromptFilenameBox->isChecked ());

        shPrefs.write ();
    }

    QDialog::done (r);
    close ();
}

// Close.

void
Preferences::closeEvent (QCloseEvent *e)
    { QDialog::closeEvent(e); }

#undef QSETTINGS_GROUP
#define QSETTINGS_GROUP "SharedPreferences"

// SharedPreferences: write.

void
SharedPreferences::write () const {

    QSettings s;
    s.beginGroup (QSETTINGS_GROUP);

    s.setValue ("quviPath",     quviPath);
    s.setValue ("curlPath",     curlPath);
    s.setValue ("playerPath",   playerPath);
    s.setValue ("umphPath",     umphPath);
    s.setValue ("grakePath",    grakePath);
    s.setValue ("saveDir",      saveDir);
    s.setValue ("filenameFormat", filenameFormat);
    s.setValue ("regexp",       regexp);
    s.setValue ("minToTray",    minToTray);
    s.setValue ("stayOnTop",    stayOnTop);
    s.setValue ("playWhenDone", playWhenDone);
    s.setValue ("showReminder", showReminder);
    s.setValue ("dontPromptFilename", dontPromptFilename);

    s.endGroup ();
}

// SharedPreferences: read.

void
SharedPreferences::read () {

    QSettings s;
    s.beginGroup (QSETTINGS_GROUP);

    quviPath     = s.value ("quviPath").toString ();
    curlPath     = s.value ("curlPath").toString ();
    playerPath   = s.value ("playerPath").toString ();
    umphPath     = s.value ("umphPath").toString ();
    grakePath    = s.value ("grakePath").toString ();
    saveDir      = s.value ("saveDir").toString ();
    filenameFormat = s.value ("filenameFormat").toString ();
    regexp       = s.value ("regexp").toString ();
    minToTray    = s.value ("minToTray").toBool ();
    stayOnTop    = s.value ("stayOnTop").toBool ();
    playWhenDone = s.value ("playWhenDone").toBool ();
    showReminder = s.value ("showReminder").toBool ();
    dontPromptFilename = s.value ("dontPromptFilename").toBool ();

    s.endGroup ();
}

// SharedPreferences: set.

void
SharedPreferences::set (Option opt, const QVariant& v) {
    switch (opt) {
    case QuviPath       : quviPath     = v.toString (); break;
    case CurlPath       : curlPath     = v.toString (); break;
    case PlayerPath     : playerPath   = v.toString (); break;
    case UmphPath       : umphPath     = v.toString (); break;
    case GrakePath      : grakePath    = v.toString (); break;
    case SaveDir        : saveDir      = v.toString (); break;
    case FilenameFormat : filenameFormat = v.toString (); break;
    case Regexp         : regexp       = v.toString (); break;
    case MinToTray      : minToTray    = v.toBool (); break;
    case StayOnTop      : stayOnTop    = v.toBool (); break;
    case PlayWhenDone   : playWhenDone = v.toBool (); break;
    case ShowReminder   : showReminder = v.toBool (); break;
    case DontPromptFilename: dontPromptFilename = v.toBool (); break;
    default:
        qDebug()
            << __PRETTY_FUNCTION__
            << __LINE__
            << "invalid option id"
            << opt;
        break;
    }
}

// SharedPreferences: get.

QVariant
SharedPreferences::get (Option opt) const {
    switch (opt) {
    case QuviPath     : return QVariant (quviPath);
    case CurlPath     : return QVariant (curlPath);
    case PlayerPath   : return QVariant (playerPath);
    case UmphPath     : return QVariant (umphPath);
    case GrakePath    : return QVariant (grakePath);
    case SaveDir      : return QVariant (saveDir);
    case FilenameFormat:return QVariant (filenameFormat);
    case Regexp       : return QVariant (regexp);
    case MinToTray    : return QVariant (minToTray);
    case StayOnTop    : return QVariant (stayOnTop);
    case PlayWhenDone : return QVariant (playWhenDone);
    case ShowReminder : return QVariant (showReminder);
    case DontPromptFilename:return QVariant (dontPromptFilename);
    default:
        qDebug()
            << __PRETTY_FUNCTION__
            << __LINE__
            << "invalid option id"
            << opt;
        break;
    }

    return QVariant ();
}


