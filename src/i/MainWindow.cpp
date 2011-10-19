/*
 * NomNom
 * Copyright (C) 2010-2011  Toni Gundogdu <legatvs@gmail.com>
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

#include <QInputDialog>
#include <QMainWindow>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QRegExp>

#ifdef ENABLE_VERBOSE
#include <QDebug>
#endif

#include <NSettingsMutator>
#include <NSettingsDialog>
#include <NAboutDialog>
#include <NFeedDialog>
#include <NSettings>
#include <NSysTray>
#include <NLang>
#include <NUtil>

#include "Recent.h"
// UI
#include "MainWindow.h"

#define QSETTINGS_GROUP             "MainWindow"

// main.cpp

extern bool have_quvi_feature_query_formats;
extern nn::NSettingsMutator settings;
extern bool have_umph_feature_all;
extern nn::NSysTray *systray;
extern Recent recent;

// Modes.

enum { StreamMedia=0, DownloadMedia };

MainWindow::MainWindow()
  : media(new Media)
{
  setupUi(this);
  restore();

// Create Download dialog.

  download = new DownloadDialog(this);

// Create Process Progress dialog for quvi.

  proc = new ProcessProgressDialog(this);

  connect(proc, SIGNAL(finished(QString)),
          this, SLOT(onProcFinished(QString)));

// Custom program icon.

#ifdef _1
  if (shPrefs.get(SharedPreferences::CustomProgramIcon).toBool())
    changeProgramIcon();
#endif
}

void MainWindow::createContextMenu()
{
#define creat_a(t,f,c) \
    do { \
        QAction *a = new QAction(t, textBrowser); \
        if (c) { \
            a->setCheckable(c); \
            /*connect(a, SIGNAL(toggled(bool)), SLOT(f(bool)));*/ \
        } \
        else \
            connect(a, SIGNAL(triggered()), SLOT(f())); \
        textBrowser->addAction(a); \
        actions[t] = a; \
    } while (0)

#define add_s \
    do { \
        QAction *a = new QAction(textBrowser); \
        a->setSeparator(true); \
        textBrowser->addAction(a); \
    } while (0)

  creat_a(tr("Address..."),  onAddress,   false);
  creat_a(tr("Feed..."),     onFeed,      false);
  creat_a(tr("Recent..."),   onRecent,    false);
  add_s;
  creat_a(tr("Settings..."), onSettings,  false);
  add_s;
  creat_a(tr("About..."),    onAbout,     false);
  creat_a(tr("Quit"),        onTerminate, false);

#undef add_s
#undef creat_a

  // Add key shortcuts.

#define _wrap(s,k) \
    do { actions[s]->setShortcut(QKeySequence(k)); } while (0)

  _wrap(tr("Address..."),    "Ctrl+A");
  _wrap(tr("Feed..."),       "Ctrl+F");
  _wrap(tr("Recent..."),     "Ctrl+R");
  // --
  _wrap(tr("Settings..."),   "Ctrl+E");
  // --
  _wrap(tr("Quit"),          "Ctrl+Q");
#undef _wrap

  // Add the context menu.

  textBrowser->setContextMenuPolicy(Qt::ActionsContextMenu);
}

// Create tray icon.

void MainWindow::createTrayIcon()
{
  systray = new nn::NSysTray(this, QString("<b>NomNom</b> %1").arg(VERSION));

  connect(systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(activated(QSystemTrayIcon::ActivationReason)));

  systray->addTrayMenuAction(SIGNAL(toggled(bool)),
                             this,
                             SLOT(setVisible(bool)),
                             tr("Show"),
                             true);

#ifdef _1
  // FIXME: If mainwindow is hidden, closing the NSettingsDialog causes
  // the program to exit.
  systray->addTrayMenuAction(SIGNAL(triggered()),
                             this,
                             SLOT(onSettings()),
                             tr("Settings..."));
#endif

  systray->addTrayMenuSeparator();

  systray->addTrayMenuAction(SIGNAL(triggered()),
                             this,
                             SLOT(onTerminate()),
                             tr("Quit"));

  systray->setIcon(QIcon(":img/nomnom.png"));
  systray->setTrayMenu();

  const bool startInTray = settings.value(nn::StartInTrayIcon).toBool();
  const bool showInTray = settings.value(nn::ShowInTrayIcon).toBool();

  systray->setVisible(showInTray);

  if (showInTray)
    startInTray ? hide() : show();
  else
    show();
}

// Handle (dropped) URL.

void MainWindow::handleURL(const QString& url)
{
  QString s = settings.eitherValue(nn::ParseUsing,
                                   nn::ParseUsingOther)
              .toString()
              .simplified();

  QStringList q_args = nn::to_cmd_args(s);
  const QString q = q_args.first();

#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__ << "quvi_path=" << q;
#endif

  if (q.isEmpty())
    {
      nn::info(this, tr("Please configure the path to the quvi. "
                        "See under the \"commands\" in the settings."));
      onSettings();
      return;
    }

  s = settings.eitherValue(nn::PlayUsing,
                           nn::PlayUsingOther)
      .toString()
      .simplified();

  QStringList p_args = nn::to_cmd_args(s);
  const QString p = p_args.first();

  if (p.isEmpty())
    {
      nn::info(this, tr("Please configure the path to a media player. "
                        "See under the \"commands\" in the settings."));
      onSettings();
      return;
    }

// Recent.

  recent << url;

// 0x1=invalid input, 0x3=no input

  have_quvi_feature_query_formats =
    nn::check_for_cmd_feature(nn::ParseUsing,
                              nn::ParseUsingOther,
                              "-F",
                              0x3);

// Query formats to an URL.

#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__
           << "have_quvi_feature_query_formats="
           << have_quvi_feature_query_formats;
#endif

  QStringList formats;
  if (have_quvi_feature_query_formats)
    {
      bool failed = false;

      if (!queryFormats(formats, q_args, url, failed))
        formats.clear();

      if (failed)
        {
          nn::info(this, proc->errmsg());
          return;
        }

      if (proc->canceled())
        return;
    }

  json.clear();

// Choose format.

  QString fmt;
  if (!selectFormat(formats, fmt))
    return;

// Query media stream data.

  q_args.replaceInStrings("%u", url);
  q_args << "-f" << fmt;

#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__ << "q_args=" << q_args;
#endif

  proc->setLabelText(tr("Checking..."));
  proc->setMaximum(0);
  proc->start(q_args);

  if (proc->canceled())
    return;

// Download media or pass media stream URL to a media player.

  QString errmsg;
  if (parseOK(errmsg))
    {
      if (modeCBox->currentIndex() == StreamMedia)
        streamMedia();
      else
        downloadMedia();
    }
  else
    nn::info(this, errmsg);
}

// Query formats to an URL.

bool MainWindow::queryFormats(QStringList& formats,
                              const QStringList& q_args,
                              const QString& url,
                              bool& failed)
{
  QStringList args = q_args;
  args.replaceInStrings("%u", url);
  args << "-F";

#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__ << "args=" << args;
#endif

  json.clear();

  proc->setLabelText(tr("Checking..."));
  proc->setMaximum(0);
  proc->start(args);

  failed = proc->failed();
  if (failed)
    return false;

#ifdef ENABLE_VERBOSE
  qDebug() <<  __PRETTY_FUNCTION__ << __LINE__ << "failed=" << failed;
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

#ifdef ENABLE_VERBOSE
        qDebug() << __PRETTY_FUNCTION__ << __LINE__ << "formats=" << formats;
#endif

        return true;
      }
  }
  return false;
}

// Select a format.

bool MainWindow::selectFormat(QStringList& formats, QString& fmt)
{
  // Prompt only if count exceeds 1 ("default)".
  if (formats.size() < 2)
    {
      fmt = "default"; // Do not translate. quvi understands only this.
      return true;
    }

  bool ok = false;
  fmt = QInputDialog::getItem(this,
                              tr("Choose format"),
                              tr("Format:"),
                              formats << tr("Enter your own"),
                              0,
                              false,
                              &ok);
  if (!ok)
    return false; // Cancel

  if (fmt == tr("Enter your own"))
    {
      fmt = QInputDialog::getText(this,
                                  tr("Enter format"),
                                  tr("Format:"),
                                  QLineEdit::Normal,
                                  "default",
                                  &ok);
    }
  return ok && !fmt.isEmpty();
}

// View media (stream).

void MainWindow::streamMedia()
{
  const QString p = settings.eitherValue(nn::PlayUsing,
                                         nn::PlayUsingOther)
                    .toString()
                    .simplified();

  QStringList args = nn::to_cmd_args(p);
  args.replaceInStrings("%m", media->get(Media::Link).toString());

  const QString cmd = args.takeFirst();

  if (!QProcess::startDetached(cmd, args))
    nn::info(this, tr("Error while running command:<p>%1</p>").arg(cmd));
}

// Download media (to a file).

void MainWindow::downloadMedia()
{
  QString fname = settings.value(nn::FilenameFormat).toString();

  const QString suffix =
    media->get(Media::Suffix).toString().simplified();

  bool ok = nn::format_filename(
              settings.value(nn::FilenameRegExp).toString(),
              media->get(Media::Title).toString().simplified(),
              media->get(Media::ID).toString().simplified(),
              media->get(Media::Host).toString().simplified(),
              suffix,
              fname
            );

  if (!ok)
    return;

  QString fpath = settings.value(nn::SaveMediaDirectory).toString();

  if (fpath.isEmpty())
    fpath = QDir::homePath();

  fpath = QDir::toNativeSeparators(fpath +"/"+ fname);

  const bool ask_where_to_save =
    settings.value(nn::AskWhereToSaveMediaFile).toBool();

  if (ask_where_to_save)
    {
      const QFileDialog::Options opts =
        QFileDialog::DontConfirmOverwrite;

      fpath = QFileDialog::getSaveFileName(this,
                                           tr("Save media as"),
                                           fpath,
                                           suffix, // Filter.
                                           0,      // Selected filter.
                                           opts);
      if (fpath.isEmpty())
        return;
    }

  if (settings.value(nn::ReplaceExistingMedia).toBool())
    QDir().remove(fpath);

  const qint64 expected_bytes =
    media->get(Media::Length).toLongLong();

  if (QFileInfo(fpath).size() < expected_bytes)
    {
      const QString p = settings.eitherValue(nn::DownloadUsing,
                                             nn::DownloadUsingOther)
                        .toString()
                        .simplified();

      QStringList args = nn::to_cmd_args(p);
      args.replaceInStrings("%u", media->get(Media::Link).toString());
      args.replaceInStrings("%f", fpath);

      download->start(args);

      if (download->canceled())
        return;

      if (download->failed())
        {
          nn::info(this, download->errmsg());
          return;
        }
    }

  const bool completeFile =
    QFileInfo(fpath).size() >= expected_bytes;

  const bool playWhenDone =
    settings.value(nn::PlayWhenDoneDownloading).toBool();

#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__ << "file="
           << QFileInfo(fpath).size()
           << expected_bytes
           << completeFile
           << playWhenDone;
#endif

  if (completeFile && playWhenDone)
    {
      const QString p = settings.eitherValue(nn::PlayUsing,
                                             nn::PlayUsingOther)
                        .toString()
                        .simplified();

      QStringList args = nn::to_cmd_args(p);
      args.replaceInStrings("%m", fpath);

      const QString cmd = args.takeFirst();

      if (!QProcess::startDetached(cmd, args))
        {
          nn::info(this,
                   tr("Error while running command:<p>%1</p>").arg(cmd));
        }
    }
}

// Change program icon.

void MainWindow::changeProgramIcon()
{
#ifdef _1
  const bool customProgramIcon =
    shPrefs.get(SharedPreferences::CustomProgramIcon).toBool();

  QString html = textBrowser->toHtml();

  const QString iconPath =
    customProgramIcon
    ? shPrefs.get(SharedPreferences::ProgramIconPath).toString()
    : ":img/nomnom.png";

  html.replace(QRegExp("img src=\".*\""), "img src=\"" +iconPath+ "\"");

  textBrowser->setHtml(html);

  QIcon icon = QIcon(iconPath);
  trayIcon->setIcon(icon);
  setWindowIcon(icon);
#endif // _1
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

  return media->fromJSON(json.mid(n), errmsg);
}

static void check_window_flags(QWidget *w)
{
  Qt::WindowFlags flags = w->windowFlags();
  if (settings.value(nn::KeepApplicationWindowOnTop).toBool())
    {
      if (!(flags & Qt::WindowStaysOnTopHint))
        flags |= Qt::WindowStaysOnTopHint;
    }
  else
    {
      if (flags & Qt::WindowStaysOnTopHint)
        flags &= ~Qt::WindowStaysOnTopHint;
    }

  if (flags != w->windowFlags())
    {
      w->setWindowFlags(flags);
      if (w->isHidden())
        w->show();
    }
}

static bool check_language(QWidget *w, const QString& lang)
{
#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__ << "lang="
           << lang
           << settings.value(nn::Language).toString();
#endif

  if (lang != settings.value(nn::Language).toString())
    {
      if (nn::ask(w, qApp->translate("MainWindow",
                                     "Language will be changed after "
                                     "you restart the application. "
                                     "Restart now?"))
          == QMessageBox::No)
        {
          return false;
        }
      return true;
    }
  return false;
}

void MainWindow::onSettings()
{
  QString lang = settings.value(nn::Language).toString();

  if (lang.isEmpty()) // NSettingsDialog uses "English" instead of ""
    lang = "English";

  nn::NSettingsDialog *d = new nn::NSettingsDialog(this);
  if (d->exec() == QDialog::Accepted)
    {
      if (check_language(this, lang))
        {
          onTerminate();
          QProcess::startDetached(QCoreApplication::applicationFilePath());
          return;
        }
      check_window_flags(this);
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
  const QStringList lst = recent.toStringList();

  if (lst.size() == 0)
    {
      nn::info(this, tr("No record of recently visited URLs found."));
      return;
    }

  bool ok = false;
  const QString s =
    QInputDialog::getItem(this,
                          tr("Recent URLs"),
                          tr("Select URL (most recent first):"),
                          lst,
                          0,
                          false,
                          &ok);
  if (!ok)
    return;

  handleURL(s);
}

// Slot: Download.

void MainWindow::onAddress()
{
  const QString url =
    QInputDialog::getText(this, tr("Address"), tr("Media page URL:"));

  if (url.isEmpty())
    return;

  handleURL(url);
}

// Slot: on feed.

void MainWindow::onFeed()
{
  const QString p = settings.eitherValue(nn::FeedUsing,
                                         nn::FeedUsingOther)
                    .toString()
                    .simplified();

  QStringList args = nn::to_cmd_args(p);
  const QString r = args.first();

  if (r.isEmpty())
    {
      nn::info(this, tr("Please configure the path to a feed reader. "
                        "See under the \"commands\" in the settings."));
      onSettings();
      return;
    }

  have_umph_feature_all =
    nn::check_for_cmd_feature(nn::FeedUsing,
                              nn::FeedUsingOther,
                              "-a",
                              0x0);

  nn::NFeedDialog *d = new nn::NFeedDialog(this, args);
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
  handleURL(e->mimeData()->text().simplified());
  e->acceptProposedAction();
}

static void update_show_state(QWidget *w)
{
  const bool v = !w->isVisible();

  QAction *a = systray->findTrayMenuAction(
                 qApp->translate("MainWindow", "Show"));

  if (a)
    a->setChecked(v);

  w->setVisible(v);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__;
#endif

  if (settings.value(nn::TerminateInstead).toBool()
      || !systray->isVisible())
    {
      if (settings.value(nn::ClearURLRecordAtExit).toBool())
        recent.clear();
      save();
      e->accept();
    }
  else
    {
      update_show_state(this);
      e->ignore();
    }
}

void MainWindow::activated(QSystemTrayIcon::ActivationReason r)
{
  if (r == QSystemTrayIcon::Trigger)
    update_show_state(this);
}

void MainWindow::onTerminate()
{
  // When systray icon is visible: the default behaviour is to ignore
  // calls to 'close' mainwindow unless "terminate instead" is true.
#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__;
#endif
  // Although the line below uses "settings" value "TerminateInstead",
  // it is not written to config.
  settings.setValue(nn::TerminateInstead, true);
  close();
}

static void tweak_window_flags(QWidget *w)
{
  Qt::WindowFlags flags = w->windowFlags();

// Remove buttons.

  flags &= ~Qt::WindowMinimizeButtonHint;
  flags &= ~Qt::WindowMaximizeButtonHint;

// Stay on top?

  if (settings.value(nn::KeepApplicationWindowOnTop).toBool())
    flags |= Qt::WindowStaysOnTopHint;

  w->setWindowFlags(flags);
}

void MainWindow::restore()
{
  QSettings s;
  s.beginGroup(QSETTINGS_GROUP);
  modeCBox->setCurrentIndex(s.value("modeCBox").toInt());
  restoreGeometry(s.value("geometry").toByteArray());
  restoreState(s.value("state").toByteArray());
  s.endGroup();

  tweak_window_flags(this);
  createContextMenu();
  createTrayIcon();
}

void MainWindow::save()
{
  QSettings s;
  s.beginGroup(QSETTINGS_GROUP);
  s.setValue("modeCBox", modeCBox->currentIndex());
  s.setValue("geometry", saveGeometry());
  s.setValue("state", saveState());
  s.endGroup();
  recent.write();
}

// vim: set ts=2 sw=2 tw=72 expandtab:
