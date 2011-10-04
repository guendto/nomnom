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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>

#ifdef _0
#include <QDebug>
#endif

#include <NSettingsMutator>
#include <NSettingsDialog>
#include <NDetect>

extern nn::NSettingsMutator settings; // main.cpp

namespace nn
{

static void detect_type(const DetectType t, QComboBox *c)
{
  NDetectResultList r;
  if (detect::find(t, r))
    {
      foreach (const NDetectResult p, r)
      {
        c->addItem(p.first, p.second.first);
#ifdef _0
        qDebug() << p;
#endif
      }
    }
  c->addItem(QObject::tr("Other"));
}

static const char *recommended[] =
{
  QT_TR_NOOP("Recommended value: \"%1\""),
  NULL
};

static const char *f_spec[] =
{
  QT_TR_NOOP("  %f .. Path to downloaded media file"),
  NULL
};

static const char *u_spec[] =
{
  QT_TR_NOOP("  %u .. Media page URL"),
  NULL
};

static const char *m_spec[] =
{
  QT_TR_NOOP("  %m .. Media stream URL or path to downloaded media file"),
  NULL
};

static QVBoxLayout* newVBox(QWidget *p,
                            const QString& t,
                            QComboBox **c,
                            QLineEdit **e,
                            const char *f,
                            const QString& r,
                            const QStringList& specs=QStringList())
{
  *c = new QComboBox;
  p->connect(*c, SIGNAL(currentIndexChanged(QString)), p, f);

  *e = new QLineEdit;
  (*e)->hide();

  QString tooltip = r;
  foreach (const QString s, specs)
  {
    tooltip += "\n" + s;
  }
  (*e)->setToolTip(tooltip);

  QLabel *l = new QLabel(t);
  l->setBuddy(*c);

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(l);
  hbox->addWidget(*c);

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addLayout(hbox);
  vbox->addWidget(*e);

  return vbox;
}

NSettingsCommands::NSettingsCommands(QWidget *parent/*=NULL*/)
  : NSettingsWidget(parent)
{

// Parse media stream URLs using

  QVBoxLayout *parseUsingBox =
    newVBox(this,
            tr("Pa&rse media stream URLs using"),
            &parseUsingCombo,
            &parseUsingEdit,
            SLOT(parseUsingChanged(QString)),
            QString(recommended[0]).arg("quvi --category-http %u"),
            QStringList() << u_spec[0]
           );
  detect_type(MediaParser, parseUsingCombo);

// Download using

  QVBoxLayout *downloadUsingBox =
    newVBox(this,
            tr("Do&wnload using"),
            &downloadUsingCombo,
            &downloadUsingEdit,
            SLOT(downloadUsingChanged(QString)),
            QString(recommended[0]).arg("curl -L -C - -o %f %u"),
            QStringList() << f_spec[0] << u_spec[0]
           );
  detect_type(Downloader, downloadUsingCombo);

// Play using

  QVBoxLayout *playUsingBox =
    newVBox(this,
            tr("Pl&ay using"),
            &playUsingCombo,
            &playUsingEdit,
            SLOT(playUsingChanged(QString)),
            QString(recommended[0]).arg("xdg-open %m"),
            QStringList() << m_spec[0]
           );
  detect_type(MediaPlayer, playUsingCombo);

// Feed using

  QVBoxLayout *feedUsingBox =
    newVBox(this,
            tr("Read &YouTube feeds using"),
            &feedUsingCombo,
            &feedUsingEdit,
            SLOT(feedUsingChanged(QString)),
            QString(recommended[0]).arg("umph")
           );
  detect_type(FeedParser, feedUsingCombo);

// Layout

  QVBoxLayout *box = new QVBoxLayout;
  box->addLayout(parseUsingBox);
  box->addLayout(downloadUsingBox);
  box->addLayout(playUsingBox);
  box->addLayout(feedUsingBox);
  box->addStretch(0);
  setLayout(box);
}

void NSettingsCommands::init()
{
}

static bool _verify(QComboBox *combo, QLineEdit *edit, QString& msg)
{
  const QString text = combo->currentText();
  if (text == QObject::tr("Other") && edit->text().isEmpty())
    {
      msg = QObject::tr("Please enter a custom command");
      edit->setFocus();
    }
  return msg.isEmpty();
}

bool NSettingsCommands::verify(QString& msg)
{
  bool r = _verify(parseUsingCombo, parseUsingEdit, msg);
  if (r)
    r = _verify(playUsingCombo, playUsingEdit, msg);
  if (r)
    r = _verify(downloadUsingCombo, downloadUsingEdit, msg);
  if (r)
    r = _verify(feedUsingCombo, feedUsingEdit, msg);
  return r;
}

static void _write(SettingKey comboKey,
                   QComboBox *combo,
                   SettingKey editKey,
                   QLineEdit *edit)
{
  const int n = combo->currentIndex();
  const QVariant v = combo->itemData(n);

// Config must use a static "other" (non-translatable) string instead
// of using the combobox UI string which may be a translated string.

  QString ct = combo->currentText();
  if (ct == QObject::tr("Other"))
    ct = "other";

  settings.setValue(comboKey, QString("%1%2%3")
                    .arg(ct)
                    .arg(NSETTINGS_CMDPATH_SEPARATOR)
                    .arg(v.toString()));
  settings.setValue(editKey, edit->text());
}

void NSettingsCommands::write()
{
// Parse using
  _write(ParseUsing, parseUsingCombo,
         ParseUsingOther, parseUsingEdit);
// Download using
  _write(DownloadUsing, downloadUsingCombo,
         DownloadUsingOther, downloadUsingEdit);
// Play using
  _write(PlayUsing, playUsingCombo,
         PlayUsingOther, playUsingEdit);
// Feed using
  _write(FeedUsing, feedUsingCombo,
         FeedUsingOther, feedUsingEdit);
}

static void _read(SettingKey comboKey,
                  QComboBox *combo,
                  SettingKey editKey,
                  QLineEdit *edit)
{
  QString text = settings.value(comboKey).toString();
  QStringList a = text.split(NSETTINGS_CMDPATH_SEPARATOR);

// Config must use a static "other" (non-translatable) string instead
// of using the combobox UI string which may be a translated string.

  if (a[0] == "other")
    a[0] = QObject::tr("Other");

  const int n = combo->findText(a[0]);
  if (n != -1)
    {
#ifdef _0
      qDebug() << __PRETTY_FUNCTION__ << __LINE__
               << n << a[0] << a[1];
#endif
      combo->setItemData(n, a[1]);
      combo->setCurrentIndex(n);
    }
#ifdef _1
  else
    {
      if (!text.isEmpty())
        {
          qWarning() << __PRETTY_FUNCTION__ << __LINE__
                     << text
                     << "not found in combobox values";
        }
    }
#endif // _1
  text = settings.value(editKey).toString();
  edit->setText(text);
}

void NSettingsCommands::read()
{
// Parse using
  _read(ParseUsing, parseUsingCombo,
        ParseUsingOther, parseUsingEdit);
// Download using
  _read(DownloadUsing, downloadUsingCombo,
        DownloadUsingOther, downloadUsingEdit);
// Play using
  _read(PlayUsing, playUsingCombo,
        PlayUsingOther, playUsingEdit);
// Feed using
  _read(FeedUsing, feedUsingCombo,
        FeedUsingOther, feedUsingEdit);
}

void NSettingsCommands::parseUsingChanged(QString text)
{
  parseUsingEdit->setVisible(text == tr("Other"));
}

void NSettingsCommands::downloadUsingChanged(QString text)
{
  downloadUsingEdit->setVisible(text == tr("Other"));
}

void NSettingsCommands::playUsingChanged(QString text)
{
  playUsingEdit->setVisible(text == tr("Other"));
}

void NSettingsCommands::feedUsingChanged(QString text)
{
  feedUsingEdit->setVisible(text == tr("Other"));
}

} // namespace nn

/* vim: set ts=2 sw=2 tw=72 expandtab: */
