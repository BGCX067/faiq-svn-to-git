/******************************************************************************/
/*! \file :   mainwindow.cpp
 *
 * PURPOSE:   Searchmonkey main window implementation of class MainWindow.
 *
 * DATE:      October 18, 2009
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
 *            Qt methods for QString and QRegExp only support index positions
 *            up to INT_MAX.  So Grep will not support lines larger than ~2 GB.
 *
 *            Still a 32-bit app even if built on Win7x64.  Would have to re-
 *            build on 64-bit MSVC.
 *
 Handy Qt Editor Colors
 ----------------------
 R   G   B
 128 000 128 purple
 128 128 000 brown
 000 000 255 blue
 197 128 000 orange
 128 000 000 dull red (label)
 *
 Original window color: hue = sat = 0; val = rbg = 240; alpha = 255
 *
 \todo 2.x.x
 -----------
 I anything tagged "todo" ("XXX" for more troublesome issues)
 II read up QFileInfoList in main loop, remove current dir parm? I don't know..., etc... (speed)
 III carefully remove ubuf from encoding.c BUT leave in looks_ucs16() (speed)
 a for lists, replace [] operator with at() (speed)
 b allow multiple folders per search?  multiple content?
 c need CLI interface
 d separate cases? (as in case-sensitive for find/grep?)
 e check out QWeb and QTimer
 f minimize to tray option?  (Qt System Tray Icon example)
 h regex/glob builder wizards
 k break ctor further into helper functions?
 l case sensitivity applies to both find AND grep (all or nothing)?
 m Qt Application example
 n setting to ignore warnings about not being able to open files
 o feature to ignore std backup files (Windows vs Linux etc)
 p more Preset definitions needed (see ack)
 q lots more Qsettings can be saved upon close!
 r
 s allow a type for files with no extension; since require a regex cannot use
   QDir::entryList() which only handles globs; so we'd have to pull up all
   filenames and then filter by the glob for every desired extension PLUS
   the regex for files with no extension; could be expensive.  Anyway:
      ^[^.\n]+\.?$ matches no-extension files excluding hidden files, and:
      ^\.?[^.\n]+\.?$ matches same including hidden files.
 t support regexp in filename -- would require another adv option
 w need a checkbox before content to avoid time-consuming edits
 x search within results option (no added results)
 y allow user to choose color for lineno and highlighted content
 1 put target presets in main window
 4 invert at each stage or invert right before showFiles() (I think the latter; I think not)
 5 make tabs movable? "+" tab causes problem
 7 adjust vertical spacers in advanced mode to be more compact (preserve layout stretch)
 8 make a polymorphic quick/advanced state class
 9 maybe allow ^.ext in Filenames box to skip these extensions
 10 if frames appear around status bar elements:
    app.setStyleSheet("QStatusBar::item { border: 0px solid black }; ");
 11 when holding return to clear dialog error boxes, the last return signal
    trips a new search and then the cycle repeats
 12 pass mw ptr around as const MainWindow * const; maybe do for other ptrs
 14 resetActiveTab() should revert to the mru EVERY time; use a stack
 15 code the restore previous tabs stuff (enable a set visible the checkbox)
 16 auto-label tabs with mm/dd when tabs from different days present
 17 debug: consider tracing QThread::currentThread() AND QObject::thread() if they differ
 18 multiple select for open/delete context menu?
 19 stop/start/pause/status handlers for threads
 20 cache grep results
 21 floating tabs? open tab in new main window?
 22 restore last session option/default
 23 new instance i.e. child process (but only initial parent app saves config data)
 24 if cancel pressed and no search visible, them prompt if want to cancel mru search
 25 use Google Apps to give user secure logon to save profiles and searches
 PERFORMANCE
 -----------
 a stuck using ::makePrintable() in Matcher::countHits() so number of matching lines comes out
   the same as status from Grep::grep().  This is too expensive.
 b try using Highlighter::highlightBlock() to count hits with a member variable counter with
   get and reset methods instead of ::countHits()
 PLATFORMS
 ---------
 Tab context menu stuff, esp. open for unregistered file types; do we need contextOpenWith()?
 LINUX
 -----
 0 try to context open file where filename starts with blank char
 "" -> Open file not function if have blank character in first character file name.
 a sort by column: may have to implement manually
 >>> check it: void QHeaderView::setSortIndicator ( int logicalIndex, Qt::SortOrder order )
 >Sets the sort indicator for the section specified by the given logicalIndex in the direction specified by order, and removes the sort indicator from any other section that was showing it.
 >logicalIndex may be -1, in which case no sort indicator will be shown and the model will return to its natural, unsorted order. Note that not all models support this and may even crash in this case.
 >See also sortIndicatorSection() and sortIndicatorOrder().
 b symlinks are enabled -- test this
 c test Tab::contextOpen(), desktop services stuff...
 SCRIPTS
 -------
 1 cygwin script using setpriority.exe to raise priority of cc1plus.exe during Qt compiles
 TEST
 ----
 PRE-RELEASE (any version)
 -----------
 a Doxygen
 b keyboard navigation (tabs, ALT-key), mnemonics, tool tips
 0 Agile test cases
 1 test
 2 test case: ack -awl '^(Name|Comment|Keywords)\[.+\]' works; can we do it?
 3 test: regexp user must escape (\) these:  [\^$.|?*+()
 4 remove excess includes, asserts, debugs (qDebug), commented code
 5 remove whitespace; LINT etc...
 6 increase initial vector sizes
 INSTALL
 -------
 a migrate from Advance Installer to Wix
 b Xneed better default initial window size
 c Xsuppot 64-bit (windows 7) environment
 d change company name to "Embedded IQ"
 e recover verpatch and script
 USER REQUESTS
 -------------
 0 regex for filenames (not just wildcard)
 1 "open with notepad"
*******************************************************************************/
#include "debug.h"

#include <QtGUI>

//#include <QTest>

#include "highlighter.h"
#include "mainwindow.h"
#include "parms.h"
#include "settingsglobal.h"
#include "sortproxy.h"
#include "stabwidget.h"
#include "tab.h"
#include "tabmanager.h"
#include "util.h"
#include "about/about.h"
#include "license/license.h"
#include "presets/folders.h"
#include "presets/presets.h"

#include "ui_settingsglobal.h"
#include "ui_tab.h"
/**************************************************************************//**
 * MainWindow(): construct the main window.
 ******************************************************************************/
MainWindow::MainWindow  (QWidget *parent)
:
QMainWindow(parent),
m_ctorDone(false),
about(new About),
license(new License),
tm(nullptr),
ui(new Ui::MainWindow)
{
   ENTEX("MainWindow::MainWindow");

   ui->setupUi(this);  // eccs  when '+' tab added

   tw = ui->resultsTabWidget;  // shortcut
   tb = tw->getTabBar();       // shortcut
   tm = new TabManager(this);

   disconnect(ui->cancelPushButton, SIGNAL(clicked()), this, SLOT(on_cancelPushButton_clicked()));
   connect(ui->cancelPushButton, SIGNAL(clicked()), this, SLOT(on_cancelPushButton_clicked()), Qt::DirectConnection);

   ///< \todo-code these items
   ui->lcdLabel->setVisible(false);
   ui->lcdNumber->setVisible(false);
   ui->progressBar->setVisible(false);
   ui->mainToolBar->setVisible(false);
   ui->resetAdvButton->setVisible(false);
   Singleton<SettingsGlobal>::instance()->getUi()->restoreCheckBox->setVisible(false);
   Singleton<SettingsGlobal>::instance()->getUi()->restoreCheckBox->setEnabled(false);

   qRegisterMetaType<const WordCount *>("ConstWordCountPtr");
   qRegisterMetaType<Error>("Error");
   qRegisterMetaType<Operation>("Operation");
   qRegisterMetaType<QDir>("QDir");
   qRegisterMetaType<QFileInfoList>("QFileInfoList");
   qRegisterMetaType< QList<Line> >("QList<Line>");
   qRegisterMetaType<QMessageBox::Icon>("QMessageBox::Icon");
   qRegisterMetaType<Qt::SortOrder>("Qt::SortOrder");

   setupTabWidget();

   qDebug() << QDate::currentDate() << " " << QTime::currentTime()
				<< " +++++++++++++++++++++++++++++++++++++++++++++++";
	qd(0) << "level 0 msg";
	qd(1) << "level 1 msg";
	qd(2) << "level 2 msg";
	qd(3) << "level 3 msg";
	qd(4) << "level 4 msg";
	qd(5) << "level 5 msg";
	qd(6) << "level 6 msg";
	qd(7) << "level 7 msg";
	qd(8) << "level 8 msg";
	qd(9) << "level 9 msg";
   qd(10) << "level 10 msg";
   qd(11) << "level 11 msg";

	qd(0) << "main thread " << QThread::currentThread();

	setContextMenuPolicy(Qt::PreventContextMenu);  // allows model context menu

   connect(this, SIGNAL(tabFind()), tm, SLOT(tabFind()));

	createActions();
	ui->mainToolBar->addAction(layoutAct);  // order matters...
	ui->mainToolBar->addAction(logHistoryAct);
	ui->mainToolBar->addAction(resultsBrowserAct);
	ui->mainToolBar->addAction(stateAct);
   ui->mainToolBar->addAction(settingsAct);
	ui->mainToolBar->addAction(newTabAct);

#ifdef Q_OS_WIN32
	ui->symLinkCheckBox->setVisible(false);
   ui->symLinkCheckBox->setChecked(true);
   ui->shortcutCheckBox->setVisible(true);
   ui->shortcutCheckBox->setChecked(false);
#else
   ui->symLinkCheckBox->setVisible(true);
   ui->symLinkCheckBox->setChecked(false);
   ui->shortcutCheckBox->setVisible(false);
   ui->shortcutCheckBox->setChecked(false);
#endif

   //sui->cStyleCheckBox->setVisible(false);
   //ui->menuLayout->setVisible(false);
	//ui->invertCheckBox->setVisible(false);

	ui->toolBox->setCurrentIndex(0);

	initState();
	initLineEditCompleter();

	// set up datetime edits
	ui->dateTimeEdit->setDisplayFormat("MM.dd.yy hh:mm:ss");
	ui->dateTimeEdit2->setDisplayFormat("MM.dd.yy hh:mm:ss");
	ui->dateTimeEdit->setCalendarPopup(true);
	ui->dateTimeEdit2->setCalendarPopup(true);

	// initialize menu bar status messages
	/*ui->menuFile->setStatusTip("Ready.");
	ui->menuBar->setStatusTip("Ready.");
	connect(ui->menuFile, SIGNAL(hovered(QAction*)), this, SLOT(statusShow(QAction*)));
	connect(ui->menuBar, SIGNAL(hovered(QAction*)), this, SLOT(statusShow(QAction*)));*/

	ui->menuBar->blockSignals(true);
	ui->menuFile->blockSignals(true);
	ui->menuBar->disconnect();
	ui->menuFile->disconnect();
	//connect(ui->menuFile, SIGNAL(triggered(QAction*)), this, SLOT(statusShow(QAction*)));

   rightLabel = new QLabel;
   statusBar()->addPermanentWidget(rightLabel);
   rightLabel->setText(tr("Ready."));

   middleLabel = new QLabel;
   statusBar()->addPermanentWidget(middleLabel);
   middleLabel->setText(tr("Ready."));

	ui->cancelPushButton->setEnabled(true);
	ui->quickCancelPushButton->setEnabled(true);
	ui->quickSearchPushButton->setFocus();
	ui->quickSearchPushButton->setDefault(true);
	ui->sizeLineEdit->setEnabled(false);
	ui->sizeLineEdit2->setEnabled(false);
	ui->sizeUnitsComboBox->setEnabled(false);
	ui->sizeUnitsComboBox2->setEnabled(false);

   ///< \todo-if 10 don't work, then try 8
   sizeValidator = new QRegExpValidator(QRegExp("\\d{1,9}"), this);
	ui->sizeLineEdit->setValidator(sizeValidator);
   ui->sizeLineEdit->setMaxLength(8);
	ui->sizeLineEdit2->setValidator(sizeValidator);
   ui->sizeLineEdit2->setMaxLength(8);

   connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
   connect(ui->actionLicense, SIGNAL(triggered()), this, SLOT(showLicense()));

	// '[*]' - file modified indicator
   setWindowTitle(tr("[*]Searchmonkey - Power Searching Without the Pain"));
   setWindowIcon(QIcon(SM_ICON));
   Singleton<SettingsGlobal>::instance()->setWindowIcon(QIcon(SM_ICON));
	setWindowFlags(Qt::WindowMinMaxButtonsHint |
						Qt::Window |
						Qt::CustomizeWindowHint |
						Qt::WindowCloseButtonHint);
	setUnifiedTitleAndToolBarOnMac(true);

   readSettings();

   ///< \todo temp test settings
   ui->quickDirectoryLineEdit->setText("../../test");
	ui->quickFileLineEdit->setText("*");
   ui->quickContentLineEdit->setText("ack");
   Singleton<SettingsGlobal>::instance()->getUi()->ignoreCheckBox->setChecked(true);

   /*
   splitter = new QSplitter(this);
   splitter->setOpaqueResize(false);
   splitter->setChildrenCollapsible(false);
   splitter->addWidget(ui->dockWidget);
   splitter->addWidget(ui->resultsTabWidget);
   */

   qd(0) << "exiting constructor**********************************"
					"*****************************************************";
	m_ctorDone = true;
}
/**************************************************************************//**
 * ~MainWindow(): Destroy main window.
 ******************************************************************************/
MainWindow::~MainWindow  () {
   ENTEX2("MainWindow::~MainWindow", 0);
	delete tm;
	delete ui;
   delete license;
   delete about;
}
/*void MainWindow::newFile() {
	if (maybeSave()) {
		ui->lineDisplayH->clear();
		ui->lineDisplayV->clear();
		setCurrentFile("");
	}
}
void MainWindow::open() { QString fileName = QFileDialog::getOpenFileName(this); }
bool MainWindow::save() {
	if (curFile.isEmpty())
		return true; // saveAs();
	else
		return true; // saveFile(curFile);
}
bool MainWindow::saveAs() {
	  QString fileName = QFileDialog::getSaveFileName(this);
	  if (fileName.isEmpty())
			return false;
	  return saveFile(fileName);
 }*/
/**************************************************************************//**
 * BoxError(): Pass along a popup error message (GUI event).  [slot]
 ******************************************************************************/
 void MainWindow::boxError(const QString & s, const QMessageBox::Icon icon, Error err) {
    (void) err;
    ::boxError(s, icon);
 }
/**************************************************************************//**
 * BrowseQuick(): Quick browse (button) for target directory.
 ******************************************************************************/
void MainWindow::browseQuick  (QLineEdit *_lineEdit) {
   ENTEX("MainWindow::browseQuick");
   qd(3) << "doc loc-" << QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);

   QSettings settings;
   QString mruQuickDir = settings.value("mruQuickDir", "").toString();

   QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
      !mruQuickDir.isEmpty() ? mruQuickDir
      : QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));

   if (!directory.isEmpty()) {
      _lineEdit->setText(directory);
      settings.setValue("mruQuickDir", directory);
   }
}
/**************************************************************************//**
 * CaseSensitivity(): Case sensitivity in terms of Qt flags.
 ******************************************************************************/
Qt::CaseSensitivity MainWindow::caseSensitivity  () {
	if (ui->actionAdvanced->isChecked())
		return ui->caseCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
	else
		return Qt::CaseInsensitive;
}
/**************************************************************************//**
 * ChangeEvent(): Process change events.
 ******************************************************************************/
void MainWindow::changeEvent  (QEvent *e) {
   QWidget::changeEvent(e);
   switch (e->type()) {
   case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
   default:
      break;
   }
}
/**************************************************************************//**
 * Clear(): Clear GUI when Clear button clicked.  [slot]
 ******************************************************************************/
void MainWindow::clear  () {
   ui->quickDirectoryLineEdit->clear();
   ui->quickFileLineEdit->clear();
   ui->quickContentLineEdit->clear();

   ui->directoryLineEdit->clear();
   ui->fileLineEdit->clear();
   ui->contentLineEdit->clear();

   update();
}
/**************************************************************************//**
 * CloseEvent(): Last chance to get in pre-shutdown tasks.
 ******************************************************************************/
void MainWindow::closeEvent  (QCloseEvent *event) {
   if (!Singleton<SettingsGlobal>::instance()->confirm(CB_CLOSING)) {
      event->ignore();
      return;
   }
   writeSettings();
   event->accept();
}
/**************************************************************************//**
 * DocumentWasModified(): Indicate text display document modified with [*].
 ******************************************************************************/
void MainWindow::documentWasModified  () {
   if (tw->count() < 3)
      return;
   Ui::Tab *tui = dynamic_cast<Tab *>(tw->currentWidget())->getUi();
	if (!tui)
		return;
	setWindowModified(tui->lineDisplayH->document()->isModified() ||
							tui->lineDisplayV->document()->isModified());
}
/**************************************************************************//**
 * EnableSearchButtons(): Enable search buttons.  [slot]
 ******************************************************************************/
void MainWindow::enableSearchButtons  (bool b) {
	ui->searchPushButton->setEnabled(b);
	ui->quickSearchPushButton->setEnabled(b);
}
/**************************************************************************//**
 * GetSyntax(): Pull syntax off the GUI.
 ******************************************************************************/
QRegExp::PatternSyntax MainWindow::getSyntax() const {
   return ::int2syntax(Singleton<SettingsGlobal>::instance()->getUi()->syntaxComboBox->currentIndex());
}
/**************************************************************************//**
 * InitLineEditCompleter(): Setup filesystem completer with all line edits.
 ******************************************************************************/
void MainWindow::initLineEditCompleter  () {
   ENTEX("MainWindow::initLineEditCompleter");
   completer = new QCompleter(this);
	completer->setCaseSensitivity(Qt::CaseSensitive);
	completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);

	dirModel = new QDirModel(completer);
	dirModel->setNameFilters(QStringList());
	dirModel->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot);
	dirModel->setSorting(QDir::Name|QDir::LocaleAware);
	dirModel->setResolveSymlinks(false);

	QModelIndex mi = dirModel->index(0,0);
	qd(4) << "QDirModel: rows-" << dirModel->rowCount() << " cols-"
			<< dirModel->columnCount() << " (0,0) = " << mi.data();

	completer->setModel(dirModel);

	ui->quickDirectoryLineEdit->setCompleter(completer);
	ui->directoryLineEdit->setCompleter(completer);
}
/**************************************************************************//**
 * InitState(): Assist constructor and Reset button with original config.
 ******************************************************************************/
void MainWindow::initState  () {
	ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
	ui->dateTimeEdit2->setDateTime(QDateTime::currentDateTime());

	ui->logHistoryDockWidget->setVisible(false);
	on_actionLogHistory_toggled(false);             // configure taskbars
	ui->resultsBrowserDockWidget->setVisible(false);
	on_actionResultsBrowser_toggled(false);         // ...

	ui->actionVertical->setChecked(true);
	ui->actionQuick->setChecked(true);
	ui->actionAdvanced->setChecked(false);

	on_actionQuick_toggled(true);  // copies adv comboboxes to quick

	//ui->replaceComboBox->setCurrentIndex(0);
	//ui->replaceLineEdit->clear();

	resetAdvanced();

	ui->dateTimeComboBox->setCurrentIndex(0);
	ui->dateTimeEdit->setEnabled(false);
	ui->dateTimeEdit2->setEnabled(false);
	ui->sizeComboBox->setCurrentIndex(0);
	ui->sizeLineEdit->clear();
	ui->sizeLineEdit->setEnabled(false);
	ui->sizeUnitsComboBox->setCurrentIndex(2);
	ui->sizeUnitsComboBox->setEnabled(false);
	ui->sizeLineEdit2->clear();
	ui->sizeLineEdit2->setEnabled(false);
	ui->sizeUnitsComboBox2->setCurrentIndex(2);
	ui->sizeUnitsComboBox2->setEnabled(false);

	ui->searchPushButton->setEnabled(true);
	ui->quickSearchPushButton->setEnabled(true);

	ui->cancelPushButton->setEnabled(false);
	ui->quickCancelPushButton->setEnabled(false);
	ui->clearPushButton->setEnabled(true);
	ui->quickClearPushButton->setEnabled(true);

	ui->quickSearchPushButton->setFocus();
	ui->quickSearchPushButton->setDefault(true);
	ui->quickClearPushButton->setAutoDefault(true);

}  // initState()
/**************************************************************************//**
 * KeyPressEvent(): Reimplemented keypress event handler.
 ******************************************************************************/
void MainWindow::keyPressEvent  (QKeyEvent *event) {
   ENTEX("MainWindow::keyPressEvent");
   qd(2) << QString("got a key: 0x%1").arg(event->key(), 0, 16);

   static int modifier = 0;  // holds a modifier key

   Tab *widget = tw->count() < 3 ? nullptr : dynamic_cast<Tab *>(tw->currentWidget());

   switch (event->key()) {

   case Qt::Key_Space:  /// tree view must be grabbing this for some reason
   case Qt::Key_Enter:  /// \todo: should fire a search (a default search at startup)
   case Qt::Key_Return: /// \todo ...
      if (widget != nullptr)
         if (widget->getUi()->proxyViewH->hasFocus() || widget->getUi()->proxyViewV->hasFocus())
            switch (widget->layout()) {
            case HORIZONTAL:
               widget->grepInit(widget->getUi()->proxyViewH->currentIndex());
               return;
            case VERTICAL:
               widget->grepInit(widget->getUi()->proxyViewV->currentIndex());
               return;
            }

		if (event->key() == Qt::Key_Space)
			return;

		if (ui->actionQuick->isChecked()) {
			if (ui->quickDirectoryLineEdit->hasFocus() ||
				 ui->quickFileLineEdit->hasFocus() ||
				 ui->quickContentLineEdit->hasFocus()) {
				on_quickSearchPushButton_clicked();
				return;
			}
		}
		else {
			if (ui->directoryLineEdit->hasFocus() ||
				 ui->fileLineEdit->hasFocus() ||
				 ui->contentLineEdit->hasFocus() ||
				 ui->sizeLineEdit->hasFocus() ||
				 ui->sizeLineEdit2->hasFocus() ||
				 ui->dateTimeEdit->hasFocus() ||
				 ui->dateTimeEdit2->hasFocus()) {
				on_quickSearchPushButton_clicked();
				return;
			}
		}

		break;

	case Qt::Key_Escape:
         tm->cancel();
         emit stopPreSearchDirCnt();
		break;

   case Qt::Key_Control:
      modifier = Qt::Key_Control;
      break;

   case Qt::Key_S:
      if (modifier == Qt::Key_Control) {
         statusShow("Skipping folder count...");
         emit stopPreSearchDirCnt();
         modifier = 0;
      }
      break;

   case Qt::Key_Down:
	case Qt::Key_Up:
		; // These keys are consumed by the tree view, that is, when the tree
		  // view has focus and the up or down arrow is used, this handler is
		  // not called and these cases are not considered.  What to do?
		break;

	default:
		break;

	}  // switch()

	QMainWindow::keyPressEvent(event);

}  // keyPressEvent()
/**************************************************************************//**
 * ReadSettings(): Read some settings.
 ******************************************************************************/
void MainWindow::readSettings  () {
   ENTEX("readSettings");

   QSettings settings;

   settings.beginGroup("MainWindow");
   resize(settings.value("size", QSize(800, 400)).toSize());
   move(settings.value("pos", QPoint(200, 200)).toPoint());
   settings.endGroup();

   qd(3) << "status: " << settings.status();

   Ui::SettingsGlobal * sgui = Singleton<SettingsGlobal>::instance()->getUi();

   settings.beginGroup("Global");
   sgui->beforeCheckBox->setChecked(settings.value("before", true).toBool());
   sgui->overwriteCheckBox->setChecked(settings.value("overwrite", false).toBool());
   sgui->cancelCheckBox->setChecked(settings.value("cancel", true).toBool());
   sgui->closingCheckBox->setChecked(settings.value("closing", false).toBool());
   sgui->lastCheckBox->setChecked(settings.value("last", false).toBool());
   sgui->restoreCheckBox->setChecked(settings.value("restore", true).toBool());
   sgui->newCheckBox->setChecked(settings.value("new", false).toBool());
   sgui->busyCheckBox->setChecked(settings.value("busy", true).toBool());
   sgui->accurateCheckBox->setChecked(settings.value("accurate", false).toBool());
   sgui->ignoreCheckBox->setChecked(settings.value("ignore", false).toBool());
   sgui->linenoCheckBox->setChecked(settings.value("lineno", true).toBool());
   sgui->whitespaceCheckBox->setChecked(settings.value("whitespace", true).toBool());

   sgui->editorLineEdit->setText(settings.value("editor", "notepad.exe").toString());
   sgui->matchesSpinBox->setValue(settings.value("matches", 0).toInt());
   sgui->syntaxComboBox->setCurrentIndex(settings.value("syntax", 0).toInt());
   settings.endGroup();
 }
/**************************************************************************//**
 * Refresh(): Refresh main window.  Color text for invalid entries.  [slot]
 ******************************************************************************/
void MainWindow::refresh  () {
	qApp->processEvents();

	QPalette palette;
	if (ui->actionAdvanced->isChecked()) {

		palette = ui->directoryLineEdit->palette();
		palette.setColor(QPalette::Text,
                       validDir(ui->directoryLineEdit->text())?
																	 COLOR_TEXT : COLOR_HIGHLIGHT);
		ui->directoryLineEdit->setPalette(palette);

		palette = ui->fileLineEdit->palette();
		palette.setColor(QPalette::Text,
							  validRx(ui->fileLineEdit->text(), QRegExp::Wildcard)?
																	 COLOR_TEXT : COLOR_HIGHLIGHT);
		ui->fileLineEdit->setPalette(palette);

		palette = ui->contentLineEdit->palette();
		palette.setColor(QPalette::Text,
                       validRx(ui->contentLineEdit->text(), getSyntax())?
																	 COLOR_TEXT : COLOR_HIGHLIGHT);
		ui->contentLineEdit->setPalette(palette);
	}
	else {
		palette = ui->quickDirectoryLineEdit->palette();
		palette.setColor(QPalette::Text,
                       validDir(ui->quickDirectoryLineEdit->text())?
																	 COLOR_TEXT : COLOR_HIGHLIGHT);
		ui->quickDirectoryLineEdit->setPalette(palette);

		palette = ui->quickFileLineEdit->palette();
		palette.setColor(QPalette::Text,
							  validRx(ui->quickFileLineEdit->text(), QRegExp::Wildcard)?
																	 COLOR_TEXT : COLOR_HIGHLIGHT);
		ui->quickFileLineEdit->setPalette(palette);

		palette = ui->quickContentLineEdit->palette();
		palette.setColor(QPalette::Text,
                       validRx(ui->quickContentLineEdit->text(), getSyntax())?
																	 COLOR_TEXT : COLOR_HIGHLIGHT);
		ui->quickContentLineEdit->setPalette(palette);
	}

	if (ui->actionAdvanced->isChecked()) {
		int pos = 0;
		QString s = ui->sizeLineEdit->text();
		palette = ui->sizeLineEdit->palette();
		palette.setColor(QPalette::Text, sizeValidator->validate(s, pos) ==
							  QValidator::Acceptable ? COLOR_TEXT : Qt::green);
		ui->sizeLineEdit->setPalette(palette);

		s = ui->sizeLineEdit2->text();
		palette = ui->sizeLineEdit2->palette();
		palette.setColor(QPalette::Text, sizeValidator->validate(s, pos) ==
							  QValidator::Acceptable ? COLOR_TEXT : Qt::green);
		ui->sizeLineEdit2->setPalette(palette);
	}

}  // refresh()
/**************************************************************************//**
 * ResetAdvanced(): Reset advanced checkboxes.  [slot]
 ******************************************************************************/
void MainWindow::resetAdvanced  () {
	ui->subdirCheckBox->setChecked(true);
	//ui->invertCheckBox->setChecked(false);
	ui->caseCheckBox->setChecked(false);
	ui->symLinkCheckBox->setChecked(false);
	ui->skipHiddenCheckBox->setChecked(false);
}
/**************************************************************************//**
 * SetupTabWidget(): setup '+' tab and tab bar context menu stuff.
 ******************************************************************************/
void MainWindow::setupTabWidget  () {
   ENTEX("MainWindow::setupTabWidget");

   // Add '+' button.
   addTabButton = new QPushButton("+");
   addButton = new QPushButton("+");
   addTabButton->resize(20,20);
   addButton->resize(20,20);
   connect(addTabButton, SIGNAL(clicked()), tm, SLOT(tabCreate()));
   tb->setTabButton(0, QTabBar::LeftSide, addTabButton);

   /// \todo implement
   // tw->setCornerWidget(addButton, Qt::TopLeftCorner);
   // Configure tab widget.
   tw->setCurrentIndex(-1);  // eccs
   tw->setMovable(true);
   tw->setTabsClosable(false);
   tw->setUsesScrollButtons(true);

   //
   // Setup tab bar context menu.
   //
   tb->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(tb, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequested(QPoint)));

   closeAct = new QAction(tr("&Close"), tw);
   closeAct->setShortcuts(QKeySequence::Close);
   // closeAct->setStatusTip(tr("Close tab"));  ///< \todo-redirect status event to middle status
   connect(closeAct, SIGNAL(triggered()), this, SLOT(on_action_Close_triggered()));

   tabContextMenu = new QMenu(tb);
   tabContextMenu->addAction(closeAct);
}
void MainWindow::on_action_Close_triggered() {
   ENTEX("MainWindow::on_action_Close_triggered");
   tm->tabClose(dynamic_cast<Tab *>(tw->widget(tb->tabAt(p))));
   p.setX(0), p.setY(0);
}
void MainWindow::on_customContextMenuRequested(QPoint pos) {
   p += pos;
   tabContextMenu->exec(tb->mapToGlobal(pos));
}
/**************************************************************************//**
 * ShowAbout(): Show About menu option. [slot]
 ******************************************************************************/
void MainWindow::showAbout  () {
   about->show();
}
/**************************************************************************//**
 * ShowLicense(): Show License menu option. [slot]
 ******************************************************************************/
void MainWindow::showLicense  () {
   license->show();
}
/**************************************************************************//**
 * StateDateTime(): Set GUI state based on dateTimeComboBox's state.  [slot]
 ******************************************************************************/
void MainWindow::stateDateTime  (QString _option) {
	if (_option == "Ignored") {
		ui->dateTimeEdit->setEnabled(false);
		ui->dateTimeEdit2->setEnabled(false);
	}
	else if (_option.indexOf("Between") != -1) {
		ui->dateTimeEdit->setEnabled(true);
		ui->dateTimeEdit2->setEnabled(true);
	}
	else {  // before/after
		ui->dateTimeEdit->setEnabled(true);
		ui->dateTimeEdit2->setEnabled(false);
	}
}
/**************************************************************************//**
 * StateSize(): Set GUI state based on sizeComboBox's state.  [slot]
 ******************************************************************************/
void MainWindow::stateSize  (QString _option) {
   if (_option.indexOf("Any") != -1) {
		ui->sizeLineEdit->setEnabled(false);
		ui->sizeLineEdit2->setEnabled(false);
		ui->sizeUnitsComboBox->setEnabled(false);
		ui->sizeUnitsComboBox2->setEnabled(false);
	}
	else if (_option.indexOf("Between") != -1) {
		ui->sizeLineEdit->setEnabled(true);
		ui->sizeLineEdit2->setEnabled(true);
		ui->sizeUnitsComboBox->setEnabled(true);
		ui->sizeUnitsComboBox2->setEnabled(true);
	}
	else {  // larger/smaller
		ui->sizeLineEdit->setEnabled(true);
		ui->sizeLineEdit2->setEnabled(false);
		ui->sizeUnitsComboBox->setEnabled(true);
		ui->sizeUnitsComboBox2->setEnabled(false);
	}
}
/**************************************************************************//**
 * StatusMiddle(): Show given status on middle status widget.  [slot]
 ******************************************************************************/
void MainWindow::statusMiddle  (const QString &msg) {
   ENTEX("MainWindow::statusdMiddle");
   qd(0) << "setting middle label text-" << msg;
   middleLabel->setText(msg);
}
/**************************************************************************//**
 * StatusRight(): Show given status on right permanent status widget.  [slot]
 ******************************************************************************/
void MainWindow::statusRight  (const QString &msg) {
   rightLabel->setText(msg);
}
/**************************************************************************//**
 * statusShow(): Show init status message in the main status bar.
 ******************************************************************************/
void MainWindow::statusShow  (QAction *) {
   statusShow(tr("Ready."));
}
/**************************************************************************//**
 * StatusShow(): Show given status on status bar.  [slot]
 ******************************************************************************/
 void MainWindow::statusShow  (const QString &msg) {
   statusBar()->showMessage(msg);
 }
/**************************************************************************//**
 * WriteSettings(): Write some settings.
 ******************************************************************************/
void MainWindow::writeSettings  () {
   ENTEX("writeSettings");

   QSettings settings;

   settings.beginGroup("MainWindow");
   settings.setValue("size", size());
   settings.setValue("pos", pos());
   settings.endGroup();

   Ui::SettingsGlobal * sgui = Singleton<SettingsGlobal>::instance()->getUi();

   settings.beginGroup("Global");
   settings.setValue("before", sgui->beforeCheckBox->isChecked());
   settings.setValue("overwrite", sgui->overwriteCheckBox->isChecked());
   settings.setValue("cancel", sgui->cancelCheckBox->isChecked());
   settings.setValue("closing", sgui->closingCheckBox->isChecked());
   settings.setValue("last", sgui->lastCheckBox->isChecked());
   settings.setValue("restore", sgui->restoreCheckBox->isChecked());
   settings.setValue("new", sgui->newCheckBox->isChecked());
   settings.setValue("busy", sgui->busyCheckBox->isChecked());
   settings.setValue("accurate", sgui->accurateCheckBox->isChecked());
   settings.setValue("ignore", sgui->ignoreCheckBox->isChecked());
   settings.setValue("lineno", sgui->linenoCheckBox->isChecked());
   settings.setValue("whitespace", sgui->whitespaceCheckBox->isChecked());

   settings.setValue("editor", sgui->editorLineEdit->text());
   settings.setValue("matches", sgui->matchesSpinBox->value());
   settings.setValue("syntax", sgui->syntaxComboBox->currentIndex());
   settings.endGroup();

   qd(0) << "status: " << settings.status();
}
/**************************************************************************//**
 * Function group [slots]: headers generated by QtDesigner.
 ******************************************************************************/
void MainWindow::on_actionAbout_Qt_triggered() {
   QMessageBox::aboutQt(this, "Qt version");
}
void MainWindow::on_actionAdvanced_toggled(bool _toggle) {
   ENTEX("MainWindow::on_actionAdvanced_toggled");
   if (_toggle) {
		qd(3) << "got sig on action advanced toggled(" << _toggle << ")";
		ui->searchPushButton->setFocus();
		ui->searchPushButton->setDefault(true);
		ui->clearPushButton->setAutoDefault(true);

		ui->quickSearchPushButton->setDefault(false);
		ui->quickClearPushButton->setAutoDefault(false);

		ui->stackedWidget->setCurrentIndex(1);
		ui->actionQuick->setChecked(false);
      stateAct->setText("&" + tr("Quick"));
		if (m_ctorDone) {  // ignore sigs emitted during construction
         ui->directoryLineEdit->setText(ui->quickDirectoryLineEdit->text());
			ui->fileLineEdit->setText(ui->quickFileLineEdit->text());
			ui->contentLineEdit->setText(ui->quickContentLineEdit->text());
		}
	}
}
void MainWindow::on_actionHorizontal_toggled(bool _toggle) {
	if (_toggle) {
      if (tw->count() < 3)
         return;
      Ui::Tab *tui = dynamic_cast<Tab *>(tw->currentWidget())->getUi();
		ui->actionVertical->setChecked(false);  // emits to vert toggle slot
		tui->resultsStackedWidget->setCurrentIndex(0);
		if (!tui->lineDisplayV->toPlainText().isEmpty()) {
			Highlighter *highlighter=new Highlighter(tui->lineDisplayH->document(),
                      ui->contentLineEdit->text(), getSyntax(), caseSensitivity());
			highlighter->init();
			tui->lineDisplayH->setHtml(tui->lineDisplayV->toHtml());
		}
      layoutAct->setText("&" + tr("Vertical"));
		dynamic_cast<Tab *>(tw->currentWidget())->layoutPushButtonSetLabel();
	}
}
void MainWindow::on_actionLogHistory_toggled(bool toggle) {
	if (ui->logHistoryDockWidget->isVisible() != toggle) {
		ui->logHistoryDockWidget->setVisible(toggle);
		logHistoryAct->setVisible(!toggle);
	}
}
void MainWindow::on_actionQuick_toggled(bool _toggle) {
	if (_toggle) {
		ui->quickSearchPushButton->setFocus();
		ui->quickSearchPushButton->setDefault(true);
		ui->quickClearPushButton->setAutoDefault(true);

		ui->searchPushButton->setDefault(false);
		ui->clearPushButton->setAutoDefault(false);

		ui->stackedWidget->setCurrentIndex(0);
		ui->actionAdvanced->setChecked(false);
      stateAct->setText("&" + tr("Advanced"));
		if (m_ctorDone) {  // ignore sigs emitted during construction
         ui->quickDirectoryLineEdit->setText(ui->directoryLineEdit->text());
			ui->quickFileLineEdit->setText(ui->fileLineEdit->text());
			ui->quickContentLineEdit->setText(ui->contentLineEdit->text());
		}
	}
}
void MainWindow::on_actionRestore_triggered() {
   if (boxConfirm(tr("Really restore configuration and settings to factory defaults?"))) {
      initState();
      Singleton<SettingsGlobal>::instance()->on_restorePushButton_clicked();
   }
}
void MainWindow::on_actionResultsBrowser_toggled(bool toggle) {
	if (ui->resultsBrowserDockWidget->isVisible() != toggle) {
		ui->resultsBrowserDockWidget->setVisible(toggle);
		resultsBrowserAct->setVisible(!toggle);
	}
}
void MainWindow::on_actionSearch_Dialog_toggled(bool toggle) {
	ui->dockWidget->setVisible(toggle);
}
void MainWindow::on_actionSettings_triggered() {
   Singleton<SettingsGlobal>::instance()->show();
}
void MainWindow::on_actionVertical_toggled(bool _toggle) {
   ENTEX("MainWindow::on_actionVertical_toggled");
   if (_toggle) {
      if (tw->count() < 3)
         return;
      Ui::Tab *tui = dynamic_cast<Tab *>(tw->currentWidget())->getUi();
		ui->actionHorizontal->setChecked(false);  // emits to horiz toggle slot
		qd(3) << "setting stack widget to 1...";
		tui->resultsStackedWidget->setCurrentIndex(1);
		if (!tui->lineDisplayH->toPlainText().isEmpty()) {
			Highlighter *highlighter=new Highlighter(tui->lineDisplayV->document(),
                      ui->contentLineEdit->text(), getSyntax(), caseSensitivity());
			highlighter->init();
			tui->lineDisplayV->setHtml(tui->lineDisplayH->toHtml());
		}
      layoutAct->setText("&" + tr("Horizontal"));
		if (tw->count() > 2)
			dynamic_cast<Tab *>(tw->currentWidget())->layoutPushButtonSetLabel();
	}
}
void MainWindow::on_browseToolButton_clicked() {
   QString dir = ::browse(this);
   if (!dir.isEmpty())
      ui->directoryLineEdit->setText(dir);
}
void MainWindow::on_cancelPushButton_clicked() {
   Tab *tab = tm->currentTab();
   if (tab == nullptr)
      return;
   if (tab->getFindState() == false)
      return;
	tm->cancel();
}
void MainWindow::on_clearPushButton_clicked() {
	clear();
}
void MainWindow::on_contentLineEdit_textChanged(QString) {
	refresh();
}
void MainWindow::on_dateTimeComboBox_currentIndexChanged(QString option) {
	stateDateTime(option);
}
void MainWindow::on_directoryLineEdit_textChanged(QString s) {
   ENTEX("MainWindow::on_directoryLineEdit_textChanged");

   int sz = s.size();
	if (sz > 0  &&  s[sz - 1] == QChar('\\')) {
		qd(3) << "sorting QDirModel for Completer...";
		dirModel->sort(0);  // allows completer to do binary search
	}
	refresh();
}
void MainWindow::on_dockWidget_visibilityChanged(bool visible) {
	qApp->processEvents();
	if (this->isMinimized())  // ignore sig if user minimizes main win; otherwise, dock widget stays hidden
		return;
	ui->actionSearchDialog->setChecked(visible);
}
void MainWindow::on_fileLineEdit_textChanged(QString) {
	refresh();
}
void MainWindow::on_lineDisplayH_textChanged() {
	documentWasModified();
}
void MainWindow::on_lineDisplayV_textChanged() {
	documentWasModified();
}
void MainWindow::on_logHistoryDockWidget_visibilityChanged(bool visible) {
	if (ui->actionLogHistory->isChecked() != visible) {
		ui->actionLogHistory->setChecked(visible);
		logHistoryAct->setVisible(!visible);
	}
}
void MainWindow::on_quickBrowseToolButton_clicked() {
	browseQuick(ui->quickDirectoryLineEdit);
}
void MainWindow::on_quickCancelPushButton_clicked() {
	on_cancelPushButton_clicked();  // both do same thing
}
void MainWindow::on_quickClearPushButton_clicked() {
	clear();
}
void MainWindow::on_quickContentLineEdit_textChanged(QString) {
	refresh();
}
void MainWindow::on_quickDirectoryLineEdit_textChanged(QString s) {
   ENTEX("MainWindow::on_quickDirectoryLineEdit_textChanged");
   int sz = s.size();
	if (sz > 0  &&  s[sz - 1] == QChar('\\')) {
		qd(3) << "sorting QDirModel for Completer...";
		dirModel->sort(0);  // allows completer to do binary search
	}
	refresh();
}
void MainWindow::on_quickFileLineEdit_textChanged(QString) {
	refresh();
}
void MainWindow::on_quickSearchPushButton_clicked() {
   ENTEX("MainWindow::on_quickSearchPushButon_clicked");
   qd(0) << "initiating a search *********************************"
            "******************************************************";
   emit tabFind();
}
void MainWindow::on_resetAdvButton_clicked() {
	resetAdvanced();
}
void MainWindow::on_resultsBrowserDockWidget_visibilityChanged(bool visible) {
	if (ui->actionResultsBrowser->isChecked() != visible) {
		ui->actionResultsBrowser->setChecked(visible);
		resultsBrowserAct->setVisible(!visible);
	}
}
void MainWindow::on_resultsTabWidget_currentChanged(int _index) {
   tm->tabChanged(_index);
}
void MainWindow::on_searchPushButton_clicked() {
	on_quickSearchPushButton_clicked();
}
void MainWindow::on_sizeComboBox_currentIndexChanged(QString option) {
	stateSize(option);
}
void MainWindow::on_sizeLineEdit_textEdited(QString) {
	refresh();
}
void MainWindow::on_sizeLineEdit2_textEdited(QString) {
	refresh();
}
void MainWindow::on_tbarActionLayout_triggered() {
	if (layoutAct->text() == "&Horizontal")
		ui->actionHorizontal->setChecked(true);
	else
		ui->actionVertical->setChecked(true);
}
void MainWindow::on_tbarActionLogHistory_triggered() {
	on_actionLogHistory_toggled(true);
	logHistoryAct->setVisible(false);
}
void MainWindow::on_tbarActionResultsBrowser_triggered() {
	on_actionResultsBrowser_toggled(true);
	resultsBrowserAct->setVisible(false);
}
void MainWindow::on_tbarActionSettings_triggered() {
   on_actionSettings_triggered();
}
void MainWindow::on_tbarActionState_triggered() {
	if (stateAct->text() == "&Quick")
		ui->actionQuick->setChecked(true);     // emits sig toggled()
	else
		ui->actionAdvanced->setChecked(true);  // emits sig toggled()
}
void MainWindow::on_dockWidget_topLevelChanged(bool top) {

   if (!top)  // docked
      return;

   static bool undockedOnce = false;

   if (!undockedOnce) {
      ui->dockWidget->setWindowTitle("Search  (Double-click dock or drag dock right)");
      undockedOnce = true;
   }
   else ui->dockWidget->setWindowTitle("Search");
}
