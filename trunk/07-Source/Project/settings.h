/**************************************************************************//**
 * NAME:    settings.h
 *
 * PURPOSE: Class definition for local settings widget.
 *
 * DATE:    December 7, 2009
*******************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtGlobal>
#include <QDialog>
#include <QDir>

#include "presets/folders.h"
#include "presets/presets.h"

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QShowEvent;
QT_END_NAMESPACE

class Folders;
class Presets;

namespace Ui {
   class Folders;
   class MainWindow;
   class Presets;
   class Settings;
}

typedef struct SAVE_IN_CASE_CANCEL {
bool binary,
     cStyle;
int pool,
    matches,
    lines,
    depthMax,
    depthMin,
    targetIdx;
   QAbstractButton *binaryButton,
                   *targetButton;
   QString command;
} Sic;

class Settings : public QDialog {
Q_OBJECT
Q_DISABLE_COPY(Settings)
public:
   explicit Settings(Ui::MainWindow *_mui, QWidget *parent = 0);
   virtual ~Settings();
   void cancelRestore();
   QFileInfoList filterSkip(const QFileInfoList &fil) const;
   inline int getDepthMax() const { return m_depthMax; }
   inline int getDepthMin() const { return m_depthMin; }
   inline Folders *getFolders() const { return folders; }
   QString getKey(const QString &);
   inline Ui::Settings *getUi() const { return ui; }
   void initTargetComboBox();
   int processMore(const QStringList &_list);
   void reset();
   void resetLimits();
   void resetPresets();
   void saveInCaseCancel();
   int set(QStringList *);
   inline int skipFilterSz() const { return m_skipFilter.size(); }
   QStringList starDotted(const QStringList &_sl);
protected:
   virtual void changeEvent(QEvent *e);
   virtual void showEvent(QShowEvent *);
private:
   int m_depthMax,                   ///< search depth max
       m_depthMin;                   ///< "          " min
   Folders *folders;
   Sic sic;                          ///< save in case cancel structure
   Ui::Folders *fui;
   Ui::MainWindow *mui;
   Ui::Settings *ui;
   QButtonGroup *binaryButtonGroup,  ///< binary radio buttons
                *targetButtonGroup;  ///< target radio buttons
   QStringList m_also,
               m_only,
               m_skip,
               m_skipFilter;
public slots:
   void refreshComboBox(int);
   void setDepthMax(int);
   void setDepthMin(int);
private slots:
   void on_recursionPushButton_clicked();
   void on_subdirCheckBox_toggled(bool checked);
   //void on_quickContentLineEdit_editingFinished();
   //void on_contentLineEdit_editingFinished();
   void on_depthMinSpinBox_editingFinished();
   void on_depthMaxSpinBox_editingFinished();
   void on_defaultPresetsPushButton_clicked();
   void on_defaultLimitsPushButton_clicked();
   void on_overridePushButton_clicked();
   void on_cStyleCheckBox_stateChanged(int);
   void on_extensionRadioButton_toggled(bool checked);
   void on_contentsRadioButton_toggled(bool checked);
   void on_binaryCheckBox_stateChanged(int);
   void on_poolSpinBox_valueChanged(int);
   void on_depthMinSpinBox_valueChanged(int);
   void on_depthMaxSpinBox_valueChanged(int);
   void on_linesSpinBox_valueChanged(int);
   void on_matchesSpinBox_valueChanged(int);
   void on_settingsButtonBox_clicked(QAbstractButton* button);
   void on_commandLineEdit_textEdited(QString);
};

#endif // SETTINGS_H
