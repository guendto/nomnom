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

#include <QDebug>

#include "util.h"
#include "Log.h"
#include "DownloadDiag.h"

// main.cpp
extern Log log;

DownloadDialog::DownloadDialog (QWidget *parent/*=NULL*/)
    : QProgressDialog (parent), _canceled (false)
{

#define _wrap(s,sl) \
    do { connect (&_proc, SIGNAL(s), this, SLOT(sl)); } while (0)

    _wrap (started (), onCurlStarted ());

    _wrap (error (QProcess::ProcessError),
        onCurlError (QProcess::ProcessError));

    _wrap (readyRead (), onCurlReadyRead ());

    _wrap (finished (int, QProcess::ExitStatus),
        onCurlFinished (int, QProcess::ExitStatus));

#undef _wrap

    connect (this, SIGNAL(canceled()), this, SLOT (onCanceled()));

    setWindowModality (Qt::WindowModal);
    setAutoClose (false);

    _proc.setProcessChannelMode (QProcess::MergedChannels);
}

void
DownloadDialog::start (const QString& cmd, const QString& fpath, Video *video) {

    Q_ASSERT (!cmd.isEmpty ());
    Q_ASSERT (!fpath.isEmpty ());
    Q_ASSERT (video != NULL);

    _lastError.clear ();

    QStringList args = cmd.split (" ");

    args.replaceInStrings ("%u", video->get (Video::Link).toString ());
    args.replaceInStrings ("%f", fpath);

    log << args.join (" ") + "\n";

    _canceled = false;

    show ();
    _proc.start (args.takeFirst (), args);
    exec ();
}

void
DownloadDialog::onCurlStarted ()
    { setLabelText (tr ("Starting download ...")); }

void
DownloadDialog::onCurlError (QProcess::ProcessError n) {

    if (!_canceled) {
        emit error (NomNom::to_process_errmsg (n));
        cancel ();
    }
}

static const QRegExp rx_prog ("^(\\d+).*(\\d+:\\d+:\\d+|\\d+d \\d+h)\\s+(\\w+)$");
static const QRegExp rx_err  ("curl:\\s+(.*)$");
static const QRegExp rx_rate ("(\\D+)");   // rate unit

void
DownloadDialog::onCurlReadyRead () {

    static char data[1024];

    while (_proc.readLine (data, sizeof (data))) {

        QString ln = QString::fromLocal8Bit (data).simplified ();

        if (rx_err.indexIn (ln) != -1) {
            _lastError = "curl: " +rx_err.cap (1);
            continue;
        }

        if (ln.split (" ").count () < 12)
            continue; // Full line updates only, PLZKTHXBYE.

#ifdef _0
        qDebug () << ln;
        qDebug () << "--";
#endif

        if (rx_prog.indexIn (ln) != -1) {

            enum {
                PERCENT = 1,
                ETA,
                RATE
            };

#ifdef _0
            qDebug ()
                << rx_prog.cap (PERCENT)
                << rx_prog.cap (ETA)
                << rx_prog.cap (RATE);
            qDebug ()
                << ln;
#endif

            setValue (rx_prog.cap (PERCENT).toInt ());

            QString rate = rx_prog.cap (RATE);

            if (rx_rate.indexIn (rate) == -1) {
                rate = QString ("%1k").arg (rate.toLongLong ()/1024.0,2,'f',1);
            }

            const QString s = tr("Copying at %1, %2")
                .arg (rate)
                .arg (rx_prog.cap (ETA))
                ;

            setLabelText (s);
        }

        else
            log << ln;

    }

}

void
DownloadDialog::onCurlFinished (int exitCode, QProcess::ExitStatus exitStatus) {

    if (exitStatus == QProcess::NormalExit && exitCode == 0)
        ;
    else {
        if (!_canceled)
            emit error (_lastError);
    }

    cancel ();
}

void
DownloadDialog::onCanceled () {

    _canceled = true;

    if (_proc.state () == QProcess::Running)
        _proc.kill ();
}


