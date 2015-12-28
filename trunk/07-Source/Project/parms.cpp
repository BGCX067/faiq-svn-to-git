/**************************************************************************//**
 * NAME:    parms.cpp
 *
 * PURPOSE: Implements parameter storage and context switching facilities.
 *
 * DATE:    February 12, 2010
*******************************************************************************/
#include "debug.h"

#include <QComboBox>
#include <QTabWidget>

#include "mainwindow.h"
#include "parms.h"
#include "settingsglobal.h"
#include "tab.h"
#include "util.h"

#include "ui_mainwindow.h"
#include "ui_settingsglobal.h"
/**************************************************************************//**
 * Parms(): create Parms object.
 ******************************************************************************/
Parms::Parms  (Tab *_tab, MainWindow *_mw)
:
followShortcuts(false),
followSymlinks(false),
replaceIdx(-1),
syntaxIdx(0),
findMatches(0UL),
m_checkBoxes(0ULL),
mw(_mw),
tab(_tab),
ui(mw->getUi()),
kase(Qt::CaseInsensitive),
m_createStamp(QDateTime::currentDateTime()),
m_headingFont(nullptr),
m_matchFont(nullptr),
content(""),
replace("")
{
   ripContext(SM_INITPARMS);
   ripGlobalSettings();
}
/**************************************************************************//**
 * BurnContext(): burn GUI context.
 ******************************************************************************/
void Parms::burnContext  () {
   ENTEX("Parms::burnContext");
   qd(5) << "burn object-" << this;

   // test m_createStamp = QDateTime::fromString("Thu Feb 03 08:40:13 2011", "ddd MMM dd hh:mm:ss yyyy");
   updateTabLabel();

   if (!mainStatus.isEmpty() && !tab->busy())
      mw->statusShow(mainStatus);

   ui->actionQuick->setChecked(state == QUICK);
	ui->actionAdvanced->setChecked(state == ADVANCED);

   qd(5) << "burning path-" << path << "filenames-" << filenames << "content-" << content;
	if (state == ADVANCED) {
		ui->directoryLineEdit->setText(path);
		ui->fileLineEdit->setText(filenames);
		ui->contentLineEdit->setText(content);
	}
	else {
		ui->quickDirectoryLineEdit->setText(path);
		ui->quickFileLineEdit->setText(filenames);
		ui->quickContentLineEdit->setText(content);
	}

	ui->actionLogHistory->setChecked(visLogHistory);
	ui->actionResultsBrowser->setChecked(visResultsBrowser);
	ui->actionSearchDialog->setChecked(visSearchDialog);

	qd(5) << "burning layout(" << layout << ")" << ((layout == HORIZONTAL) ? "horizontal" : "vertical");
	if (layout == HORIZONTAL)
		ui->actionHorizontal->setChecked(true);
	else
		ui->actionVertical->setChecked(true);

	//ui->replaceLineEdit->setText(replace);
	//ui->replaceComboBox->setCurrentIndex(replaceIdx);
	ui->sizeComboBox->setCurrentIndex(sizeIdx);

	ui->caseCheckBox->setChecked(kase == Qt::CaseSensitive);

	ui->subdirCheckBox->setChecked(searchSubs);
   ui->shortcutCheckBox->setChecked(followShortcuts);
	ui->symLinkCheckBox->setChecked(followSymlinks);
	ui->skipHiddenCheckBox->setChecked(skipHidden);

	if (0 && sizeEnabled) {
		ui->sizeLineEdit->setEnabled(true);
	}
	ui->sizeLineEdit->setText(size);
	if (0 && size2Enabled) {
		ui->sizeLineEdit2->setEnabled(true);
	}
	ui->sizeLineEdit2->setText(size2);
	if (0 && sizeUnitsEnabled) {
		ui->sizeUnitsComboBox->setEnabled(true);
	}
	ui->sizeUnitsComboBox->setCurrentIndex(sizeUnitsIdx);
	if (0 && sizeUnits2Enabled) {
		ui->sizeUnitsComboBox2->setEnabled(true);
	}
	ui->sizeUnitsComboBox2->setCurrentIndex(sizeUnits2Idx);
	ui->dateTimeComboBox->setCurrentIndex(dateTimeIdx);
	if (0 && dateTimeEnabled) {
		ui->dateTimeEdit->setEnabled(true);
	}
	ui->dateTimeEdit->setDateTime(dateTime);
	if (0 && dateTime2Enabled) {
		ui->dateTimeEdit2->setEnabled(true);
	}
	ui->dateTimeEdit2->setDateTime(dateTime2);

   qd(5) << "burn complete...";

}  // burnContext()
/**************************************************************************//**
 * CaseSensitive(): case sensitivite based on GUI.
 ******************************************************************************/
bool Parms::caseSensitive  () const {
	if (state == ADVANCED)
		return kase == Qt::CaseSensitive;
	else
		return false;  // quick mode is case insensitive
}
/**************************************************************************//**
 * CaseSensitivity(): case sensitivity in terms of Qt flags.
 ******************************************************************************/
Qt::CaseSensitivity Parms::caseSensitivity  () const {
	return kase;
}
/**************************************************************************//**
 * ModeDateTime(): mode is find files with date/time constraints.
 ******************************************************************************/
bool Parms::modeDateTime  () const {
   return state == ADVANCED  &&  dateTimeTxt != QObject::tr("Any date and time");
}
/**************************************************************************//**
 * ModeInvert(): mode is find/grep for non-matching filenames/lines.
 ******************************************************************************/
bool Parms::modeInvert  () const {
   return false;  /// temp method \todo implement as local setting
}
/**************************************************************************//**
 * ModeShortcut(): mode is find/grep following shortcuts.
 ******************************************************************************/
bool Parms::modeShortcut  () const {
   return state == ADVANCED  &&  followShortcuts;
}
/**************************************************************************//**
 * ModeSize(): mode is find/grep for non-matching filenames/lines.
 ******************************************************************************/
bool Parms::modeSize  () const {
   return state == ADVANCED  &&  sizeTxt != QObject::tr("Any size");
}
/**************************************************************************//**
 * ModeSkipHidden(): mode is find/grep skipping hidden files.
 ******************************************************************************/
bool Parms::modeSkipHidden  () const {
	return state == ADVANCED  &&  skipHidden;
}
/**************************************************************************//**
 * ModeSymLink(): mode is find/grep following symbolic links.
 ******************************************************************************/
bool Parms::modeSymLink  () const {
	return state == ADVANCED  &&  followSymlinks;
}
/**************************************************************************//**
 * RipContext(): rip GUI context.
 ******************************************************************************/
void Parms::ripContext  (bool init) {
   ENTEX("Parms::ripContext");
	qd(5) << "rip object-" << this;

   if (init)
      mainStatus = "Ready.";
   else if (!tab->busy())
      mainStatus = mw->statusBar()->currentMessage();
   else
      mainStatus = "";

   state = ui->actionQuick->isChecked() ? QUICK : ADVANCED;  // rip first

	path = (state == QUICK) ? ui->quickDirectoryLineEdit->text()
									: ui->directoryLineEdit->text();
	filenames = (state == QUICK) ? ui->quickFileLineEdit->text()
										  : ui->fileLineEdit->text();
	content = (state == QUICK) ? ui->quickContentLineEdit->text()
										: ui->contentLineEdit->text();
   qd(5) << "ripped  path-" << path << "filenames-" << filenames << "content-" << content;

   setFilelist();

   visLogHistory = ui->actionLogHistory->isChecked();
	visResultsBrowser = ui->actionResultsBrowser->isChecked();
	visSearchDialog = ui->actionSearchDialog->isChecked();

	layout = ui->actionHorizontal->isChecked() ? HORIZONTAL : VERTICAL;
	qd(5) << "ripped layout(" << layout << ")" << ((layout == HORIZONTAL) ? "horizontal" : "vertical");

	//replace = ui->replaceLineEdit->text();
	//replaceIdx = ui->replaceComboBox->currentIndex();
	sizeIdx = ui->sizeComboBox->currentIndex();

	kase = (state == QUICK) ? Qt::CaseInsensitive
									: ui->caseCheckBox->isChecked() ? Qt::CaseSensitive :
																				 Qt::CaseInsensitive;
	searchSubs = ui->subdirCheckBox->isChecked();
   skipHidden = ui->skipHiddenCheckBox->isChecked();
   followShortcuts = ui->shortcutCheckBox->isChecked();
	followSymlinks = ui->symLinkCheckBox->isChecked();

	sizeTxt = ui->sizeComboBox->currentText();
	//if ((sizeEnabled = ui->sizeLineEdit->isEnabled()))
	size = ui->sizeLineEdit->text();
	//if ((size2Enabled = ui->sizeLineEdit2->isEnabled()))
	size2 = ui->sizeLineEdit2->text();
	sizeUnits = ui->sizeUnitsComboBox->currentText();
	sizeUnits2 = ui->sizeUnitsComboBox2->currentText();
	//if ((sizeUnitsEnabled = ui->sizeUnitsComboBox->isEnabled()))
	sizeUnitsIdx = ui->sizeUnitsComboBox->currentIndex();
	//if ((sizeUnits2Enabled = ui->sizeUnitsComboBox2->isEnabled()))
	sizeUnits2Idx = ui->sizeUnitsComboBox2->currentIndex();

	dateTimeIdx = ui->dateTimeComboBox->currentIndex();
	//if ((dateTimeEnabled = ui->dateTimeEdit->isEnabled()))
	dateTimeTxt = ui->dateTimeComboBox->currentText();
	dateTime = ui->dateTimeEdit->dateTime();
	//if ((dateTime2Enabled = ui->dateTimeEdit2->isEnabled()))
   dateTime2 = ui->dateTimeEdit2->dateTime();

   ripGlobalSettings();

}  // ripContext()
/**************************************************************************//**
 * RipGlobalSettings(): rip all global settings into Parms object.
 ******************************************************************************/
 void Parms::ripGlobalSettings  () {
   ENTEX("Parms::ripGlobalSettings");

   setCheckBoxes();

   SettingsGlobal *sg = Singleton<SettingsGlobal>::instance();  // shortcut
   Ui::SettingsGlobal *sgui = sg->getUi();                      // ""

   m_editor = sgui->editorLineEdit->text();
   findMatches = static_cast<quint32>(sgui->matchesSpinBox->value());

   int idx = sgui->syntaxComboBox->currentIndex();
   // We now have all the data to build the regular expression object.
   m_rx = QRegExp(content, kase, int2syntax(idx));

   if (sg->userChangedHeadingFont())
      m_headingFont = new QFont(sg->getHeadingFont());
   if (sg->userChangedMatchFont())
      m_matchFont = new QFont(sg->getMatchFont());
 }
/**************************************************************************//**
 * SetCheckBoxes(): Trip bits corresponding to global settings' checkboxes.
 ******************************************************************************/
void Parms::setCheckBoxes  () {
   Ui::SettingsGlobal *sgui = Singleton<SettingsGlobal>::instance()->getUi();

   m_checkBoxes = 0ULL;

   if (sgui->beforeCheckBox->isChecked())
      m_checkBoxes |= CB_BEFORE;
   if (sgui->overwriteCheckBox->isChecked())
      m_checkBoxes |= CB_OVERWRITE;
   if (sgui->cancelCheckBox->isChecked())
      m_checkBoxes |= CB_CANCEL;
   if (sgui->closingCheckBox->isChecked())
      m_checkBoxes |= CB_CLOSING;
   if (sgui->lastCheckBox->isChecked())
      m_checkBoxes |= CB_LAST;
   if (sgui->restoreCheckBox->isChecked())
      m_checkBoxes |= CB_RESTORE;
   if (sgui->newCheckBox->isChecked())
      m_checkBoxes |= CB_NEW;
   if (sgui->busyCheckBox->isChecked())
      m_checkBoxes |= CB_BUSY;
   if (sgui->accurateCheckBox->isChecked())
      m_checkBoxes |= CB_ACCURATE;
   if (sgui->ignoreCheckBox->isChecked())
      m_checkBoxes |= CB_IGNORE;
   if (sgui->linenoCheckBox->isChecked())
      m_checkBoxes |= CB_LINENO;
   if (sgui->whitespaceCheckBox->isChecked())
      m_checkBoxes |= CB_WHITESPACE;
}
/**************************************************************************//**
 * SetFilelist(): build handy list of filename wildcard patterns.
 ******************************************************************************/
void Parms::setFilelist  () {
   ENTEX("Parms::setFilelist");

   filelist.clear();

   if (filenames.isEmpty()) {
      filelist << "*";
      return;
   }

   int n = 0;
   char** patterns;
   qd(5) << "parsing-" << filenames;
   ::parseDelim(filenames.toStdString().c_str(), ", ;", &patterns, &n);

   qd(5) << "wildcards-" << n;
   for (int i = 0; i < n; ++i) {
      if (*patterns[i]) {
         qd(5) << "pushing-" << patterns[i];
         filelist << patterns[i];
      }
   }
}
 /**************************************************************************//**
 * Validate(): validate parms.
 *
 * Returns   : true on success, false otherwise
 ******************************************************************************/
bool Parms::validate  () {
   ENTEX2("Parms::validate", 0);

   //
   // Establish search path.
   //
   if (path.isEmpty()) {
      path = QDir::homePath();
      if (state == QUICK)
         ui->quickDirectoryLineEdit->setText(path);
      else
         ui->directoryLineEdit->setText(path);
   }

   if (!::validDir(path)) {
      ::boxError(QObject::tr("Invalid folder"));
      return false;
   }

   //
   // Establish filename search patterns.
   //
   if (filenames.isEmpty()) {
      filenames = "*";
      if (state == QUICK)
         ui->quickFileLineEdit->setText(filenames);
      else
         ui->fileLineEdit->setText(filenames);
   }
   if (!::validRx(filelist, QRegExp::Wildcard)) {
      QString s = QObject::tr("Invalid filename") + ::charS(filelist.size());
      ::boxError(s);
      return false;
   }
   qd(2) << "filelist-" << filelist;
   qd(2) << "filenames-" << filenames;

   //
   // Establish content to grep.
   //
   if (!::validRx(m_rx)) {
      ::boxError(QObject::tr("Invalid content"));
      return false;
   }

   qd(0) << ": p-" << this << " (content-" << content << ")";

   return true;

}  // validate()
 /**************************************************************************//**
 * UpdateTabLabel(): set the tab label based on the tab age.
 *
 * Notes           : n hh:mm        (less than day old)
 *                   n ddd hh:mm    (less than week old)
 *                   n d/M/yy hh:mm (week or older)
 ******************************************************************************/
void Parms::updateTabLabel  () {
   ENTEX("Parms::updateTabLebal");

   QDateTime now = QDateTime::currentDateTime();

   int days = m_createStamp.daysTo(now);

   if (days < 1)
      return;

   //
   // We end up rewriting the label a lot, but it avoids a bunch of checks.
   //
   QTabWidget *tw = mw->getUi()->resultsTabWidget;  // shortcut
   int idx = tw->indexOf(tab);
   QString label = tw->tabText(idx).section(' ', 0, 0);  // grab "&n"

   if (days >= 1  &&  days < 7)
      label += m_createStamp.toString(" ddd h:mm");
   else
      label += m_createStamp.toString(" d/M/yy h:mm");

   tw->setTabText(idx, label);
}
