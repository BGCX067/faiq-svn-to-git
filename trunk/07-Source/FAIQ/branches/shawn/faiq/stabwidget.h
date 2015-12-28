/**************************************************************************//**
 * NAME:      stabwidget.h
 *
 * PURPOSE:   Provides a subclass with access to tab bar.
 *
 * DATE:      November 23, 2012
*******************************************************************************/
#ifndef STABWIDGET_H
#define STABWIDGET_H

#include <QTabBar>
#include <QTabWidget>
#include <QWidget>

class STabWidget : public QTabWidget {
Q_OBJECT
public:
   STabWidget(QWidget *parent = 0) {}
   inline QTabBar *getTabBar() const { return this->tabBar(); }
};

#endif // STABWIDGET_H
