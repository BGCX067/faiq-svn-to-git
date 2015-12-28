/**************************************************************************//**
 * NAME:    tabmanager.cpp
 *
 * PURPOSE: Manages the tabs.
 *
 * DATE:    June 18, 2010
 *
 \todo
 a fully implement search state classes
 b grey out search button as needed else nasty cores drop
 c turn off TabsDebug before release
*******************************************************************************/
#include "debug.h"

#include <limits.h>

#include <QMutex>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QWidget>

#include "parms.h"
#include "settingsglobal.h"
#include "stabwidget.h"
#include "tabmanager.h"
#include "tab.h"
#include "presets/folders.h"
#include "presets/presets.h"

#include "ui_mainwindow.h"
#include "ui_settingsglobal.h"
/**************************************************************************//**
 * TabManager(): construct the tab manager.
 *
 * Parms       : _mw - main window ptr
 ******************************************************************************/
TabManager::TabManager  (MainWindow *_mw)
:
m_noRecurse(false),
m_tabAdded(false),
mw(_mw),
mru(nullptr),
tw(mw->getUi()->resultsTabWidget),  // shortcut ptr
m_closeTab(new QWidget)
{
   ENTEX("TabManager::TabManager");

   tui.resize(32);

   connect(this, SIGNAL(closeMainWindow()), mw, SLOT(close()));
   connect(this, SIGNAL(statusRight(QString)), mw, SLOT(statusRight(QString)));
}
/**************************************************************************//**
 * ~TabManager(): remove and delete all open tabs.
 ******************************************************************************/
 TabManager::~TabManager  () {
   ENTEX2("TabManager::~TabManager", 0);

   for (int i = 0, sz = tui.size(); i < sz; ++i)
      if (tui[i] != nullptr)
         tabClose(tui[i], SM_FORCE);
}
/**************************************************************************//**
 * Cancel(): cancel the current tab operation.  [slot]
 ******************************************************************************/
void TabManager::cancel  () const {
   if (tw->count() < 3)
      return;
   currentTab()->cancel();
}
/**************************************************************************//**
 * CancelAll(): cancel all tab operations.  [slot]
 ******************************************************************************/
 void TabManager::cancelAll  () const {
   for (int i = 0, sz = tui.size(); i < sz; ++i)  // cancel open tabs
      if (tui[i] != nullptr)
			tui[i]->cancel();
}
 /**************************************************************************//**
 * CloseTabHide(): hide close tab.
 ******************************************************************************/
void TabManager::closeTabHide  () {
   m_closeTab = tw->widget(0);  // save ptr to show later
   tw->removeTab(0);  // eccs
}
/**************************************************************************//**
 * CloseTabShow(): show close tab.
 *
 * Notes    : Inserting new tab at index less than or equal to current index
 *            will increment current index i.e. emit currentChanged(), but
 *            keep current page.
 ******************************************************************************/
void TabManager::closeTabShow  () {
   ENTEX("TabManager::closeTabsShow");

	QPushButton * button = new QPushButton("x");
   button->resize(20,20);
   button->setCheckable(false);

   connect(button, SIGNAL(clicked()), this, SLOT(tabClose()));

   m_tabAdded = true;
   tw->insertTab(0, m_closeTab, QString());  // eccs

	tw->getTabBar()->setTabButton(0, QTabBar::RightSide, button);
}
/**************************************************************************//**
 * CurrentTab(): returns the current tab from current widget.
 ******************************************************************************/
Tab *TabManager::currentTab  () const {
   return tw->count() < 3 ? nullptr  // no tabs
                          : dynamic_cast<Tab *>(tw->currentWidget());
}
/**************************************************************************//**
 * CurrentTab(): is the passed tab the current tab?
 ******************************************************************************/
bool TabManager::currentTab  (Tab *tab) const {
   ENTEX("TabManager::currentTab");
	quintptr p1 = reinterpret_cast<quintptr>(tab),
				p2 = reinterpret_cast<quintptr>(dynamic_cast<Tab *>(tw->currentWidget()));
	qd(4) << "comparing " << p1 << " == " << p2 << " (" << (p1 == p2) << ")";
	return p1 == p2;
}
/**************************************************************************//**
 * ResetMru(): reset the active (current) tab to the highest indexed tab.
 ******************************************************************************/
void TabManager::resetMru  () {
   ENTEX("TabManager::resetMru");

   int max = 0,
       target = -1;
   for (int i = 0, sz = tui.size(); i < sz; ++i) {
      if (tui[i] == nullptr)
			continue;
		int idx = tw->indexOf(tui[i]);
		if (idx >= max) {
			max = idx;
         target = i;  // tab index with highest tui index position
		}
   }

   mru = (target == -1) ? nullptr : tui[target];

   if (target == -1) {
      qd(7) << "reset mru to nullptr";
   }
   else {
      qd(7) << "reset mru to tui[" << target << "]" << tui[target];
   }
}
/**************************************************************************//**
 * Shutdown(): shutdown the current tab operation.
 ******************************************************************************/
void TabManager::shutdown  () const {
   Tab *tab = currentTab();
   if (tab == nullptr)
      return;
   tab->shutdown();
}
/**************************************************************************//**
 * Special(): is this tab special i.e. the '+' or 'x' tab?
 *
 * Parms    : w - a tabbed widget within the main window QTabWidget
 ******************************************************************************/
bool TabManager::special  (const QWidget * const w) const {
	quintptr p = reinterpret_cast<quintptr>(w);

	return p == reinterpret_cast<quintptr>(mw->getUi()->addTabWidget) ||
			 p == reinterpret_cast<quintptr>(m_closeTab);
}
/**************************************************************************//**
 * Tab(): returns tab from passed widget.
 *
 * Notes: make sure w is a member of the QTabWidget (tw)
 ******************************************************************************/
Tab *TabManager::tab  (QWidget *w) const {
   return w == nullptr ? nullptr : dynamic_cast<Tab *>(w);
}
/**************************************************************************//**
 * TabChanged(): response to QTabWidget::currentChanged() signal.
 *
 * Notes       : Called when a tab is removed, even if current did NOT change.
 *               Number of Tabs
 *               --------------
 *               0 -> never ('+' tab always exists)
 *               1 -> '+' tab
 *               2 -> never ('x' tab comes and goes with first/last tab)
 *               n -> sure (includes '+' and 'x' tabs)
 *               When will this method get called?
 *                1. app selects a new tab when (n > -1)  [setCurrentIndex()]
 *                2. user selects a special tab (n = 1, n > 2)
 *                3. user selects a new tab when (n tabs)
 *
 *                4. app adds first tab when (n = 0)
 *                5. user inserts first tab when (n = 1)
 *                6. app inserts 'x' tab when (n = 2)
 *                7. user inserts another tab when (n tabs)
 ******************************************************************************/
void TabManager::tabChanged  (int _index) {
   ENTEX2("TabManager::tabChanged", 0);
   if (m_noRecurse) {
      m_noRecurse = false;
      return;
   }

   if (!m_tabAdded) {  // selected
      switch (tw->count()) {
         case 0:
         case 1:
         case 2: m_noRecurse = true;
                 tw->setCurrentIndex(-1);
                 return;
        default: if (special(tw->widget(_index))) {
                    m_noRecurse = true;
                    tw->setCurrentIndex(mru == nullptr ? -1 : tw->indexOf(mru));  //
                    return;
                 }
                 mru->getParms()->ripContext();
                 mru = tab(tw->widget(_index));
                 mru->getParms()->burnContext();
      }
   }
   else {
      m_tabAdded = false;
      switch (tw->count()) {
         case 0:
         case 1:
         case 2: m_noRecurse = true;
                 tw->setCurrentIndex(-1);
                 return;
         case 3: mru = tab(tw->widget(_index));
                 mru->getParms()->ripContext();
                 return;
        default: mru->getParms()->ripContext();
                 mru = tab(tw->widget(_index));
                 mru->getParms()->burnContext();
      }
   }

   tabsDebug("tabChanged()");

   if (!mru->busy())
      emit statusRight(tr("Ready."));

}  // tabChanged()
/**************************************************************************//**
 * TabClose(): close the current tab.  [slot]
 *
 * Notes     : Handle these cases when user triggers 'x' tab button:
 *                1. user closes last tab when (n = 3)
 *                2. user closes a tab when (n > 3)
 ******************************************************************************/
void TabManager::tabClose  () {
   ENTEX2("TabManager::tabClose", 7);

   if (tw->count() < 3)
		return;

   if (!Singleton<SettingsGlobal>::instance()->confirm(CB_BEFORE))
      return;

   shutdown();  // current tab threads

   int index = tw->currentIndex();

   quintptr ptr = reinterpret_cast<quintptr>(tw->widget(index));  // pointer to delete

   // take a major load off tabChanged()
   disconnect(tw, SIGNAL(currentChanged(int)), mw, SLOT(on_resultsTabWidget_currentChanged(int)));

   tw->removeTab(index);

   int i = tabFindPtr(ptr);
   delete tui[i];
   tui[i] = nullptr;

   if (tw->count() == 2) {
      closeTabHide();
      if (Singleton<SettingsGlobal>::instance()->getUi()->lastCheckBox->isChecked()) {
         emit closeMainWindow();
         return;
      }
   }

   resetMru();

   tw->setCurrentIndex(mru == nullptr ? -1 : tw->indexOf(mru));

   connect(tw, SIGNAL(currentChanged(int)), mw, SLOT(on_resultsTabWidget_currentChanged(int)));

   if (mru != nullptr) {
      mru->getParms()->burnContext();
      if (!mru->busy())
         emit statusRight(tr("Ready."));
   }

   tabsDebug("tabClose()");

}  // tabClose()
/**************************************************************************//**
 * TabClose(): remove the indicated tab and delete underlying tab object.
 *
 * Notes     : Handle these cases, mostly from user context menu close:
 *                1. user wants to close current tab
 *                2. user closes another tab when (n > 3)
 *                3. destructor closes all tabs
 ******************************************************************************/
void TabManager::tabClose  (Tab *tab, bool force) {
   ENTEX2("TabManager::tabClose", 7);

   if (!force && currentTab(tab)) {
      tabClose();
      return;
   }

   if (tw->count() < 3)
      return;

   if (!force)  // destructor uses force
      if (!Singleton<SettingsGlobal>::instance()->confirm(CB_BEFORE))
         return;

   tab->shutdown();

   quintptr ptr = reinterpret_cast<quintptr>(tab);  // pointer to delete

   int index = tw->indexOf(tab);

   // take a load off tabChanged()
   disconnect(tw, SIGNAL(currentChanged(int)), mw, SLOT(on_resultsTabWidget_currentChanged(int)));

   tw->removeTab(index);

   int i = tabFindPtr(ptr);
   delete tui[i];
   tui[i] = nullptr;

   connect(tw, SIGNAL(currentChanged(int)), mw, SLOT(on_resultsTabWidget_currentChanged(int)));

   tabsDebug("tabClose()");

}  // tabClose()
/**************************************************************************//**
 * TabCreate(): create a tab.  Show close tab on first creation.  [slot]
 *
 * Notes    : Inserting new tab at index less than or equal to current index
 *            will increment current index, but keep current page.
 ******************************************************************************/
void TabManager::tabCreate  () {
   ENTEX("TabManager::tabCreate");

   int idx = ::getAvailIdx(tui);

   tui[idx] = new Tab(this);

   if (tw->count() == 1)
      closeTabShow();

   QString label = tabLabel(idx + 1);

   m_tabAdded = true;
   int index = tw->insertTab(idx + 1, tui[idx], QIcon(SM_ICON), label);  // may eccs

   tw->setCurrentIndex(index);  // eccs

	tui[idx]->getUi()->layoutPushButton->setAutoDefault(true);
   tui[idx]->getUi()->refreshPushButton->setVisible(false);
   tui[idx]->setTabToolTip();
}
/**************************************************************************//**
 * TabFind(): execute search on current tab.  [slot]
 *
 * Parms    : p - find parameters
 *
 * Notes    : Number of Tabs
 *            --------------
 *            0 -> never ('+' tab always exists)
 *            1 -> '+' tab
 *            2 -> never ('x' tab comes and goes with first/last tab)
 *            n -> sure (includes '+' and 'x' tabs)
 ******************************************************************************/
void TabManager::tabFind  () {
   ENTEX("TabManager::tabFind");

   SettingsGlobal *sg = Singleton<SettingsGlobal>::instance();

   if (tw->count() < 3 ||                                     // '+' tab
      (sg->check(CB_NEW)  && currentTab()->getRowCount()) ||  // n tabs
      (sg->check(CB_BUSY) && currentTab()->busy())) {         // n tabs
      tabCreate();
	}
   else if (currentTab()->getRowCount()) {
      if (!sg->confirm(CB_OVERWRITE))
         return;
   }

   currentTab()->findInit();
}
/**************************************************************************//**
 * TabFindPtr(): locate tab pointer in tui container.
 *
 * Parms       : ptr - to tab in the tab widget
 *
 * Notes       : since new tabs assigned into tui vector, it will be located
 ******************************************************************************/
int TabManager::tabFindPtr  (quintptr ptr) const {
   ENTEX("TabManager::tabFindPtr");

   int i = 0;
   for (int sz = tui.size(); i < sz; ++i)
      if (reinterpret_cast<quintptr>(tui[i]) == ptr)
         break;

   return i;
}
/**************************************************************************//**
 * TabLabel(): make a useful label for tab with current time and date.
 *
 * Parms     : tag - serial number to be displayed in tab label
 *
 * Returns   : nice label like "&4 20:37"
 ******************************************************************************/
QString TabManager::tabLabel  (int tag) {
   ENTEX("TabManager::tabLabel");

   QString s = tr("&%1 ").arg(tag);
   s += QDateTime::currentDateTime().toString("h:mm");

   return s;
}
/**************************************************************************//**
 * TabsDebug(): display tabs, active tab and current tab.
 ******************************************************************************/
void TabManager::tabsDebug  (const QString &_f) const {
   ENTEX("TabManager::tabsDebug");

	QString s;
   for (int i = 0, sz = tui.size(); i < sz; ++i) {
      if (tui[i] != nullptr) {
         qd(2) << "tui[" << i << "] p-" << tui[i]->getParms();
			if (tui[i] == tw->currentWidget())
            s += "c";
         if (tui[i] == mru)
            s += "m";
			s += QString("%1").arg(i);
		}
	}
   qd(7) << _f << "TABS:" << s;
}
