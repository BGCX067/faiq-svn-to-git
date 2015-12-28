/**************************************************************************//**
 * NAME:    settings.cpp
 *
 * PURPOSE: Implementation of local settings widget.
 *
 * DATE:    December 7, 2009
 *
 \todo
 a void QTreeView::resizeColumnToContents ( int column ) to "fit" columns
   bool QTreeView::isColumnHidden ( int column ) const
   bool QAbstractItemModel::insertColumn ( int column, const QModelIndex & parent = QModelIndex() )
   showColumn(), hideColumn()
*******************************************************************************/
#include "debug.h"

#include <QListWidget>
#include <QMap>
#include <QShowEvent>
#include <QStatusBar>
#include <QString>

#include "settings.h"
#include "settingsglobal.h"
#include "util.h"
#include "presets/folders.h"
#include "presets/presets.h"

#include "ui_folders.h"
#include "ui_mainwindow.h"
#include "ui_presets.h"
#include "ui_settings.h"
/**************************************************************************//**
 * Settings(): Construct.
 ******************************************************************************/
Settings::Settings(Ui::MainWindow *_mui, QWidget *parent)
:
QDialog(parent),
folders(new Folders(this)),
fui(folders->getUi()),
mui(_mui),
ui(new Ui::Settings),
binaryButtonGroup(new QButtonGroup),
targetButtonGroup(new QButtonGroup)
{
   ENTEX2("Settings::Settings", 2);

   ui->setupUi(this);

   // Setup the Limits tab.
   ui->poolLabel->setVisible(false);    ///< \todo start using
   ui->poolSpinBox->setVisible(false);
   bool subdir = mui->subdirCheckBox->isChecked();
   ui->depthMaxSpinBox->setEnabled(subdir);
   ui->depthMinSpinBox->setEnabled(subdir);
   ui->recursionPushButton->setText(mui->subdirCheckBox->isChecked() ? "Disable folder recursion"
                                                                     : "Enable folder recursion");
   resetLimits();

   // Setup the Presets tab.
   initTargetComboBox();

   targetButtonGroup->addButton(ui->alsoRadioButton);
   targetButtonGroup->addButton(ui->onlyRadioButton);
   targetButtonGroup->addButton(ui->skipRadioButton);
   ui->settingsTabWidget->setEnabled(true);
   ui->groupBox->setEnabled(true);
   ui->linesSpinBox->setEnabled(true);
   ui->matchesSpinBox->setEnabled(true);
   ui->alsoRadioButton->setChecked(true);
   binaryButtonGroup->addButton(ui->contentsRadioButton);
   binaryButtonGroup->addButton(ui->extensionRadioButton);
   bool binary = ui->binaryCheckBox->isChecked();
   ui->contentsRadioButton->setEnabled(binary);
   ui->extensionRadioButton->setEnabled(binary);

   resetPresets();

   // Setup the Columns tab.
   ui->columnsWidget->setEnabled(false);
   ui->columnsWidget->setVisible(false);

   connect(ui->commandLineEdit, SIGNAL(textEdited(QString)), this, SLOT(on_commandLineEdit_textEdited(QString)));
   connect(ui->targetComboBox, SIGNAL(activated(int)), this, SLOT(refreshComboBox(int)));
}
/**************************************************************************//**
 * ~Settings(): Destruct.
 ******************************************************************************/
Settings::~Settings() {
   ENTEX2("Settings::~Settings", 0);
   delete folders;
   delete ui;
}
/**************************************************************************//**
 * CancelRestore(): If user presses Cancel button restore initial settings.
 ******************************************************************************/
void Settings::cancelRestore  () {
   ENTEX("Settings::cancelRestore");
   ui->binaryCheckBox->setChecked(sic.binary);
   ui->cStyleCheckBox->setChecked(sic.cStyle);
   ui->poolSpinBox->setValue(sic.pool);
   ui->matchesSpinBox->setValue(sic.matches);
   ui->linesSpinBox->setValue(sic.lines);
   ui->depthMaxSpinBox->setValue(sic.depthMax);
   ui->depthMinSpinBox->setValue(sic.depthMin);
   ui->targetComboBox->setCurrentIndex(sic.targetIdx);
   sic.binaryButton->setChecked(true);
   sic.targetButton->setChecked(true);
   ui->commandLineEdit->setText(sic.command);
}
/**************************************************************************//**
 * ChangeEvent(): Handle change event.
 ******************************************************************************/
void Settings::changeEvent(QEvent *e) {
	switch (e->type()) {
	case QEvent::LanguageChange:
      ui->retranslateUi(this);
		break;
	default:
		break;
   }
}
/**************************************************************************//**
 * FilterSkip(): Filter QFileInfoList based on presets user wants to skip.
 ******************************************************************************/
QFileInfoList Settings::filterSkip  (const QFileInfoList &fil) const {
   ENTEX("Settings::filterSkip");

   QFileInfoList subset;
   for (int i = 0, sz = fil.size(); i < sz; ++i) {
      if (!m_skipFilter.contains("*." + fil[i].suffix()) &&
          !m_skipFilter.contains("*." + fil[i].completeSuffix()))
         subset << fil[i];
   }
   return subset;
}
/**************************************************************************//**
 * GetKey(): Grab key from beginning of preset string: <key> - <description>
 ******************************************************************************/
QString Settings::getKey  (const QString &_s) {
   ENTEX("Settings::getKey");

   int idx = _s.indexOf("-");
   QString s = _s.left(idx).trimmed();
   qd(3) << "got key-" << s;
   return s;
}
/**************************************************************************//**
 * InitTargetComboBox(): Initialize combobox with presets.
 ******************************************************************************/
void Settings::initTargetComboBox  () {
   ENTEX("Settings::initTargetComboBox");

   int index = 0;
   QMapIterator<QString, Dext> i(Singleton<SettingsGlobal>::instance()->getPmRef());
   while (i.hasNext()) {
      i.next();

      QString s = i.key() + " - " + i.value().description;

      qd(4) << "inserting-" << s << " at: " << index;
      ui->targetComboBox->insertItem(index++, s);
   }
   ui->targetComboBox->setCurrentIndex(0);
}
/**************************************************************************//**
 * ProcessMore(): Process arguments in the Command Line lineedit:
 *                [-also type] [-only type] [-skip type] [...]
 *
 * Parms        : arguments
 *
 * Returns      : 0 on success, -1 on usage error
 ******************************************************************************/
 int Settings::processMore  (const QStringList &_list) {
   ENTEX2("Settings::processMore", 3);

   if (_list.size() % 2)
      return -1;

   SettingsGlobal * sg = Singleton<SettingsGlobal>::instance();

   for (int i = 0, sz = _list.size(); i < sz; i += 2) {
      QString s = _list[i];      // the switch
      QString v = _list[i + 1];  // the value

      if (!sg->getPmRef().contains(v))
         continue;  // already searching for this type

      QStringList sl = ::starDotAdd(sg->getPmRef()[v].extensions);

      if (s == "-also")
         m_also += sl;
      else if (s == "-only")
         m_only += sl;
      else if (s == "-skip")
         m_skip += sl;
      else
         return -1;
   }

   return 0;
}
/**************************************************************************//**
 * RefreshComboBox(): Since entries may be added or edited globally.  [slot]
 ******************************************************************************/
void Settings::refreshComboBox  (int) {
   ENTEX("Settings::refreshComboBox");
   ui->targetComboBox->clear();
   initTargetComboBox();
}
/**************************************************************************//**
 * Reset(): Refresh.
 ******************************************************************************/
void Settings::reset() {
   folders->reset();
}
/**************************************************************************//**
 * ResetLimits(): Reset Limits tab.
 ******************************************************************************/
void Settings::resetLimits  () {
   ui->poolSpinBox->setValue(2);
   ui->matchesSpinBox->setValue(250);
   ui->linesSpinBox->setValue(0);
   ui->depthMaxSpinBox->setValue(m_depthMax = 8);
   ui->depthMinSpinBox->setValue(m_depthMin = 0);
}
/**************************************************************************//**
 * ResetPresets(): Reset Presets tab.
 ******************************************************************************/
void Settings::resetPresets  () {
   ui->binaryCheckBox->setChecked(true);
   ui->extensionRadioButton->setChecked(true);
   ui->cStyleCheckBox->setChecked(false);
}
/**************************************************************************//**
 * SaveInCaseCancel(): Save snapshot in case user changes then cancels.
 ******************************************************************************/
void Settings::saveInCaseCancel  () {
   sic.binary = ui->binaryCheckBox->isChecked();
   sic.cStyle = ui->cStyleCheckBox->isChecked();
   sic.pool = ui->poolSpinBox->value();
   sic.matches = ui->matchesSpinBox->value();
   sic.lines = ui->linesSpinBox->value();
   sic.depthMax = ui->depthMaxSpinBox->value();
   sic.depthMin = ui->depthMinSpinBox->value();
   sic.targetIdx = ui->targetComboBox->currentIndex();
   sic.binaryButton = binaryButtonGroup->checkedButton();
   sic.targetButton = targetButtonGroup->checkedButton();
   sic.command = ui->commandLineEdit->text();
}
/**************************************************************************//**
 * Set(): Based on all extensions indicated by the user, load up a string list
 *        corresponding to each type of usage - also, only, skip.
 *
 * Parms  : patterns - original user-entered pattern list
 *
 * Returns: -1 on usage, -2 if options cancel out, 0 otherwise.
 ******************************************************************************/
int Settings::set  (QStringList *patterns) {
   ENTEX2("Settings::set", 3);
   //
   // Process Filenames combobox.
   //
   m_also = *patterns;

#if 0
   //
   // Process Target combobox.
   //
   QString key = getKey(ui->targetComboBox->currentText());

   if (!key.isEmpty()) {

      QStringList values = ::starDotAdd(Singleton<SettingsGlobal>::instance()->pm[key].extensions);

      if (ui->alsoRadioButton->isChecked())
         m_also += values;
      else if (ui->onlyRadioButton->isChecked())
         m_only = values;
      else if (ui->skipRadioButton->isChecked())
         m_skip = values;
      else
         Q_ASSERT(0);
   }
#endif

   //
   // Process Command Line combobox
   //
   QString s = ui->commandLineEdit->text().trimmed();
   QStringList list = s.split(QRegExp("\\s+"));
   if (!s.isEmpty()) {
      if (processMore(list) == -1) {
         ::boxError(tr("Usage: [-also type] [-only type] [-skip type] "
                    "[...]"));

         return(-1);
      }
   }
   qDebug() << "also-" << m_also;
   qDebug() << "only-" << m_only;
   qDebug() << "skip-" << m_skip;

   //
   // Now that we have a string list for also, only and skip, use the following
   // logic to reduce these lists to a single list of patterns.
   //
   list.clear();
   list = m_only.size() ? m_only : m_also;  // only trumps also

   if (list.contains("*")) {  // search all files with a skip filter
      patterns->clear();
      *patterns << "*";
      m_skipFilter = m_skip;
      qDebug() << "set skip filter-" << m_skipFilter;
      return 0;
   }

   if (m_skip.size())
      list = listSubtract(list, m_skip);

   if (list.empty())  // user options cancelled out, nothing to do
      return -2;

   *patterns = list;

   if (!validRx(*patterns, QRegExp::Wildcard)) {
      ::boxError(QString(QObject::tr("Invalid Filename(s)")));
      return -1;
   }

   return 0;

}  // set()
/**************************************************************************//**
 * SetDepthMax(): Set max search depth from signal.  [slot]
 ******************************************************************************/
void Settings::setDepthMax  (int depth) {
   m_depthMax = depth;
}
/**************************************************************************//**
 * SetDepthMin(): Set min search depth from signal.  [slot]
 ******************************************************************************/
void Settings::setDepthMin  (int depth) {
   m_depthMin = depth;
}
/**************************************************************************//**
 * ShowEvent(): Execute when shown from hidden state (not restored).
 ******************************************************************************/
void Settings::showEvent  (QShowEvent *event) {
   ENTEX("Settings::showEvent");
   if (!event->spontaneous()) {  // if not restored from minimized state
      qd(2) << "got non-spontaneous event [" << event->type() << "]...calling " << FUNC;
      saveInCaseCancel();
   }
}
/**************************************************************************//**
 * Function group [slots]: headers generated by QtDesigner.
 ******************************************************************************/
void Settings::on_binaryCheckBox_stateChanged(int _state) {
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
   bool checked = _state == Qt::Checked;
   ui->contentsRadioButton->setEnabled(checked);
   ui->extensionRadioButton->setEnabled(checked);
}
void Settings::on_commandLineEdit_textEdited  (QString)  {
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);

   QStatusBar sb(this);  ///< \todo-test all this
   sb.setVisible(true);
   sb.raise();
   sb.showMessage("Usage: [-also type] [-only type] [-skip type] [...]");

   this->setStatusTip("Usage: [-also type] [-only type] [-skip type] [...]");
}
void Settings::on_contentsRadioButton_toggled(bool checked) {
   (void)checked;
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
void Settings::on_cStyleCheckBox_stateChanged(int) {
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
void Settings::on_defaultLimitsPushButton_clicked() {
   resetLimits();
}
void Settings::on_defaultPresetsPushButton_clicked() {
   resetPresets();
}
void Settings::on_depthMaxSpinBox_editingFinished() {
   setDepthMax(ui->depthMaxSpinBox->value());
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
void Settings::on_depthMinSpinBox_editingFinished() {
   setDepthMin(ui->depthMinSpinBox->value());
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
void Settings::on_depthMaxSpinBox_valueChanged(int val) {
   if (val <= m_depthMin)
      ui->depthMaxSpinBox->setValue(m_depthMin + 1);
}
void Settings::on_depthMinSpinBox_valueChanged(int val) {
   if (val >= m_depthMax)
      ui->depthMaxSpinBox->setValue(m_depthMax - 1);
}
void Settings::on_extensionRadioButton_toggled(bool checked) {
   (void) checked;
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
void Settings::on_linesSpinBox_valueChanged(int) {
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
void Settings::on_matchesSpinBox_valueChanged(int) {
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
void Settings::on_overridePushButton_clicked() {
   folders->show();
}
void Settings::on_poolSpinBox_valueChanged(int) {
   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}
void Settings::on_settingsButtonBox_clicked(QAbstractButton *button) {

   if (button->text() == "OK") {
      hide();
   }
   else if (button->text() == "Cancel") {
      cancelRestore();
      hide();
   }
   else if (button->text() == "Apply") {
      ;
   }
   else Q_ASSERT(0);

   ui->settingsButtonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}
/*
void Settings::on_contentLineEdit_editingFinished() {
   ui->linesSpinBox->setEnabled(!mui->contentLineEdit->text().isEmpty());
   ui->matchesSpinBox->setEnabled(!mui->contentLineEdit->text().isEmpty());
}
void Settings::on_quickContentLineEdit_editingFinished() {
   ui->linesSpinBox->setEnabled(!mui->quickContentLineEdit->text().isEmpty());
   ui->matchesSpinBox->setEnabled(!mui->quickContentLineEdit->text().isEmpty());
}
*/
void Settings::on_subdirCheckBox_toggled(bool checked) {
   ui->depthMaxSpinBox->setEnabled(checked);
   ui->depthMinSpinBox->setEnabled(checked);
   ui->recursionPushButton->setText(checked ? "Disable folder recursion"
                                            : "Enable folder recursion");
}
void Settings::on_recursionPushButton_clicked() {
   if (ui->recursionPushButton->text() == "Disable folder recursion") {
      mui->subdirCheckBox->setChecked(false);
      ui->recursionPushButton->setText("Enable folder recursion");
   }
   else {
      mui->subdirCheckBox->setChecked(true);
      ui->recursionPushButton->setText("Disable folder recursion");
   }
}
