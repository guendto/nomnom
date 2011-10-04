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

#include <QScriptValueIterator>
#include <QScriptEngine>
#include <QVariant>
#include <QString>
#include <QLabel>

#ifdef _0
#include <QDebug>
#endif

#include "Media.h"

// Ctor.

Media::Media ()
  : QObject(), _length (0)
{ }

Media::Media (const Media& v)
  : QObject(), _length(0)
{
  _link        = v._link;
  _title       = v._title;
  _pageURL     = v._pageURL;
  _id          = v._id;
  _format      = v._format;
  _length      = v._length;
  _suffix      = v._suffix;
  _ctype       = v._ctype;
}

Media&
Media::operator=(const Media&)
{
  return *this;
}

// Parse from JSON.

bool
Media::fromJSON (const QString& data, QString& error)
{
  QScriptEngine e;

  QScriptValue v = e.evaluate ("(" +data+ ")");

  if (e.hasUncaughtException ())
    {
      error = tr ("Uncaught exception at line %1: %2")
              .arg (e.uncaughtExceptionLineNumber ())
              .arg (v.toString ());
      return false;
    }

  _title   = v.property ("page_title").toString ().simplified ();
  _pageURL = v.property ("page_url").toString ();
  _id      = v.property ("id").toString ();
  _host    = v.property ("host").toString ();

  QScriptValueIterator i (v.property ("link"));

  if ( !i.hasNext () )
    {
      error = tr("Expected at least one media link from quvi(1), got none.");
      return false;
    }

  i.next  ();
  v = i.value ();

  _length = v.property ("length_bytes").toVariant ().toLongLong ();
  _ctype  = v.property ("content_type").toString ();
  _suffix = v.property ("file_suffix").toString ();
  _link   = v.property ("url").toString ();

#ifdef _0
  qDebug ()
      << _length
      << _ctype
      << _suffix
      << _link;
#endif

  return true;
}

// Get.

QVariant
Media::get (Detail d) const
{
  switch (d)
    {
    case Link       :
      return QVariant (_link);
    case Title      :
      return QVariant (_title);
    case PageURL    :
      return QVariant (_pageURL);
    case ID         :
      return QVariant (_id);
    case Format     :
      return QVariant (_format);
    case Length     :
      return QVariant (_length);
    case Suffix     :
      return QVariant (_suffix);
    case ContentType:
      return QVariant (_ctype);
    case Host       :
      return QVariant(_host);
    }

  return QVariant();
}

// Set.

void
Media::set (Detail d, const QString& s)
{
  bool ignored = false;
  switch (d)
    {
    case Link       :
      _link     = s;
      break;
    case Title      :
      _title    = s;
      break;
    case PageURL    :
      _pageURL  = s;
      break;
    case ID         :
      _id       = s;
      break;
    case Format     :
      _format   = s;
      break;
    case Length     :
      _length   = s.toLongLong(&ignored);
      break;
    case Suffix     :
      _suffix   = s;
      break;
    case ContentType:
      _ctype    = s;
      break;
    case Host       :
      _host     = s;
      break;
    default         :
      break;
    }
}

// MediaException: ctor.

MediaException::MediaException (const QString& errmsg)
  : errmsg(errmsg)
{ }

// MediaException: what.

const QString&
MediaException::what() const
{
  return errmsg;
}

// vim: set ts=2 sw=2 tw=72 expandtab:
