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

#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QLabel>

#include <NErrorWhileDialog>

namespace nn
{

NErrorWhileDialog::NErrorWhileDialog(const QStringList& args,
                                     const QString& errmsg,
                                     const int errcode,
                                     QWidget *parent)
  : QDialog(parent)
{
  QVBoxLayout *box = new QVBoxLayout;

  QLabel *l = new QLabel(tr("Error while running command:"));
  box->addWidget(l);

  QPlainTextEdit *t = new QPlainTextEdit(this);
  t->setPlainText(args.join(" "));
  t->setReadOnly(true);
  box->addWidget(t);

  l = new QLabel(tr("Error message follows:"));
  box->addWidget(l);

  t = new QPlainTextEdit(this);
  t->setPlainText(tr("%1\n[code #%2]").arg(errmsg).arg(errcode));
  t->setReadOnly(true);
  box->addWidget(t);

  QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
  box->addWidget(bb);

  setFixedSize(QSize(400,340));
  setWindowTitle(tr("Error"));
  setLayout(box);
  show();
}

} // namespace nn

/* vim: set ts=2 sw=2 tw=72 expandtab: */
