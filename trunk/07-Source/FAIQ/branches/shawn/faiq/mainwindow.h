/*****************************************************************************/
/*! \file : mainwindow.h
 *
 * PURPOSE: Main program header file.
 *
 * DATE:    November 19, 2012
*******************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTime>
#include <QVector>

#include "stabwidget.h"

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QCloseEvent;
class QKeyEvent;
class QProgressBar;
class QPushButton;
class QTabBar;
class QTabWidget;
class QTime;
QT_END_NAMESPACE

class StabWidget;
class Tab;

namespace Ui {
   class MainWindow;
}

const int nullptr = 0,
          NUMZONES = 5;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
   MainWindow();
   inline virtual ~MainWindow() { delete ui; }
   void createTabs();
   void buttonPause(QPushButton *, int i);
   void buttonsStart();
   void initState();
   void prestart();
   static void progressBarReset(QProgressBar *pb);
   void setupTabWidget();
   Tab *tab(QWidget *w) const;
   inline int tm() const { return overallTimer->elapsed() / 1000; }
   QString tmf() const;

public:
   inline bool getPause() const { return paused; }
   inline Ui::MainWindow *getUi() const { return ui; }

private:
   STabWidget *tw;          ///< shortcut ptr to the tab widget
   Ui::MainWindow *ui;      ///< ptr to the main window ui
   QTabBar *tb;             ///< shortcut ptr to tab widget tab bar
   QTime *overallTimer;
   QVector<QPushButton *> v;
   QVector<QVector<QPushButton *> > buttons;
   QVector<Tab *> tabs;
   bool paused;
   int zones;

public slots:
   inline void clear() { ui->infoTextEdit->clear(); }
   inline void done() { if (++zones == NUMZONES) prestart(); }
   void pause();
   inline void restart() { initState(); }
   void start();

protected:
   void closeEvent(QCloseEvent *event);
   void keyPressEvent(QKeyEvent *event);
   void keyReleaseEvent(QKeyEvent *event);
};

#endif ///< MAINWINDOW_H
