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

#ifndef nomnom_reminder_h
#define nomnom_reminder_h

#include "ui_Reminder.h"

class Reminder : public QDialog, private Ui::Reminder {
    Q_OBJECT
public:
    Reminder(QWidget *parent, const QString& group);
public:
    bool conditionalExec();
protected:
    void done(int);
    void closeEvent(QCloseEvent*);
private slots:
    // UI
    void onNext ();
private:
    QString showReminderKey;
    QString firstRunKey;
    bool showReminder;
    size_t currTip;
    bool firstRun;
};

#endif


