/**************************************************************************//**
 * NAME:    folders.h
 *
 * PURPOSE: Defines class Folders (a data attribute of class Settings).
 *
 * DATE:    September 27, 2010
*******************************************************************************/
#ifndef FOLDERS_H
#define FOLDERS_H

#include <QStringList>
#include <QWidget>

#include "settings.h"

#include "ui_settings.h"

// ack: autom4te.cache, blib, _build, .bzr, .cdv, cover_db, CVS, _darcs, ~.dep,
//      ~.dot, .git, .hg, ~.nib, .pc, ~.plst, RCS, SCCS, _sgbak and .svn
const QStringList EXCLUDES_DFLT = (QStringList()
<< "autom4te.cache" << "blib" << "_build" << ".bzr" << ".cdv" << "cover_db"
<< "CVS" << "_darcs" << ".dep" << ".dot" << ".git" << ".hg" << ".nib" << ".pc"
<< ".plst" << "RCS" << "SCCS" << "_sgbak" << ".svn" << "bin_sparc");  // << "Local Settings"

QT_BEGIN_NAMESPACE
class QFile;
class QListWidget;
QT_END_NAMESPACE

class Settings;

namespace Ui {
   class Folders;
}

class Folders : public QWidget {
Q_OBJECT
public:
   explicit Folders(QWidget *parent = 0);
   virtual ~Folders();
   void add();
   void cancelRestore();
   inline int dirFilterSz() const { return m_right.size(); }
   inline QStringList filter() const { return m_right; }
   bool filterDir(const QString &);
   QStringList filterFolders(const QStringList &);
   bool filterFolders(const QString &);
   inline Ui::Folders *getUi() const { return ui; }
   void initListWidget();
   bool listContains(const QListWidget *lw, const QString &s);
   void moveAllItems(QListWidget *src, QListWidget *dst);
   void moveSelectedItem(QListWidget *src, QListWidget *dst);
   void okApply();
   void reset();
   void saveInCaseCancel();
   void set();
   void setSearchAll(bool b) { m_searchAll = b; }
protected:
   void changeEvent(QEvent *);
   void showEvent(QShowEvent *);
private:
   bool m_searchAll;
   QStringList m_left,    // search filter (includes)
               m_right,   // search filter (excludes)
               m_saveLeft,
               m_saveRight;
   Ui::Folders *ui;
private slots:
   void on_addPushButton_clicked();
   void on_buttonBox_clicked(QAbstractButton *);
   void on_defaultPushButton_clicked();
   void on_deletePushButton_clicked();
   void on_leftAllPushButton_clicked();
   void on_leftPushButton_clicked();
   void on_rightAllPushButton_clicked();
   void on_rightPushButton_clicked();
   void on_searchAllDirsCheckBox_toggled(bool checked);
};

#endif // FOLDERS_H
