/**************************************************************************//**
 * NAME:    folders.cpp
 *
 * PURPOSE: Provides methods and data for folder inclusions and exclusions, a
 *          data attribute of class Settings.
 *
 * DATE:    September 27, 2010
 \todo
 a mem to test reactivation of apply button
*******************************************************************************/
#include "debug.h"

#include <QFile>
#include <QInputDialog>
#include <QStringList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QShowEvent>

#include "folders.h"
#include "util.h"

#include "ui_folders.h"
/**************************************************************************//**
 * Folders(): Construct.
 ******************************************************************************/
Folders::Folders(QWidget *parent)
:
QWidget(parent),
m_searchAll(false),
ui(new Ui::Folders)
{
   setWindowFlags(Qt::Dialog);

   ui->setupUi(this);

   ui->excludeListWidget->setSortingEnabled(true);
   ui->includeListWidget->setSortingEnabled(true);

   ui->searchAllFoldersCheckBox->setChecked(false);

   ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}
/**************************************************************************//**
 * ~Folders(): Destruct.
 ******************************************************************************/
Folders::~Folders() {
   ENTEX2("Folders::~Folders", 0);
   delete ui;
}
/**************************************************************************//**
 * Add(): Add a directory name to the exclude list.
   bool ok;
   QString s =
   QInputDialog::getText(this, QObject::tr("Enter a new directory name"),
                               QObject::tr("directory name:"),QLineEdit::Normal,
                               QString(), &ok);
 ******************************************************************************/
void Folders::add  () {
   QString dir = ::browse(this, FOLDERS);
   if (!dir.isEmpty() && !listContains(ui->excludeListWidget, dir)
                      && !listContains(ui->includeListWidget, dir)) {
      QListWidgetItem *newItem = new QListWidgetItem;
      newItem->setText(dir);
      ui->excludeListWidget->insertItem(0, newItem);
   }
}
/**************************************************************************//**
 * CancelRestore(): Cancel button clicked.
 ******************************************************************************/
void Folders::cancelRestore  () {
   ui->searchAllFoldersCheckBox->setChecked(m_searchAll);

   ui->includeListWidget->clear();
   ui->excludeListWidget->clear();
   list2ListWidget(m_saveLeft, ui->includeListWidget);
   list2ListWidget(m_saveRight, ui->excludeListWidget);
}
/**************************************************************************//**
 * ChangeEvent(): Handle change event.
 ******************************************************************************/
void Folders::changeEvent(QEvent *e) {
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
 * FilterFolders(): Determine if a directory is in the exclude list.
 ******************************************************************************/
bool Folders::filterFolders  (const QString &Folders) {
   return m_right.contains(Folders);
}
/**************************************************************************//**
 * FilterFolderss(): Filter Directory list based on excludes user wants to skip.
 ******************************************************************************/
QStringList Folders::filterFolders  (const QStringList &Folders) {
   return listSubtract(Folders, m_right);
}
/**************************************************************************//**
 * InitListWidget(): Put any configured values into list widget.
 ******************************************************************************/
void Folders::initListWidget  () {
   list2ListWidget(m_right, ui->excludeListWidget);
   list2ListWidget(m_left, ui->includeListWidget);
}
/**************************************************************************//**
 * ListContains(): Does list contain specified item specified by given string?
 ******************************************************************************/
bool Folders::listContains(const QListWidget *_lw, const QString &_s) {
   int cnt = _lw->count();
   if (cnt < 1)
      return false;

   for (int i = 0; i < cnt; ++i) {
      QListWidgetItem * item = _lw->item(i);
      QString data = (item->data(Qt::DisplayRole)).toString();
      if (data == _s)
         return true;
   }
   return false;
}
/**************************************************************************//**
 * MoveSelectedItem(): Move selected item from one list widget to another.
 ******************************************************************************/
void Folders::moveSelectedItem  (QListWidget *src, QListWidget *dst) {
   if (src->count() < 1)
      return;

   if (src->currentItem() && !src->currentItem()->isSelected())
      return;  // may only be selection rectangle

   QListWidgetItem *item = src->takeItem(src->row(src->currentItem()));

   dst->insertItem(0, item);

   ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
/**************************************************************************//**
 * MoveAllItems(): Move all items from one list widget to another.
 ******************************************************************************/
void Folders::moveAllItems  (QListWidget *src, QListWidget *dst) {
   int count = src->count();
   for (int i = 0; i < count; ++i)
      dst->insertItem(0, src->takeItem(src->row(src->item(0))));
   ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
/**************************************************************************//**
 * OkApply(): OK or Apply button clicked.
 ******************************************************************************/
 void Folders::okApply  () {
   listWidget2List(ui->excludeListWidget, &m_right);
   listWidget2List(ui->includeListWidget, &m_left);
}
/**************************************************************************//**
 * Reset(): Reset Folders automatic variables.
 ******************************************************************************/
void Folders::reset  () {
   m_left.clear();
   m_right.clear();
   m_saveLeft.clear();
   m_saveRight.clear();
}
/**************************************************************************//**
 * SaveInCaseCancel(): Save initial state in case user cancels.
 ******************************************************************************/
void Folders::saveInCaseCancel  () {
   m_searchAll = ui->searchAllFoldersCheckBox->isChecked();
   listWidget2List(ui->excludeListWidget, &m_saveRight);
   listWidget2List(ui->includeListWidget, &m_saveLeft);
}
/**************************************************************************//**
 * SetFolderFilter(): Set directory filter list based on user-defined excludes.
 ******************************************************************************/
void Folders::set  () {
   ENTEX("Folders::set");
   if (ui->searchAllFoldersCheckBox->isChecked()) {
      m_right.clear();
      return;
   }
   listWidget2List(ui->excludeListWidget, &m_right);
   qd(2) << "filter (m_right)-" << m_right;
}
/**************************************************************************//**
 * ShowEvent(): Execute when shown from hidden state (not restored).
 ******************************************************************************/
void Folders::showEvent  (QShowEvent *event) {
   ENTEX("Folders::showEvent");
   if (!event->spontaneous()) {  // if not restored from minimized state
      qd(3) << "got non-spontaneous event [" << event->type() << "]...calling saveInCaseCancel()";
      saveInCaseCancel();
   }
}
/**************************************************************************//**
 * Function group [slots]: headers generated by QtDesigner.
 ******************************************************************************/
void Folders::on_buttonBox_clicked(QAbstractButton *button) {
   ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

   if (button->text() == "OK") {
      okApply();
      hide();
   }
   else if (button->text() == "Apply") {
      okApply();
   }
   else if (button->text() == "Cancel") {
      cancelRestore();
      hide();
   }
}
void Folders::on_addPushButton_clicked() {
   add();
}
void Folders::on_defaultPushButton_clicked() {
   ui->searchAllFoldersCheckBox->setChecked(false);
   reset();
   ui->excludeListWidget->clear();
   ui->includeListWidget->clear();
   list2ListWidget(EXCLUDES_DFLT, ui->excludeListWidget);
}
void Folders::on_deletePushButton_clicked() {

   QListWidget *e = ui->excludeListWidget;  // shortcut

   if (e->currentItem() && !e->currentItem()->isSelected())
      return;  // may only be selection rectangle

   QListWidgetItem *item = e->takeItem(e->row(e->currentItem()));

   delete item;
}
void Folders::on_leftAllPushButton_clicked() {
   moveAllItems(ui->excludeListWidget, ui->includeListWidget);
}
void Folders::on_leftPushButton_clicked() {
   moveSelectedItem(ui->excludeListWidget, ui->includeListWidget);
}
void Folders::on_rightAllPushButton_clicked() {
   moveAllItems(ui->includeListWidget, ui->excludeListWidget);
}
void Folders::on_rightPushButton_clicked() {
   moveSelectedItem(ui->includeListWidget, ui->excludeListWidget);
}
void Folders::on_searchAllDirsCheckBox_toggled(bool toggle) {
   ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
   setSearchAll(toggle);

   ui->excludeListWidget->setDisabled(toggle);
   ui->includeListWidget->setDisabled(toggle);
   ui->addPushButton->setDisabled(toggle);
   ui->deletePushButton->setDisabled(toggle);
   ui->leftPushButton->setDisabled(toggle);
   ui->rightPushButton->setDisabled(toggle);
   ui->leftAllPushButton->setDisabled(toggle);
   ui->rightAllPushButton->setDisabled(toggle);
}
