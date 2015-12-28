/*****************************************************************************/
/*! \file : debug.h
 *
 * PURPOSE: Provides crude debug facilities for app.  See main.cpp.
 *
 * DATE:    September 28, 2009
*******************************************************************************/
#ifndef DEBUG_H
#define DEBUG_H

//#define QT_NO_DEBUG         // Qt defines this already on command-line in Release mode
//#define QT_NO_DEBUG_STREAM  // tempting, but I don't think we need this

// uncomment these two for release build
// #define QT_NO_DEBUG_OUTPUT    // buildout qDebug()
// #define QT_NO_WARNING_OUTPUT  // buildout qWarning()

#include <QDebug>
#include <QString>
#include <QThread>

const int DEBUG_LEVELS = 13;
extern bool LEVEL[DEBUG_LEVELS];

/*
#ifndef QT_NO_DEBUG
*/
#define qd(x) if (LEVEL[x]) qDebug() << ee.fn()
#define ENTEX(x) Entex ee((x))
#define ENTEX2(x,y) Entex ee((x),(y))
#define FUNC ee.fn()
/*
#else
#define qd(x) qDebug()
#define ENTEX(x)
#define ENTEX2(x,y)
#define FUNC ""
#endif
*/

class Entex {
public:
   inline Entex(const QString &_fname, int _level = -1)
   :
   level(_level),
   fname(_fname)
   {
      if (level == -1)
         return;
      if (level < -1  ||  level > (DEBUG_LEVELS - 1))
         level = 0;
      if (LEVEL[level]) {
         qDebug() << fn() << "entering...";
      }
   }
   inline virtual ~Entex() {
      if (level == -1)
         return;
      if (LEVEL[level]) {
         qDebug() << fn() << "...exiting";
      }
   }
   inline QString fn() const {
      return QObject::tr("%1(0x%2): ").arg(fname).arg(reinterpret_cast<quintptr>(QThread::currentThread()), 0, 16);
   }
private:
   int level;
   QString fname;
};

#endif // DEBUG_H
