/**************************************************************************//**
 * NAME:    tabmanager.h
 *
 * PURPOSE: Class definition for the tab manager.
 *
 * DATE:    June 18, 2010
*******************************************************************************/
#ifndef TABMANAGER_H
#define TABMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>

#include "mainwindow.h"
#include "settings.h"
#include "tab.h"
#include "types.h"

const bool SM_FORCE = true;

QT_BEGIN_NAMESPACE
class QTabWidget;
class QWidget;
QT_END_NAMESPACE

class STabWidget;
class Tab;

namespace Ui {
	class Tab;
}

class TabManager : public QObject {
Q_OBJECT
public:
	TabManager(MainWindow *mw);
	virtual ~TabManager();
	void closeTabHide();
	void closeTabShow();
   Tab *currentTab() const;
   bool currentTab(Tab *tab) const;
	inline QWidget *getCloseTab() const { return m_closeTab; }
   inline MainWindow *getMw() const { return mw; }
   inline STabWidget *getTw() { return tw; }
   void resetMru();
   void shutdown() const;
   bool special(const QWidget * const p) const;
   Tab *tab(QWidget *w) const;
   void tabChanged(int index);
   void tabClose(Tab *tab, bool force = false);
   int tabFindPtr(quintptr ptr) const;
   QString tabLabel(int idx);
   void tabsDebug(const QString & f) const;

private:
   bool m_noRecurse;           ///< blocks recursive signals in tabChanged()
   bool m_tabAdded;            ///< indicates current tab changed because tab was added
   MainWindow *mw;             ///< pass on to child tabs (only)
   Tab *mru;
	STabWidget *tw;             ///< shortcut ptr to main window tab widget
	QVector<Tab *> tui;         ///< the results tabs in the main window
	QWidget *m_closeTab;        ///< hide/show close tab (can't change visibility)

public slots:
   void cancel() const;
   void cancelAll() const;
   void tabClose();
   void tabCreate();
   void tabFind();

signals:
   void closeMainWindow();
   void statusRight(const QString &);
};

#endif // TABMANAGER_H
