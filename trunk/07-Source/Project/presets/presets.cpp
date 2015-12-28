/**************************************************************************//**
 * NAME:    presets.cpp
 *
 * PURPOSE: Provides methods and data for search type presets.  Exists as a
 *          data attribute of class Settings.
 *
 * DATE:    December 12, 2009
 \todo
 allow spaces in preset names i.e. command-line arguments
*******************************************************************************/
#include "debug.h"

#include <QAbstractButton>
#include <QRegExpValidator>
#include <QStatusBar>

#include "settingsglobal.h"
#include "types.h"
#include "presets/presets.h"

#include "ui_presets.h"
/**************************************************************************//**
 * Presets(): Construct.
 ******************************************************************************/
Presets::Presets  (QWidget *parent)
:
QWidget(parent),
ui(new Ui::Presets)
{
   ui->setupUi(this);

   QRegExp rx("[^- ]{1,32767}");                /// string without '-' or ' '
   validator = new QRegExpValidator(rx, this);
   ui->nameLineEdit->setValidator(validator);
}
/**************************************************************************//**
 * ~Presets(): Destruct.
 ******************************************************************************/
Presets::~Presets  () {
   ENTEX2("Presets::~Presets", 0);
   delete ui;
}
/**************************************************************************//**
 * ChangeEvent(): Process change events.
 ******************************************************************************/
void Presets::changeEvent  (QEvent *e) {
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
 * OkApply(): Essentially adds/edits a preset.
 ******************************************************************************/
void Presets::okApply  () {
   if (ui->extsLineEdit->text().isEmpty())
      return;

   Singleton<SettingsGlobal>::instance()->addPreset(ui);

   ui->nameLineEdit->clear();
   ui->descLineEdit->clear();
   ui->extsLineEdit->clear();
}
/**************************************************************************//**
 * Function group [slots]: headers generated by QtDesigner.
 ******************************************************************************/
void Presets::on_buttonBox_clicked  (QAbstractButton *button) {
   if (button->text() == "OK") {
      okApply();
      hide();
   }
   else if (button->text() == "Apply") {
      ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
      okApply();
   }
   else if (button->text() == "Cancel") {
      ui->extsLineEdit->clear();
      hide();
   }
}