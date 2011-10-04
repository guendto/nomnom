/*
 * NomNom
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

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QSettings>

#ifdef _0
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

int main(int argc, char *argv[])
{
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

// Qt translation.

  QTranslator qtTranslator;
#ifdef _0
  qDebug() << __PRETTY_FUNCTION__ << __LINE__
           << "qt_" + QLocale::system().name()
           << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
  qtTranslator.load("qt_" + QLocale::system().name(),
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

// Application translation.

  bool r = false;

  QSettings s;
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
