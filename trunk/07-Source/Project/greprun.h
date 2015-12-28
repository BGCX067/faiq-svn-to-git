/**************************************************************************//**
 * NAME:    greprun.h
 *
 * PURPOSE: Helper class, probably superfulous, to thread class Grep.
 *
 * DATE:    July 17, 2010
*******************************************************************************/
#ifndef GREPRUN_H
#define GREPRUN_H
#include "debug.h"

#include <QObject>
#include <QRunnable>

#include "atomicqueue.h"
#include "grep.h"
#include "tab.h"
#include "types.h"

class Grep;
class Parms;
class Tab;

class GrepRun : public QObject, public QRunnable {
Q_OBJECT
public:
	/***********************************************************************//**
    * GrepRun(): Construct.
	 ***************************************************************************/
   GrepRun  (const QString &path, Tab *tab)
   :
   m_done(false),
   m_path(path)
   {
      connect(this, SIGNAL(grepDone(int)), tab, SLOT(grepDone(int)));
      grep = new Grep(path, tab);
	}
   /***********************************************************************//**
    * ~GrepRun(): Destruct.
    ***************************************************************************/
   inline ~GrepRun  () {
      ENTEX2("GrepRun::~GrepRun", 0);

		delete grep;
	}
   /***********************************************************************//**
    * Cancel(): Cancel the grep operation.
    ***************************************************************************/
   void cancel  () {
      grep->cancel();
   }
   /***********************************************************************//**
    * Done(): Is run() done?
    ***************************************************************************/
   inline bool done() const {
      return m_done;
   }
   /***********************************************************************//**
    * GetGrep(): Get the grep pointer.
    ***************************************************************************/
   inline Grep *getGrep  () const {
      return grep;
   }
   /***********************************************************************//**
    * GetPath(): Get the path.
    ***************************************************************************/
   inline QString getPath  () const {
      return m_path;
   }
   /***********************************************************************//**
    * Run(): Execute a grep operation.
	 ***************************************************************************/
   void run  () {
      ENTEX2("GrepRun::run", 0);
		grep->grepInit();
      emit grepDone(grep->getBytes());
      m_done = true;
	}
private:
   bool m_done;
   Grep *grep;
   QString m_path;
signals:
   void grepDone(int);
};

#endif // GREPRUN_H
