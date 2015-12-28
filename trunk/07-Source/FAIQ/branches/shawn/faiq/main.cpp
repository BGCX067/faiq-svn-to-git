/**************************************************************************//**
 * NAME:      main.cpp
 *
 * PURPOSE:   FAIQ program entry point.
 *
 * DATE:      November 19, 2012
 ******************************************************************************/
#include "debug.h"

#include <iostream>

#include <QApplication>
#include <QCoreApplication>

#include "mainwindow.h"

#ifndef QT_NO_CONCURRENT

bool LEVEL[DEBUG_LEVELS];

int main  (int argc, char *argv[]) {

   LEVEL[0] = 1;  // "core logging" w/ error paths
   LEVEL[1] = 1;  // "always" w/ infrequent events
   LEVEL[2] = 1;  // "more detail" w/ infrequent subsystem events
   LEVEL[3] = 1;  // "heavy detail"
   LEVEL[4] = 0;  // "ridiculous detail"
   LEVEL[6] = 0;  // threads
   LEVEL[7] = 0;  // tabs
   LEVEL[8] = 0;  // grep
   LEVEL[9] = 0;  // timing info
   LEVEL[10] = 0;  // Matcher w/ AtomicQueue
   LEVEL[11] = 0;  // library code (util.cpp...)
   LEVEL[12] = 0;  // temporary debug

   QApplication app(argc, argv);

   QCoreApplication::setOrganizationName("UNO");
   QCoreApplication::setOrganizationDomain("unomaha.edu");
   QCoreApplication::setApplicationName("FAIQ");

   MainWindow w;
   w.show();

   return app.exec();
}

#else

int main() {
   qDebug()  << "Qt Concurrent is not yet supported on this platform.";
   std::cerr << "Qt Concurrent is not yet supported on this platform.";
}

#endif
