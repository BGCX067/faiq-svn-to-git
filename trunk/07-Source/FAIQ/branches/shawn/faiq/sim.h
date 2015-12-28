/**************************************************************************//**
 * NAME:    sim.h
 *
 * PURPOSE: Definitions for simulator class.
 *
 * DATE:    November 20, 2012
*******************************************************************************/
#ifndef SIM_H
#define SIM_H
#include "debug.h"

#include <QtGlobal>

#include <QCoreApplication>
#include <QObject>
#include <QRunnable>
#include <QString>
#include <QThread>
#include <QTime>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class Sim;

class Sim : public QThread, public QRunnable {
Q_OBJECT
private:
public:
   inline Sim(int _s) { ENTEX2("Sim::sim", 0); m_cancel = m_done = false; s = _s; }
   inline virtual ~Sim() {}
   inline void cancel() { m_cancel = true; }
   inline bool done() const { return m_done; }
   void run() {
      ENTEX2("Sim::run", 0);

      while (!m_cancel) {

         QThread::msleep((::qrand() % 2001) + 1000);

         emit sensor(s);

         qApp->processEvents();
      }
      m_done = true;
   }
private:
   bool m_cancel;
   bool m_done;
   int s;
signals:
   void sensor(int s);
};

#endif
