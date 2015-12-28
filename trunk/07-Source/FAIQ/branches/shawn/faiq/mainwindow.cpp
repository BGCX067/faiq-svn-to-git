/******************************************************************************/
/*! \file :   mainwindow.cpp
 *
 * PURPOSE:   FAIQ main window implementation of class MainWindow.
 *
 * DATE:      November 19, 2012
 *
 * PLATFORM:  Qt 4.7.0 (32-bit)
 *
 * NOTES:     Qt uses implicit sharing (shared data with reference counting and
 *            copy-on-write) for many classes, including QList and QString.
 *            Therefore, these classes may be, and are, passed by value with no
 *            concern for copying overhead.
 *
 *            Qt automatically deletes objects subclassed from QRunnable, and,
 *            usually, we let it.
 *
 *            Still a 32-bit app even if built on Win7x64.  Would have to re-
 *            build on 64-bit MSVC.
*******************************************************************************/
#include "debug.h"

#include <QtGUI>

#include "mainwindow.h"
#include "stabwidget.h"
#include "tab.h"

#include "ui_tab.h"
/**************************************************************************//**
 * MainWindow(): construct the main window.
 ******************************************************************************/
MainWindow::MainWindow  ()
:
ui(new Ui::MainWindow),
overallTimer(new QTime),
paused(true),
zones(0)
{
   ENTEX2("MainWindow::MainWindow", 0);

   overallTimer->start();

   QDateTime dt(QDate::currentDate());
   ::qsrand(dt.toTime_t());

   ui->setupUi(this);

   ui->actionExit->setEnabled(false);

   ui->restartButton->hide();

   tw = ui->tabWidget;     // shortcut
   tb = tw->getTabBar();   // shortcut

   setupTabWidget();

   qDebug() << QDate::currentDate() << " " << QTime::currentTime()
            << " +++++++++++++++++++++++++++++++++++++++++++++++";
   qd(0) << "level 0 msg";
   qd(1) << "level 1 msg";
   qd(2) << "level 2 msg";

   qd(0) << "main thread " << QThread::currentThread();

   //createActions();

   setWindowTitle(tr("FAIQ - Fire and Infiltration Quashing System"));
   setWindowFlags(Qt::WindowMinMaxButtonsHint |
                  Qt::Window |
                  Qt::CustomizeWindowHint |
                  Qt::WindowCloseButtonHint);

   connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
   connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(pause()));
   connect(ui->restartButton, SIGNAL(clicked()), this, SLOT(restart()));
   connect(ui->startButton, SIGNAL(clicked()), this, SLOT(start()));

   ui->clearButton->setEnabled(true);
   ui->pauseButton->setEnabled(false);
   ui->restartButton->setEnabled(false);
   ui->startButton->setEnabled(true);

   connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(clear()));
   connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pause()));
   connect(ui->actionRestart, SIGNAL(triggered()), this, SLOT(restart()));
   connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(start()));

   ui->actionClear->setEnabled(true);
   ui->actionPause->setEnabled(false);
   ui->actionRestart->setEnabled(false);
   ui->actionStart->setEnabled(true);

   tabs.fill(nullptr, NUMZONES);

   createTabs();
   initState();

   ui->actionExit->setEnabled(true);
}
/**************************************************************************//**
 * CloseEvent(): Last chance to get in pre-shutdown tasks.
 ******************************************************************************/
void MainWindow::closeEvent  (QCloseEvent *event) {
   ENTEX("MainWindow::closeEvent");

   for (int i = 0; i < NUMZONES; ++i) {

      tw->removeTab(i);

      delete tabs.at(i);
   }
   event->accept();
}
/**************************************************************************//**
 * createTabs(): create tabs.
 ******************************************************************************/
void MainWindow::createTabs  () {
   ENTEX2("Tab::tabCreate", 0);

   for (int i = 0; i < NUMZONES; ++i) {

      QString label = QString("Zone %1").arg(i + 1);

      tw->insertTab(i, tabs[i] = new Tab(this), QIcon(), label);
   }
   tw->setCurrentIndex(0);
}
/**************************************************************************//**
 * InitState(): Assist constructor and Restart button with original config.
 ******************************************************************************/
void MainWindow::initState  () {
   ENTEX2("MainWindow::initState", 2);
   static int constructed = 0;

   if (constructed) {
      ui->restartButton->setEnabled(false);
      ui->infoTextEdit->clear();
   }

   for (int i = 0; i < NUMZONES; ++i) {

      Ui::Tab *tui = tabs.at(i)->getUi();  // shortcut

      tui->audibleAlarmButton->setEnabled(false);
      tui->automaticDoorLocksButton->setEnabled(false);
      tui->directionalIndicatorsButton->setEnabled(false);
      tui->electricalShutdownButton->setEnabled(false);
      tui->emergencyServicesButton->setEnabled(false);
      tui->emergencyServicesButton->show();
      tui->fireInvestigateButton->setEnabled(false);
      tui->fireInvestigateButton->show();
      tui->fireTextEdit1->setEnabled(false);
      tui->fireTextEdit2->setEnabled(false);
      tui->etaLabel->hide();
      tui->etaLineEdit->hide();
      tui->secInvestigateButton->setEnabled(false);
      tui->secInvestigateButton->show();
      tui->secTextEdit1->setEnabled(false);
      tui->secTextEdit2->setEnabled(false);
      tui->sprinklerSystemButton->setEnabled(false);

      tui->occupantsLineEdit->setText(QString("%1").arg(1000));

      progressBarReset(tui->pb);
   }
   for (int i = 0; i < NUMZONES; ++i) {
      Tab *t = tabs.at(i);  // shortcut

      t->alarmFire = t->alarmSec = false;
      t->control = 0;
      t->casualties = 0;
      t->escalations = 0;
      t->eta = 0;
      t->evac= 0;
      t->trapAudible = 0;
      t->trapAutomaticDoor = 0;
      t->trapDirectional = 0;
      t->trapElectrical = 0;
      t->trapEmergency = 0;
      t->trapFireInvestigate = 0;
      t->trapSecInvestigate = 0;
      t->trapSprinkler = 0;
      t->zoneDestroyed = t->zoneSaved = false;

      progressBarReset(t->getUi()->pb);

      delete t->autoTimer;
      t->autoTimer = new QTime;

      delete t->etaTimer;
      t->etaTimer = new QTime;
   }

   paused = true;
   zones = 0;
   statusBar()->showMessage("Ready.");

   if (constructed++)
      ui->restartButton->setEnabled(true);
}
/**************************************************************************//**
 * KeyPressEvent(): Block keyboard!
 ******************************************************************************/
void MainWindow::keyPressEvent  (QKeyEvent *event) {
   ENTEX("MainWindow::keyPressEvent");
   qd(2) << QString("got a key: 0x%1").arg(event->key(), 0, 16);

   switch (event->key()) {

      case Qt::Key_Space:
      case Qt::Key_Enter:
      case Qt::Key_Return:
         ;
         break;

      case Qt::Key_Escape:
         ;
         break;

      case Qt::Key_Down:
      case Qt::Key_Up:
         ;
         break;

      default:
         break;
   }
   event->accept();
}
/**************************************************************************//**
 * KeyReleaseEvent(): Block keyboard!
 ******************************************************************************/
void MainWindow::keyReleaseEvent  (QKeyEvent *event) {
   switch (event->key()) {

      case Qt::Key_Space:
      case Qt::Key_Enter:
      case Qt::Key_Return:
         ;
         break;

      case Qt::Key_Escape:
         ;
         break;

      case Qt::Key_Down:
      case Qt::Key_Up:
         ;
         break;

      default:
         break;
   }
   event->accept();
}
/**************************************************************************//**
 * Pause(): pause application.  [slot]
 ******************************************************************************/
void MainWindow::pause  () {
   ui->pauseButton->setEnabled(false);
   ui->actionPause->setEnabled(false);

   ui->startButton->setEnabled(true);
   ui->actionStart->setEnabled(true);

   ui->restartButton->setEnabled(false);
   ui->actionRestart->setEnabled(false);

   paused = true;

#if 1
   for (int i = 0; i < NUMZONES; ++i) {

      Ui::Tab *tui = tabs.at(i)->getUi();  // shortcut

      buttonPause(tui->audibleAlarmButton, i);
      buttonPause(tui->automaticDoorLocksButton, i);
      buttonPause(tui->directionalIndicatorsButton, i);
      buttonPause(tui->electricalShutdownButton, i);
      buttonPause(tui->emergencyServicesButton, i);
      buttonPause(tui->fireInvestigateButton, i);
      buttonPause(tui->secInvestigateButton, i);
      buttonPause(tui->sprinklerSystemButton, i);

      buttons.append(v);
      v.clear();
   }
}
/**************************************************************************//**
 * ButtonPause(): pause button.
 ******************************************************************************/
void MainWindow::buttonPause  (QPushButton *button, int i) {
   QColor color(Qt::blue);
   QPalette palette;
   palette.setColor(QPalette::ButtonText, color);

   if (button->isEnabled()) {
      button->setPalette(palette);
      button->setEnabled(false);
      v.append(button);
   }
}
#endif
/**************************************************************************//**
 * ButtonsStart(): start buttons.
 ******************************************************************************/
void MainWindow::buttonsStart  () {
   QColor color(Qt::black);
   QPalette palette;
   palette.setColor(QPalette::Normal, QPalette::Text, color);

   for (int i = 0; i < NUMZONES; ++i) {
      for (int j = 0; j < buttons.at(i).size(); ++j) {
         buttons[i][j]->setEnabled(true);
         buttons[i][j]->setPalette(palette);
      }
   }
   for (int i = 0; i < NUMZONES; ++i) {
      buttons[i].clear();
   }
   buttons.clear();
}
/**************************************************************************//**
 * Prestart(): handle state where all zones have completed.  [inlined slot]
 ******************************************************************************/
void MainWindow::prestart  () {
   ui->pauseButton->setEnabled(false);
   ui->startButton->setEnabled(false);
}
/***************************************************************************//**
* ProgressBarReset(): reset a progress bar.
******************************************************************************/
void MainWindow::progressBarReset  (QProgressBar *pb) {
   pb->hide();
   pb->reset();
   pb->setRange(0, 1000);
   pb->setValue(0);
}
/**************************************************************************//**
 * SetupTabWidget(): setup '+' tab and tab bar context menu stuff.
 ******************************************************************************/
void MainWindow::setupTabWidget  () {
   ENTEX("MainWindow::setupTabWidget");

   tw->setCurrentIndex(-1);
   tw->setMovable(false);
   tw->setTabsClosable(false);
   tw->setUsesScrollButtons(false);
}
/**************************************************************************//**
 * Start(): start (or resume) application.  [slot]
 ******************************************************************************/
void MainWindow::start  () {
   ui->startButton->setEnabled(false);
   ui->actionStart->setEnabled(false);

   ui->pauseButton->setEnabled(true);
   ui->actionPause->setEnabled(true);

   ui->restartButton->setEnabled(true);
   ui->actionRestart->setEnabled(true);

   paused = false;

   static int constructed = 0;
   if (constructed++)
      buttonsStart();
}
/**************************************************************************//**
 * Tab(): returns tab from passed widget.
 *
 * Notes: make sure w is a member of the QTabWidget (tw)
 ******************************************************************************/
Tab *MainWindow::tab  (QWidget *w) const {
   return w == nullptr ? nullptr : dynamic_cast<Tab *>(w);
}
/**************************************************************************//**
 * Tmf(): mm:ss.
 ******************************************************************************/
QString MainWindow::tmf  () const {
   int min = tm() / 60;
   int sec = tm() % 60;
   QString m, s;

   m = QString((min < 10) ? "0%1" : "%1").arg(min);
   s = QString((sec < 10) ? "0%1" : "%1").arg(sec);

   return m + ":" + s;
}
