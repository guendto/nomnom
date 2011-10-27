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

#ifndef nlogdialog_h
#define nlogdialog_h

#include <QDialog>

class QTreeWidget;
class QToolBox;

namespace nn
{

class NLogDialog : public QDialog
{
  Q_OBJECT
public:
  NLogDialog(QWidget *parent=NULL);
public:
  QString selected() const;
protected:
  void done(int);
private:
  void m_info(const QString& msg);
  void foreachWidget();
private slots:
  void selected(QString);
private:
  QToolBox *_toolbox;
  QString _selected;
};

class NLogWidget : public QWidget
{
  Q_OBJECT
public:
  NLogWidget(QWidget *parent);
protected:
  virtual void init() = 0;
#ifdef _1
  virtual void read() = 0;
#endif
// Friends
  friend class NLogDialog;
};

class NLogRecent : public NLogWidget
{
  Q_OBJECT
public:
  NLogRecent(QWidget *parent=NULL);
signals:
  void selected(QString);
private slots:
  void selected();
  void reset();
protected:
  virtual void init();
#ifdef _1
  virtual void read();
#endif
private:
  int confirmClear();
private:
  QTreeWidget *_treew;
};

} // namespace nn

#endif

/* vim: set ts=2 sw=2 tw=72 expandtab: */
