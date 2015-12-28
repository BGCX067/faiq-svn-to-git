/**************************************************************************//**
 * NAME:    tab.h
 *
 * PURPOSE: Class definition for tab widget.
 *
 * DATE:    November 24, 2012
*******************************************************************************/
#ifndef TAB_H
#define TAB_H

#include <QtGlobal>

#include <QObject>
#include <QString>
#include <QThreadPool>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include "mainwindow.h"
#include "sim.h"

#include "ui_tab.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QString;
class QTabBar;
class QTabWidget;
class QTextEdit;
class QTime;
class QWidget;
QT_END_NAMESPACE

class MainWindow;
class Sim;
class STabWidget;
class Tab;

namespace Ui {
   class Tab;
}

const bool GREEN = false,
           RED = true;
const int FIRE = 0,
          SECURITY = 1;

class Tab : public QWidget {
Q_OBJECT
public:
   bool alarmFire;
   bool alarmSec;
   bool zoneDestroyed;
   bool zoneSaved;
   int casualties;
   int control;
   int escalations;
   int eta;
   int evac;
   int trapAudible;
   int trapAutomaticDoor;
   int trapDirectional;
   int trapElectrical;
   int trapEmergency;
   int trapFireInvestigate;
   int trapSecInvestigate;
   int trapSprinkler;
   QTime *autoTimer;
   QTime *etaTimer;

public:
   Tab(MainWindow *mw);
   virtual ~Tab();
   void con(const char * s) { con(QString(s)); }
   void con(QString s);
   void cursorToTop();
   void emergencyServicesArrival();
   void escalate(int);
   void flow();
   inline bool getAlarm() const { return alarmFire || alarmSec; }
   inline Ui::Tab *getUi() const { return ui; }

   inline int normalize(int minus, int val) const { return minus < val ? minus : val; }
   inline int occupants() const { return ui->occupantsLineEdit->text().toInt(); }
   inline void ot() const { ui->overallLineEdit->setText(mw->tmf()); }
   inline void reduceETA() { ui->etaLineEdit->setText(QString("%1").arg((eta - normalize(etaTimer->elapsed(), eta)) / 1000)); }
   inline void reduceOccupants(int rand) { ui->occupantsLineEdit->setText(QString("%1").arg(occupants() - normalize(rand, occupants()))); }
   inline void showCasualties() { ui->casualtiesLineEdit->setText(QString("%1").arg(casualties)); }

   void setTabTextColor(QColor);
   void setTabWindowColor(QColor);
   void simCancel();
   void simInit();
   void sense(QTextEdit *, QTextEdit *, int);
   void sensorium(int);
   inline void setAlarm(int alarm) { (alarm == FIRE) ? alarmFire = true : alarmSec = true; }
   void spreadDamage(int);
   void zoneDestroy();
   void zoneDisable(bool);

private:
   MainWindow * const mw;      ///< ptr to the main window app
   STabWidget *tw;             ///< shortcut ptr to main window tab widget
   Ui::Tab * const ui;         ///< ptr to this gui
   QTabBar *tb;
   QString id;
   QThreadPool pool;           ///< sim threads
   QVector<Sim *> sims;

signals:
   void done();

public slots:
   inline void sensor(int s) { sensorium(s); }

private slots:
   void on_audibleAlarmButton_clicked();
   void on_automaticDoorLocksButton_clicked();
   void on_directionalIndicatorsButton_clicked();
   void on_electricalShutdownButton_clicked();
   void on_emergencyServicesButton_clicked();
   void on_fireInvestigateButton_clicked();
   void on_secInvestigateButton_clicked();
   void on_sprinklerSystemButton_clicked();
};

#endif // TAB_H
