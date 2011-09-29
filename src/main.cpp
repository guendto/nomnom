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

#include <QApplication>
#include <QLibraryInfo>

#ifdef _0
#include <QDebug>
#endif

#include <NFeed>
#include <NLang>

#include "util.h"
#include "Recent.h"
// UI:
#include "Preferences.h"
#include "MainWindow.h"

// Global: Hosts (or websites) that quvi supports.
QMap<QString,QStringList> hosts;

// Global: quvi version.
QString quviVersion;

// Global: Preferences.
SharedPreferences shPrefs;

// Global: Recent.
Recent recent;

// Global: We have quvi --query-formats
bool have_quvi_feature_query_formats = false;

// Global: Feed items.
nn::feed::NFeedList feedItems;

// Global: We have umph --all feature
bool have_umph_feature_all = false;

int
main (int argc, char *argv[])
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
  if (s.contains("language"))
    r = nn::lang::choose(s.value("language").toString());

  if (!r) // Use system locale.
    nn::lang::choose();

// Window.

  MainWindow *w = new MainWindow;
  recent.read();
  w->show();
  return app.exec();
}

// vim: set ts=2 sw=2 tw=72 expandtab:
