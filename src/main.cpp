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

#include <iostream>

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QSettings>

#ifdef ENABLE_VERBOSE
#include <QDebug>
#endif

#include <NSettingsMutator>
#include <NDetectDialog>
#include <NSysTray>
#include <NFeed>
#include <NLang>

#include "Recent.h"
// UI:
#include "MainWindow.h"

using namespace nn;

bool have_quvi_feature_query_formats = false;
bool have_umph_feature_all = false;
NSettingsMutator settings;
feed::NFeedList feedItems;
NSysTray *systray = NULL;
Recent recent;

static void set_value(const SettingKey k, const detect::NResult& r)
{
  settings.setValue(k, QString("%1%2%3")
                    .arg(r.first)
                    .arg(NSETTINGS_CMDPATH_SEPARATOR)
                    .arg(r.second.first));
}

static void first_run(QSettings& s)
{
  NDetectDialog *d = new NDetectDialog;
  d->showModeComboBox(false);
  d->exec();
  set_value(DownloadUsing, d->downloader());
  set_value(ParseUsing,    d->mediaParser());
  set_value(PlayUsing,     d->mediaPlayer());
  set_value(FeedUsing,     d->feedParser());
  s.setValue("FirstRun", false);
  settings.write();
}

static bool config_path(const QSettings& s)
{
  std::clog << qPrintable(s.fileName()) << std::endl;
  return true;
}

static void print_nresult(const nn::detect::NResult& r)
{
  std::clog << "  " <<  qPrintable(r.first)
            << " (v" << qPrintable(r.second.second) << ")"
            << std::endl;
}

static void dump_nresults(const nn::DetectType n, const QString& s)
{
  nn::detect::NResultList l;
  nn::detect::find(n, l);
  std::clog << qPrintable(s) << std::endl;
  foreach (const nn::detect::NResult r, l)
  {
    print_nresult(r);
  }
}

static bool detect_cmds()
{
  std::clog << "Detect commands from $PATH..." << std::endl;
  dump_nresults(MediaParser, "Media parsers:");
  dump_nresults(MediaPlayer, "Media players:");
  dump_nresults(FeedParser, "Feed parsers:");
  dump_nresults(Downloader, "Downloaders:");
}

static bool version()
{
  std::clog << PACKAGE_VERSION << std::endl;
}

static bool help()
{
  const QString arg0 = QCoreApplication::arguments()[0];
  std::clog
      << "Usage: " << qPrintable(arg0) << " [options]\n"
      << "Options:\n"
      << "  --config-path  Print path to a local user config file and exit\n"
      << "  --detect       Print detected commands from $PATH and exit\n"
#ifdef ENABLE_VERBOSE
      << "  --verbose      Turn on verbose output\n"
#endif
      << "  --version      Print version and exit\n"
      << "  --help         Print help and exit"
      << std::endl;
}

#ifdef ENABLE_VERBOSE
static bool verbose = false;
#endif

static bool parse_args(const QSettings& qs)
{
  QStringList args = QCoreApplication::arguments();
  args.takeFirst();
  foreach (const QString s, args)
  {
    if (s == "--config-path")
      return config_path(qs);
    else if (s == "--detect")
      return detect_cmds();
#ifdef ENABLE_VERBOSE
    else if (s == "--verbose")
      verbose = true;
#endif
    else if (s == "--version")
      return version();
    else if (s == "--help")
      return help();
    else
      {
        std::clog << "error: invalid option: " << qPrintable(s) << std::endl;
        return true;
      }
  }
  return false;
}

#ifdef ENABLE_VERBOSE
static void new_msg_handler(QtMsgType t, const char *m)
{
  if (!verbose)
    return;

  switch (t)
    {
    case QtDebugMsg:
    default:
      std::clog << m << std::endl;
      break;
    case QtWarningMsg:
      std::clog << "(w): " << m << std::endl;
      break;
    case QtCriticalMsg:
      std::clog << "(c): " << m << std::endl;
      break;
    case QtFatalMsg:
      std::clog << "(f): " << m << std::endl;
      abort();
      break;
    }
}
#endif

int main(int argc, char *argv[])
{
#ifdef ENABLE_VERBOSE
  qInstallMsgHandler(new_msg_handler);
#endif

  QApplication app(argc, argv);
#define APPNAME   "NomNom"
#define APPDOMAIN "nomnom.sourceforge.net"
  QCoreApplication::setOrganizationName  (APPNAME);
  QCoreApplication::setOrganizationDomain(APPDOMAIN);
  QCoreApplication::setApplicationName   (APPNAME);
  QCoreApplication::setApplicationVersion(PACKAGE_VERSION);
#undef APPNAME
#undef APPDOMAIN

  settings.read();

// Command line args.

  QSettings s;

  if (parse_args(s))
    return 0;

// Qt translation.

  QTranslator qtTranslator;

#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__
           << "qt_" + QLocale::system().name()
           << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));

  app.installTranslator(&qtTranslator);

// Application translation.

  bool r = false;

  s.beginGroup("Settings"); // settings/nsettingsmutator.cpp
  if (s.contains("Language"))
    r = lang::choose(s.value("Language").toString());

  if (!r) // Use system locale.
    lang::choose();

// Detect commands if this is a first run.

  if (s.contains("FirstRun"))
    {
      if (s.value("FirstRun").toBool() == true)
        first_run(s);
    }
  else
    first_run(s);

  s.endGroup();

// Window.

  MainWindow *w = new MainWindow;
  recent.read();
  return app.exec();
}

// vim: set ts=2 sw=2 tw=72 expandtab:
