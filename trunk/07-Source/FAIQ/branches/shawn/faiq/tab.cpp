/**************************************************************************//**
 * NAME:    tab.cpp
 *
 * PURPOSE: Implements tab widgets.
 *
 * DATE:    November 12, 2012
*******************************************************************************/
#include "debug.h"

#include <QPalette>
#include <QProgressBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTime>
#include <QTextCursor>
#include <QTextEdit>
#include <QWidget>

#include "stabwidget.h"
#include "tab.h"
/**************************************************************************//**
 * Tab(): create a new tab.
 ******************************************************************************/
Tab::Tab  (MainWindow *_mw)
:
alarmFire(false),
alarmSec(false),
zoneDestroyed(false),
zoneSaved(false),
casualties(0),
control(0),
escalations(0),
eta(0),
evac(0),
trapAudible(0),
trapAutomaticDoor(0),
trapDirectional(0),
trapElectrical(0),
trapEmergency(0),
trapFireInvestigate(0),
trapSecInvestigate(0),
trapSprinkler(0),
autoTimer(new QTime),
etaTimer(new QTime),
mw(_mw),
tw(mw->getUi()->tabWidget),  // shortcut ptr
ui(new Ui::Tab),
tb(tw->getTabBar())          // shortcut ptr
{
   ui->setupUi(this);

   ui->casualtiesLabel->hide();
   ui->casualtiesLineEdit->hide();
   ui->etaLabel->hide();
   ui->etaLineEdit->hide();
   ui->finalLineEdit->hide();

   MainWindow::progressBarReset(ui->pb);

   static int cnt = 0;
   ui->zoneLineEdit->setText(QString("%1").arg(++cnt));
   id = QString("> ZONE %1: ").arg(cnt);
   con("Ready.");

   connect(ui->audibleAlarmButton, SIGNAL(clicked()), this, SLOT(on_audibleAlarmButton_clicked()));
   connect(ui->automaticDoorLocksButton, SIGNAL(clicked()), this, SLOT(on_automaticDoorLocksButton_clicked()));
   connect(ui->directionalIndicatorsButton, SIGNAL(clicked()), this, SLOT(on_directionalIndicatorsButton_clicked()));
   connect(ui->electricalShutdownButton, SIGNAL(clicked()), this, SLOT(on_electricalShutdownButton_clicked()));
   connect(ui->emergencyServicesButton, SIGNAL(clicked()), this, SLOT(on_emergencyServicesButton_clicked()));
   connect(ui->fireInvestigateButton, SIGNAL(clicked()), this, SLOT(on_fireInvestigateButton_clicked()));
   connect(ui->secInvestigateButton, SIGNAL(clicked()), this, SLOT(on_secInvestigateButton_clicked()));
   connect(ui->sprinklerSystemButton, SIGNAL(clicked()), this, SLOT(on_sprinklerSystemButton_clicked()));

   connect(this, SIGNAL(done()), mw, SLOT(done()));

   // two simulators
   pool.setMaxThreadCount(2);

   sims.fill(nullptr, 2);
   for (int i = 0; i < 2; ++i) {
      sims[i] = new Sim(i);
      connect(sims.at(i), SIGNAL(sensor(int)), this, SLOT(sensor(int)));
   }
   simInit();
}
/**************************************************************************//**
 * ~Tab(): cancel concurrent operations.
 ******************************************************************************/
Tab::~Tab  () {
   ENTEX2("Tab::~Tab", 0);

   simCancel();

   delete sims.at(0);
   delete sims.at(1);

   delete ui;
}
/**************************************************************************//**
 * Con(): write to info console.
 ******************************************************************************/
void Tab::con  (QString s) {
   //mw->getUi()->infoTextEdit->insertPlainText(mw->tmf() + id + s + "\n");
   cursorToTop();
   mw->getUi()->infoTextEdit->textCursor().insertText(mw->tmf() + id + s + "\n");
}
/**************************************************************************//**
 * CursorToTop(): move cursor to the top of the TextEdit.
 ******************************************************************************/
void Tab::cursorToTop  () {
   QTextCursor cursor = mw->getUi()->infoTextEdit->textCursor();
   cursor.movePosition(QTextCursor::Start);
   mw->getUi()->infoTextEdit->setTextCursor(cursor);
}
/**************************************************************************//**
 * DisableZone(): disable and hide zone buttons.
 ******************************************************************************/
void Tab::zoneDisable  (bool color) {

   ui->audibleAlarmButton->setEnabled(false);
   ui->automaticDoorLocksButton->setEnabled(false);
   ui->directionalIndicatorsButton->setEnabled(false);
   ui->electricalShutdownButton->setEnabled(false);
   ui->emergencyServicesButton->setEnabled(false);
   ui->sprinklerSystemButton->setEnabled(false);

   ui->audibleAlarmButton->hide();
   ui->automaticDoorLocksButton->hide();
   ui->directionalIndicatorsButton->hide();
   ui->electricalShutdownButton->hide();
   ui->emergencyServicesButton->hide();
   ui->sprinklerSystemButton->hide();

   ui->fireTextEdit1->setEnabled(color);
   ui->fireTextEdit2->setEnabled(color);
   ui->secTextEdit1->setEnabled(color);
   ui->secTextEdit2->setEnabled(color);
}
/**************************************************************************//**
 * EmergencyServicesArrival(): emergency services have arrived.
 ******************************************************************************/
void Tab::emergencyServicesArrival  () {
   ENTEX2("Tab::emergencyServicesArrival", 2);

   if (zoneDestroyed)
      return;

   zoneSaved = true;

   simCancel();

   con("EMERGENCY SERVICES HAVE ARRIVED:  You saved this zone!");

   qApp->processEvents();
   zoneDisable(GREEN);

   QString occupants = ui->occupantsLineEdit->text();
   con(QString("Remaining %1 occupants evacuated.").arg(occupants));
   ui->occupantsLineEdit->setText("0");

   ui->etaLineEdit->setText("0");

   setTabWindowColor(QColor(Qt::darkGreen));
   setTabTextColor(QColor(Qt::darkGreen));

   ui->finalLineEdit->show();

   emit done();
}
/**************************************************************************//**
 * Escalate(): escalate due to twin sensors for fire or security.
 ******************************************************************************/
void Tab::escalate  (int fire) {
   ENTEX2("escalate", 2);

   if (++escalations > 2)
      return;

   if (fire == FIRE) {
      ui->fireInvestigateButton->setEnabled(false);
      ui->fireInvestigateButton->hide();

      ui->electricalShutdownButton->setEnabled(true);
      ui->electricalShutdownButton->show();
      ui->sprinklerSystemButton->setEnabled(true);
      ui->sprinklerSystemButton->show();
   }
   else {
      ui->secInvestigateButton->setEnabled(false);
      ui->secInvestigateButton->hide();

      ui->automaticDoorLocksButton->setEnabled(true);
      ui->automaticDoorLocksButton->show();
   }

   if (escalations < 2) {
      ui->emergencyServicesButton->setEnabled(true);
      ui->emergencyServicesButton->show();
      autoTimer->start();
   }
   ui->directionalIndicatorsButton->setEnabled(true);
   ui->directionalIndicatorsButton->show();
   ui->audibleAlarmButton->setEnabled(true);
   ui->audibleAlarmButton->show();

   ui->pb->show();
}
/**************************************************************************//**
 * Flow(): simulate evacuation and control.
 ******************************************************************************/
void Tab::flow  () {
   ENTEX2("Tab::flow", 2);

   if (ui->etaLineEdit->isVisible()  &&  eta > 0)
      reduceETA();

   int rand = (::qrand() % 200) + 1;

   if ((::qrand() % 100) < 50) {
      if (!ui->audibleAlarmButton->isEnabled() || !ui->directionalIndicatorsButton->isEnabled())
         return;
      int occ = (rand + evac);
      casualties += (occ / 10); showCasualties();
      reduceOccupants(occ + (occ / 10));
   }
   else {
      ++casualties; showCasualties();
      spreadDamage(rand - control);
   }
}
/**************************************************************************//**
 * Sensorium(): process sensor signals.  [inlined slot]
 ******************************************************************************/
void Tab::sensorium  (int s) {
   ENTEX("Tab::sensorium");
   //qd(0) << "received sensor sig on object-" << this;

   ot();

   if (mw->getPause() || zoneSaved || zoneDestroyed)
      return;

   if (ui->emergencyServicesButton->isEnabled()) {
      if (autoTimer->elapsed() > 22000) {
         con("FAIQ system called Emergency Services automatically.");
         on_emergencyServicesButton_clicked();
      }
   }
   else if (eta  &&  etaTimer->elapsed() > eta) {
      emergencyServicesArrival();
      eta = 0;
   }

   if ((::qrand() % 100) < 90)  // throttle
      return;

   sense((s == FIRE) ? ui->fireTextEdit1 : ui->secTextEdit1,
         (s == FIRE) ? ui->fireTextEdit2 : ui->secTextEdit2, s);
}
/**************************************************************************//**
 * Sense(): process sensor signal.
 ******************************************************************************/
void Tab::sense  (QTextEdit *t1, QTextEdit *t2, int type) {
   ENTEX2("Tab::sense", 2);

   int sensor = 0;

   if (!t1->isEnabled()) {

      t1->setEnabled(true);

      if (type == FIRE)
         ui->fireInvestigateButton->setEnabled(true);
      else
         ui->secInvestigateButton->setEnabled(true);

      sensor = 1;
   }
   else if (!t2->isEnabled()) {

      t2->setEnabled(true);

      setAlarm(type);

      setTabTextColor(QColor(Qt::red));

      sensor = 2;

      escalate(type);
   }

   if (sensor)
      con(QString("%1 sensor number %2 tripped.").arg((type == FIRE) ? "Fire" : "Security").arg(sensor));

   if (getAlarm())
      flow();
}
/**************************************************************************//**
 * SetTabTextColor(): set tab text to noticeable color.
 ******************************************************************************/
void Tab::setTabTextColor  (QColor color) {
   ENTEX2("Tab::setTabTextColor", 2);

   tb->setTabTextColor(tw->indexOf(dynamic_cast<QWidget *>(this)), color);
}
/**************************************************************************//**
 * SetTabWindowColor(): set tab window text to noticeable color.
 ******************************************************************************/
void Tab::setTabWindowColor  (QColor color) {
   ENTEX2("Tab::setTabWindowColor", 2);

   QPalette palette;
   palette.setColor(QPalette::Normal, QPalette::WindowText, color);
   this->setPalette(palette);
}
/**************************************************************************//**
 * simCancel(): cancel threads.
 ******************************************************************************/
void Tab::simCancel  () {
   ENTEX2("Tab::simCancel", 2);

   sims.at(0)->cancel();
   sims.at(1)->cancel();

   while (!sims.at(0)->done());
   while (!sims.at(1)->done());
}
/**************************************************************************//**
 * SimInit(): initialize simulators.
 ******************************************************************************/
void Tab::simInit  () {
   ENTEX2("Tab::simInit", 0);

   for (int i = 0; i < 2; ++i) {
      qd(0) << "starting thread on Sim object...." << sims.at(i);

      sims.at(i)->setAutoDelete(false);

      pool.start(sims.at(i));
   }
   qd(0) << "active threads:" << pool.activeThreadCount();
}
/**************************************************************************//**
 * SpreadDamage(): spread damage.
 ******************************************************************************/
void Tab::spreadDamage  (int rand) {

   int max = ui->pb->maximum(),
       progress = ui->pb->value() + rand;

   ui->pb->setValue((progress > max) ? max : progress);

   if (progress >= max)
      zoneDestroy();
}
/**************************************************************************//**
 * zoneDestoy(): destroy zone.
 ******************************************************************************/
void Tab::zoneDestroy  () {
   ENTEX2("Tab::zoneDestroy", 2);

   if (zoneSaved)
      return;

   zoneDestroyed = true;

   simCancel();

   con("YOU JUST LOST THIS ZONE. Way to go champ.");

   zoneDisable(RED);

   int dead = ui->occupantsLineEdit->text().toInt();
   ui->occupantsLineEdit->setText("0");
   con(QString("Remaining %1 occupants have perished in the destruction.").arg(dead));
   casualties += dead; showCasualties();

   setTabWindowColor(QColor(Qt::red));
   setTabTextColor(QColor(Qt::red));

   QColor color(Qt::darkRed);
   QPalette palette;
   palette.setColor(QPalette::Normal, QPalette::Text, color);
   ui->finalLineEdit->setPalette(palette);
   ui->finalLineEdit->clear();
   ui->finalLineEdit->setText("DESTROYED.");
   ui->finalLineEdit->show();

   emit done();
}
/**************************************************************************//**
 * Function Group [slots].
 ******************************************************************************/
void Tab::on_audibleAlarmButton_clicked() {
   //if (trapAudible++)
      //return;
   ui->audibleAlarmButton->setEnabled(false);

   if (ui->directionalIndicatorsButton->isEnabled()) {
      int rand = (::qrand() % 15) + 1;
      casualties += rand; showCasualties();
      reduceOccupants(rand);

      con(QString("You caused %1 people to be trampled to death! You need more training.").arg(rand));
      con("Auto-engaging directional floor indicators...");

      ui->directionalIndicatorsButton->setEnabled(false);
   }
   else evac += 16;
}
void Tab::on_automaticDoorLocksButton_clicked() {
   if (trapAutomaticDoor++)
      return;

   ui->automaticDoorLocksButton->setEnabled(false);

   if (alarmFire) {
      int rand = (::qrand() % 15) + 1;
      casualties += rand; showCasualties();
      reduceOccupants(rand);

      con(QString("Hey buddy, you just trapped %1 occupants in a fire zone!").arg(rand));
   }
   else control += 4;
}
void Tab::on_directionalIndicatorsButton_clicked() {
   if (trapDirectional++)
      return;
   ui->directionalIndicatorsButton->setEnabled(false);

   control += 4;
}
void Tab::on_electricalShutdownButton_clicked() {
   if (trapElectrical++)
      return;
   ui->electricalShutdownButton->setEnabled(false);

   if (occupants()) {
      int rand = (::qrand() % 15) + 1;
      casualties += rand; showCasualties();
      reduceOccupants(rand);

      con(QString("Ooops! Hey ace, %1 people just fell down the stairs in the dark.").arg(rand));
   }
   else control += 4;
}
void Tab::on_emergencyServicesButton_clicked() {
   if (trapEmergency++)
      return;
   ui->emergencyServicesButton->setEnabled(false);
   ui->emergencyServicesButton->hide();
   ui->casualtiesLabel->show();
   ui->casualtiesLineEdit->show();
   ui->etaLabel->show();
   ui->etaLineEdit->show();

   ui->casualtiesLineEdit->setText(QString("%1").arg(casualties));

   eta = ((::qrand() % 121) + 60) * 1000;
   ui->etaLineEdit->setText(QString("%1").arg(eta/1000));
   con("Emergency Services are on the way. ETA: " + QString("%1 seconds.").arg(eta/1000));
   etaTimer->start();
}
void Tab::on_fireInvestigateButton_clicked() {
   if (trapFireInvestigate++)
      return;

   ui->fireInvestigateButton->hide();
   ui->fireInvestigateButton->setEnabled(false);
   con("Investigating fire sensor...");

   if ((::qrand() % 100) < 75) {
      con("\"Small fire detected. Doesn't look too bad. We're going for donuts.\"");
      return;
   }
   if (!alarmSec)
      setTabTextColor(QColor(Qt::darkGreen));
   ui->fireTextEdit1->setEnabled(false);
   con("Fire investigation team report:  CEO SMOKING IN OFFICE AGAIN, FALSE ALARM.");
}
void Tab::on_secInvestigateButton_clicked() {
   if (trapSecInvestigate++)
      return;

   ui->secInvestigateButton->hide();
   ui->secInvestigateButton->setEnabled(false);
   con("Investigating security sensor...");

   if ((::qrand() % 100) < 75) {
      con("\"Suspicious visitors in lobby. On our way to check IDs.\"");
      return;
   }
   if (!alarmFire)
      setTabTextColor(QColor(Qt::darkGreen));
   ui->secTextEdit1->setEnabled(false);
   con("Security Investigation team report:  SQUIRREL, FALSE ALARM.");
}
void Tab::on_sprinklerSystemButton_clicked() {
   if (trapSprinkler++)
      return;
   ui->sprinklerSystemButton->setEnabled(false);

   bool occ = occupants(),
        esb = ui->electricalShutdownButton->isEnabled();

   if (occ) {
      int rand = (::qrand() % 7) + 1;
      casualties += rand; showCasualties();
      reduceOccupants(rand);

      con(QString("Whoa! Hey pal, %1 people just slipped down the stairs!").arg(rand));
   }
   if (esb) {
      int rand = (::qrand() % 7) + 1;
      casualties += rand; showCasualties();
      reduceOccupants(rand);

      con(QString("Ouch! Hey bub, you just electrocuted %1 occupants!").arg(rand));
   }
   control += 4;
}
