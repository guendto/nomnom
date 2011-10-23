/* NomNom
 * Copyright (C) 2011  Toni Gundogdu <legatvs@gmail.com>
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

#include <QScriptValueIterator>
#include <QScriptEngine>

#ifdef ENABLE_VERBOSE
#include <QDebug>
#endif

#include <NFeedProgressDialog>

extern QString umph_path;

namespace nn
{

NFeedProgressDialog::NFeedProgressDialog(QWidget *parent/*=NULL*/)
  : QProgressDialog(parent), _cancelled(false), _errcode(-1), _proc(NULL)
{
  _proc = new QProcess;
  _proc->setProcessChannelMode(QProcess::MergedChannels);

  connect(_proc, SIGNAL(error(QProcess::ProcessError)),
          this, SLOT(error(QProcess::ProcessError)));
  connect(_proc, SIGNAL(readyRead()), this, SLOT(read()));
  connect(_proc, SIGNAL(finished(int, QProcess::ExitStatus)),
          this, SLOT(finished(int, QProcess::ExitStatus)));
  connect(this, SIGNAL(canceled()), this, SLOT(cleanup()));

  setWindowModality(Qt::WindowModal);
  setAutoClose(false);
}

bool NFeedProgressDialog::open(QStringList args)
{
  setLabelText(tr("Working..."));

  _cancelled = false;
  _args = args;

  _buffer.clear();
  _errmsg.clear();

  setMaximum(0);
  setMinimum(0);

  show();
#ifdef ENABLE_VERBOSE
  qDebug() << __PRETTY_FUNCTION__ << __LINE__ << "args=" << args;
#endif
  _proc->start(args.takeFirst(), args);
  exec();

  return _errmsg.isEmpty();
}

bool NFeedProgressDialog::results(feed::NFeedList& dst, QString& err)
{
  const int n = _buffer.indexOf("{");
  if (n == -1)
    {
      err = tr("Unexpected data from umph");
      return false;
    }

  QScriptEngine e;
  QScriptValue v = e.evaluate("("+_buffer.mid(n)+")");
  if (e.hasUncaughtException())
    {
      err = tr("Uncaught exception at line %1: %2")
            .arg(e.uncaughtExceptionLineNumber())
            .arg(v.toString());
      return false;
    }

  QScriptValueIterator i(v.property("video"));
  if (!i.hasNext())
    {
      err = tr("umph did not return any video entries");
      return false;
    }
  dst.clear();

  QString t,u;
  while (i.hasNext())
    {
      i.next();
      if (i.flags() & QScriptValue::SkipInEnumeration)
        continue;
      v = i.value();
      t = v.property("title").toString();
      u = v.property("url").toString();

      feed::NFeedItem item(t,u);
      dst.append(item);
    }
  return true;
}

void NFeedProgressDialog::error(QProcess::ProcessError n)
{
  if (!_cancelled)
    {
      _errmsg = _proc->errorString();
      _errcode = n;
    }
  cancel();
}

void NFeedProgressDialog::read()
{
  static char b[1024];
  while (_proc->readLine(b, sizeof(b)))
    _buffer += QString::fromLocal8Bit(b);
}

void NFeedProgressDialog::finished(int ec, QProcess::ExitStatus es)
{
  if (es != QProcess::NormalExit || ec != 0)
    {
      if (!_cancelled)
        {
          QRegExp rx("error: (.*)");
          QString m = (rx.indexIn(_buffer) != -1)
                      ? rx.cap(1).simplified()
                      : _buffer;
          _errcode = ec;
          _errmsg = m;
        }
    }
  cancel();
}


void NFeedProgressDialog::cleanup()
{
  _cancelled = true;
  if (_proc->state() == QProcess::Running)
    _proc->kill();
}

QString NFeedProgressDialog::errmsg() const
{
  return _errmsg;
}

int NFeedProgressDialog::errcode() const
{
  return _errcode;
}

bool NFeedProgressDialog::cancelled() const
{
  return _cancelled;
}

} // namespace nn

/* vim: set ts=2 sw=2 tw=72 expandtab: */
