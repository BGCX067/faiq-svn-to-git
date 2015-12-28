/**************************************************************************//**
 * NAME:      main.cpp
 *
 * PURPOSE:   Searchmonkey program entry point.
 *
 * DATE:      October 18, 2009
 *
 * NOTES:     See util.h for most of the application's constants.
 ******************************************************************************/
#include "debug.h"

#include <iostream>

#include <QApplication>
#include <QCoreApplication>

#include "mainwindow.h"
#include "settingsglobal.h"

#ifndef QT_NO_CONCURRENT

bool LEVEL[DEBUG_LEVELS];

int main  (int argc, char *argv[]) {

   LEVEL[0] = 1;  // "core logging" w/ error paths
   LEVEL[1] = 1;  // "always" w/ infrequent events
   LEVEL[2] = 1;  // "more detail" w/ infrequent subsystem events
   LEVEL[3] = 0;  // "heavy detail"
   LEVEL[4] = 0;  // "ridiculous detail"
	LEVEL[5] = 1;  // Parms
	LEVEL[6] = 1;  // threads
	LEVEL[7] = 1;  // tabs
   LEVEL[8] = 1;  // grep
	LEVEL[9] = 1;  // timing info
   LEVEL[10] = 0;  // Matcher w/ AtomicQueue
   LEVEL[11] = 0;  // library code (util.cpp...)
   LEVEL[12] = 1;  // temporary debug

   QApplication app(argc, argv);

   QCoreApplication::setOrganizationName("Embedded IQ");
   QCoreApplication::setOrganizationDomain("embeddediq.com");
   QCoreApplication::setApplicationName("Searchmonkey");

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
