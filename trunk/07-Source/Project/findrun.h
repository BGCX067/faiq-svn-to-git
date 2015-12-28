/**************************************************************************//**
 * NAME:    findrun.h
 *
 * PURPOSE: Helper class, probably superfulous, to thread class Find.
 *
 * DATE:    July 17, 2010
*******************************************************************************/
#ifndef FINDRUN_H
#define FINDRUN_H
#include "debug.h"

#include <QObject>
#include <QRunnable>

#include "find.h"
#include "tab.h"

class Find;
class Parms;
class Tab;

class FindRun : public QObject, public QRunnable {
Q_OBJECT
public:
	/***********************************************************************//**
    * FindRun(): Construct.
	 ***************************************************************************/
    FindRun  (Tab *tab) {
      connect(this, SIGNAL(findDone(int)), tab, SLOT(findDone(int)));
      find = new Find(tab);
	}
   /***********************************************************************//**
    * FindRun(): Destruct.
    ***************************************************************************/
    ~FindRun  () {  /// this is called automatically because of autoDelete flag
      ENTEX2("FindRun::~FindRun", 0);
      delete find;
   }
	/***********************************************************************//**
    * Run(): Execute a find operation.
	 ***************************************************************************/
   void run  () {
      ENTEX2("FindRun::run", 0);
      int numFiles = find->findInit();
      emit findDone(numFiles);
   }

private:
	Find *find;
signals:
   void findDone(int);
};

#endif // FINDRUN_H
