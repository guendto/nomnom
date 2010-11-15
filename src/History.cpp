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

#include <QSettings>
#include <QDebug>

#include "History.h"

#define QSETTINGS_GROUP "History"

History::History  () :maxItems (10) { }
History::~History () { }

void
History::write () const
    { QSettings ().setValue(QString("%1/drops").arg(QSETTINGS_GROUP), drops); }

void
History::read () {
    drops = QSettings ().value (QString ("%1/drops")
        .arg (QSETTINGS_GROUP))
        .toStringList ();
}

void
History::append (const QString& s) {

    drops << s;

    drops.removeDuplicates ();

    if (drops.size () > maxItems)
        drops.takeFirst ();
}

History&
History::operator<<(const QString& s) {
    append (s);
    return *this;
}

QStringList History::toStringList () const { return drops; }
void        History::clear        ()       { drops.clear (); }


