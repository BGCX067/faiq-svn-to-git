/**************************************************************************//**
 * NAME:    settingsglobal.cpp
 *
 * PURPOSE: Implementation of global settings widget.
 *
 * DATE:    September 30, 2010
*******************************************************************************/
#include "debug.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QFontDialog>
#include <QRegExp>
#include <QSettings>
#include <QShowEvent>
#include <QString>
#include <QStringList>

#include "settingsglobal.h"
#include "util.h"

#include "ui_presets.h"
#include "ui_settingsglobal.h"
/**************************************************************************//**
 * SettingsGlobal(): Construct.
 ******************************************************************************/
SettingsGlobal::SettingsGlobal  (QWidget *parent)
:
QDialog(parent),
m_matches(0),
m_syntax(0),
m_checkBoxes(0ULL),
ui(new Ui::SettingsGlobal)
{
   ui->setupUi(this);

   presets = new Presets;
   init();

   ///< \todo-activate
   ui->headingFontPushButton->setVisible(false);
   ui->headingFontPushButton->setEnabled(false);
   ui->matchFontPushButton->setVisible(false);
   ui->matchFontPushButton->setEnabled(false);
   ui->editorLabel->setVisible(false);
   ui->editorPushButton->setVisible(false);
   ui->editorLineEdit->setVisible(false);
   ui->lookTextEdit->setVisible(false);
   //connect(ui->headingFontPushButton, SIGNAL(clicked()), this, SLOT(chooseHeadingFont()));
   //connect(ui->matchFontPushButton, SIGNAL(clicked()), this, SLOT(chooseMatchFont()));

   ripCheckBoxes();
}
/**************************************************************************//**
 * ~SettingsGlobal(): Destruct.
 ******************************************************************************/
SettingsGlobal::~SettingsGlobal  () {
   ENTEX2("SettingsGlobal::~SettingsGlobal", 0);
   delete presets;
   delete ui;
}
/**************************************************************************//**
 * AddPreset(): Add preset.
 ******************************************************************************/
void SettingsGlobal::addPreset  (Ui::Presets *_ui) {
   QString name = _ui->nameLineEdit->text(),
           desc = _ui->descLineEdit->text(),
           exts = _ui->extsLineEdit->text().trimmed();

   Dext dext(desc, exts.split(QRegExp("\\s+")));
   pm[name] = dext;
}
/**************************************************************************//**
 * CancelRestore(): Cancel button clicked.
 ******************************************************************************/
void SettingsGlobal::cancelRestore  () {
   ui->beforeCheckBox->setChecked(m_checkBoxes & CB_BEFORE);
   ui->overwriteCheckBox->setChecked(m_checkBoxes & CB_OVERWRITE);
   ui->cancelCheckBox->setChecked(m_checkBoxes & CB_CANCEL);
   ui->closingCheckBox->setChecked(m_checkBoxes & CB_CLOSING);
   ui->lastCheckBox->setChecked(m_checkBoxes & CB_LAST);
   ui->restoreCheckBox->setChecked(m_checkBoxes & CB_RESTORE);
   ui->newCheckBox->setChecked(m_checkBoxes & CB_NEW);
   ui->busyCheckBox->setChecked(m_checkBoxes & CB_BUSY);
   ui->accurateCheckBox->setChecked(m_checkBoxes & CB_ACCURATE);
   ui->ignoreCheckBox->setChecked(m_checkBoxes & CB_IGNORE);
   ui->linenoCheckBox->setChecked(m_checkBoxes & CB_LINENO);
   ui->whitespaceCheckBox->setChecked(m_checkBoxes & CB_WHITESPACE);

   ui->editorLineEdit->setText(m_editor);
   ui->matchesSpinBox->setValue(m_matches);
   ui->syntaxComboBox->setCurrentIndex(m_syntax);
}
/**************************************************************************//**
 * ChangeEvent(): Process change events.
 ******************************************************************************/
void SettingsGlobal::changeEvent  (QEvent *e) {
   QDialog::changeEvent(e);
   switch (e->type()) {
   case QEvent::LanguageChange:
       ui->retranslateUi(this);
       break;
   default:
       break;
   }
}
/**************************************************************************//**
 * ChooseFont(): allow the user to choose a font.
 *
 * Parms: _font - only modified if user chooses another font
 ******************************************************************************/
void SettingsGlobal::chooseFont  (QFont *_font) {
   bool ok;
   QFont font = QFontDialog::getFont(&ok);
    if (ok)
      *_font = font;
}
/**************************************************************************//**
 * ChooseHeadingFont(): allow the user to choose the match font.  [slot]
 ******************************************************************************/
void SettingsGlobal::chooseHeadingFont  () {
   chooseFont(&m_headingFont);
}
/**************************************************************************//**
 * ChooseMatchFont(): allow the user to choose the header font.  [slot]
 ******************************************************************************/
void SettingsGlobal::chooseMatchFont  () {
   chooseFont(&m_matchFont);
}
/**************************************************************************//**
 * Confirm(): execute operation prompt with option to stop prompting.
 *
 * Returns  : false on cancel operation, true otherwise
 ******************************************************************************/
bool SettingsGlobal::confirm  (quint64 box, const QString &title) {
   ENTEX2("SettingsGlobal::confirm", 0);

   if (!check(box))  // user does not want prompt
      return true;

   QCheckBox  again(tr("Don't prompt me again")),
             *gui;
   QString s,
           t;
   switch (box) {
      case CB_BEFORE:
         gui = ui->beforeCheckBox;
         s = tr("Really close tab?");
         t = tr("Close tab confirmation");
         break;
      case CB_CANCEL:
         gui = ui->cancelCheckBox;
         s = tr("Really cancel? Results may be incomplete.");
         t = title + " " + tr("Cancel confirmation");  // tab label...
         break;
      case CB_CLOSING:
         gui = ui->closingCheckBox;
         s = tr("Really exit program?");
         t = tr("Exit confirmation");
         break;
      case CB_OVERWRITE:
         gui = ui->overwriteCheckBox;
         s = tr("Overwrite current tab?");
         t = tr("Overwrite confirmation");
         break;
      default:
         break;
   }

   bool ret = ::boxConfirm(s, t, &again);

   if (again.isChecked()) {  // clear local bitmap and gui
      clear(box);
      gui->setChecked(false);
   }

   return ret;
}
/**************************************************************************//**
 * Init(): Init preset map with predefined string constants from header file
 *         and then with user settings, which may overwrite the former.
 ******************************************************************************/
void SettingsGlobal::init  () {
   ENTEX("SettingsGlobal::init");

   setPredefinedPresets();

   QSettings settings;
   settings.beginGroup("Presets");
   QStringList presets = settings.childGroups();
   qd(0) << "presets-" << presets;
   for (int i = 0, sz = presets.size(); i < sz; ++i) {
      settings.beginGroup(presets.at(i));
      pm[presets.at(i)] =
         Dext(settings.value("Description").toString(),
              settings.value("Extensions").toString().split(QRegExp("\\s+"), QString::SkipEmptyParts));
      settings.endGroup();
   }
}
/**************************************************************************//**
 * InitListWidget(): Initialize Presets ListWidget with presets.
 ******************************************************************************/
void SettingsGlobal::initListWidget  () {
   ENTEX("SettingsGlobal::initListWidget");

   ui->presetListWidget->clear();

   QMap<QString, Dext>::const_iterator i = pm.constBegin();
   int index = 0;
   while (i != pm.constEnd()) {
      QString s = i.key() + " - " + i.value().description;
      qd(0) << "inserting-" << s << " at pos: " << index;
      ui->presetListWidget->insertItem(index, s);
      ++index;
      ++i;
   }
}
/**************************************************************************//**
 * RipCheckBoxes(): rip global checkboxes into a handy and speedy bitmap.
 ******************************************************************************/
quint64 SettingsGlobal::ripCheckBoxes  () {

   if (ui->beforeCheckBox->isChecked())
      m_checkBoxes |= CB_BEFORE;
   if (ui->overwriteCheckBox->isChecked())
      m_checkBoxes |= CB_OVERWRITE;
   if (ui->cancelCheckBox->isChecked())
      m_checkBoxes |= CB_CANCEL;
   if (ui->closingCheckBox->isChecked())
      m_checkBoxes |= CB_CLOSING;
   if (ui->lastCheckBox->isChecked())
      m_checkBoxes |= CB_LAST;
   if (ui->restoreCheckBox->isChecked())
      m_checkBoxes |= CB_RESTORE;
   if (ui->newCheckBox->isChecked())
      m_checkBoxes |= CB_NEW;
   if (ui->busyCheckBox->isChecked())
      m_checkBoxes |= CB_BUSY;
   if (ui->accurateCheckBox->isChecked())
      m_checkBoxes |= CB_ACCURATE;
   if (ui->ignoreCheckBox->isChecked())
      m_checkBoxes |= CB_IGNORE;
   if (ui->linenoCheckBox->isChecked())
      m_checkBoxes |= CB_LINENO;
   if (ui->whitespaceCheckBox->isChecked())
      m_checkBoxes |= CB_WHITESPACE;

   return m_checkBoxes;
}
/**************************************************************************//**
 * SaveInCaseCancel(): save settings in case user cancels dialog.
 ******************************************************************************/
void SettingsGlobal::saveInCaseCancel  () {
   ripCheckBoxes();

   m_editor = ui->editorLineEdit->text();
   m_matches = ui->matchesSpinBox->value();
   m_syntax = ui->syntaxComboBox->currentIndex();
}
/**************************************************************************//**
 * SetPredefinedPresets(): set predefined presets into preset map.
 ******************************************************************************/
void SettingsGlobal::setPredefinedPresets  () {
   pm["C/C++"] = Dext("source code", extListCcpp);
   pm["Log"] = Dext("detects common logs", extListLog);
   pm["PHP"] = Dext("source code", extListPHP);
   pm["Perl"] = Dext("source code", extListPerl);
   pm["Python"] = Dext("source code", extListPython);
   pm["Text"] = Dext("some common plain text files", extListText);
   pm["Verilog"] = Dext("source code", extListVerilog);
   pm["Web"] = Dext("many common source files", extListWeb);
}
/**************************************************************************//**
 * ShowEvent(): execute when shown from hidden state (not restored).
 ******************************************************************************/
void SettingsGlobal::showEvent  (QShowEvent *event) {
   ENTEX("SettingsGlobal::showEvent");

   if (!event->spontaneous()) {  // if not restored from minimized state
      qd(2) << FUNC << "got non-spontaneous event [" << event->type() << "]";
      saveInCaseCancel();
   }
}
/**************************************************************************//**
 * Function group [slots]: headers generated by QtDesigner.
 ******************************************************************************/
void SettingsGlobal::on_settingsGlobalButtonBox_clicked  (QAbstractButton *button) {
   if (button->text() == "OK") {
      ripCheckBoxes();
      hide();
   }
   else if (button->text() == "Cancel") {
      cancelRestore();
      hide();
   }
   //else if (button->text() == "Apply") {}
   //ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}
void SettingsGlobal::on_restorePushButton_clicked  () {
   ui->beforeCheckBox->setChecked(true);
   ui->overwriteCheckBox->setChecked(false);
   ui->cancelCheckBox->setChecked(true);
   ui->closingCheckBox->setChecked(false);
   ui->lastCheckBox->setChecked(true);
   ui->restoreCheckBox->setChecked(true);
   ui->newCheckBox->setChecked(false);
   ui->busyCheckBox->setChecked(true);
   ui->accurateCheckBox->setChecked(false);
   ui->ignoreCheckBox->setChecked(false);
   ui->linenoCheckBox->setChecked(true);
   ui->whitespaceCheckBox->setChecked(true);

   ui->editorLineEdit->setText("notepad.exe");
   ui->matchesSpinBox->setValue(0);
   ui->syntaxComboBox->setCurrentIndex(0);

   setPredefinedPresets();
}
