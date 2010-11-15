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

#ifndef nomnom_mainwindow_h
#define nomnom_mainwindow_h

#include <QSystemTrayIcon>
#include <QProcess>
#include <QPointer>
#include <QLabel>
#include <QMenu>
#include <QHash>

#include "Video.h"
#include "Download.h"

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT
public:
    MainWindow  ();
protected:
    void showEvent      (QShowEvent*);
    void hideEvent      (QHideEvent*);
    void closeEvent     (QCloseEvent*);
    void dragEnterEvent (QDragEnterEvent*);
    void dropEvent      (QDropEvent*);
private:
    void createContextMenu  ();
    void createTray         ();
    void readSettings       ();
    bool handleURL          (const QString&);
    void streamVideo        ();
    void downloadVideo      ();
    bool parseOK            ();
    void parseError         ();
private slots:
    // UI
    void onTrayActivated(QSystemTrayIcon::ActivationReason);
    void onPreferences  ();
    void onAddress      ();
    void onStop         ();
    void onFeed         ();
    void onRake         ();
    void onAbout        ();
    void onHistory      ();
    void onLog          ();
    // Process (quvi).
    void onQuviStarted      ();
    void onQuviError        (QProcess::ProcessError);
    void onQuviReadyRead    ();
    void onQuviFinished     (int, QProcess::ExitStatus);
    // Download (dialog).
    void onDownloadError    (QString);
private:
    QPointer<QSystemTrayIcon> trayIcon;
    QHash<QString,QAction*> actions;
    QPointer<Download> download;
    QPointer<QMenu> trayMenu;
    QPointer<Video> video;
    QProcess proc;
    QString json;
};

#endif


