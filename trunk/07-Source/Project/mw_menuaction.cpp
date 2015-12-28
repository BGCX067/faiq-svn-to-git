/**************************************************************************//**
 * NAME:    mw_menuaction.cpp
 *
 * PURPOSE: Searchmonkey actions (implementation of class MainWindow).
 *
 * DATE:    September 23, 2009
*******************************************************************************/
#include "debug.h"

#include <QAction>

#include "mainwindow.h"
#include "tabmanager.h"

void MainWindow::createActions() {
   ENTEX2("MainWindow::createActions", 2);

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcut(tr("Ctrl+Q"));
	exitAct->setToolTip(tr("Exit application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	layoutAct = new QAction(tr("Hor&izontal"), this);
	layoutAct->setShortcut(tr("Ctrl+I"));
	layoutAct->setToolTip(tr("Flip it"));
	connect(layoutAct, SIGNAL(triggered()), this,
								  SLOT(on_tbarActionLayout_triggered()));

	logHistoryAct = new QAction(tr("&History"), this);
	logHistoryAct->setShortcut(tr("Ctrl+H"));
	logHistoryAct->setToolTip(tr("Log history"));
	connect(logHistoryAct, SIGNAL(triggered()), this,
								  SLOT(on_tbarActionLogHistory_triggered()));
	/*
	newAct = new QAction(tr("&New"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setToolTip(tr("Create new file"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));
	*/
	newTabAct = new QAction(tr("+"), this);
	newTabAct->setShortcuts(QKeySequence::AddTab);
	newTabAct->setToolTip(tr("New tab"));
	connect(newTabAct, SIGNAL(triggered()), tm, SLOT(tabCreate()));
	/*
	openAct = new QAction(tr("&Open..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
	*/
	resultsBrowserAct = new QAction(tr("&Results"), this);
	resultsBrowserAct->setShortcut(tr("Ctrl+R"));
	resultsBrowserAct->setToolTip(tr("Results browser"));
	connect(resultsBrowserAct, SIGNAL(triggered()), this,
										SLOT(on_tbarActionResultsBrowser_triggered()));

	stateAct = new QAction(tr("&Advanced"), this);
	stateAct->setShortcut(tr("Ctrl+Q"));
	stateAct->setToolTip(tr("Simple / Power search"));
	connect(stateAct, SIGNAL(triggered()), this,
							SLOT(on_tbarActionState_triggered()));

	settingsAct = new QAction(tr("&Settings"), this);
	settingsAct->setShortcut(tr("Ctrl+S"));
	settingsAct->setToolTip(tr("Tweak it"));
	connect(settingsAct, SIGNAL(triggered()), this,
                        SLOT(on_tbarActionSettings_triggered()));

   logHistoryAct->setVisible(false);
   resultsBrowserAct->setVisible(false);
}
