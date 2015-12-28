/**************************************************************************//**
 * NAME:    settingsgloabal.h
 *
 * PURPOSE: Class definition for global settings widget.
 *
 * DATE:    September 30, 2010
*******************************************************************************/
#ifndef SETTINGSGLOBAL_H
#define SETTINGSGLOBAL_H

#include <QtGlobal>

#include <QDialog>
#include <QFont>
#include <QStringList>

#include "presets/presets.h"

#include "ui_presets.h"

// Global settings' checkboxes.
const quint64 CB_BEFORE  = 1ULL << 0;
const quint64 CB_OVERWRITE = 1ULL << 1;
const quint64 CB_CANCEL = 1ULL << 2;
const quint64 CB_CLOSING = 1ULL << 3;
const quint64 CB_LAST = 1ULL << 4;
const quint64 CB_RESTORE = 1ULL << 5;
const quint64 CB_NEW = 1ULL << 6;
const quint64 CB_BUSY = 1ULL << 7;
const quint64 CB_ACCURATE = 1ULL << 8;
const quint64 CB_IGNORE = 1ULL << 9;
const quint64 CB_LINENO = 1ULL << 10;
const quint64 CB_WHITESPACE = 1ULL << 11;
const quint64 CB_MASK = ULLONG_MAX;

const QStringList extListNoExt = (QStringList()
<< "");
const QStringList extListCcpp = (QStringList()
<< "c" << "c__" << "c++" << "c--" << "cc" << "cpp" << "cxx" << "h" << "hh"
<< "hpp" << "hxx");
const QStringList extListJava = (QStringList()
<< "j" << "jav" << "java" << "jbc" << "jcm" << "js" << "jse" << "jsfl"
<< "jst" << "jsxinc" << "mmjs");
const QStringList extListLog  = (QStringList()
<< "7" << "adiumlog" << "asl" << "awp" << "cgl" << "cil" << "dce" << "dte"
<< "dtg" << "dth" << "err" << "full" << "gthr" << "hwl" << "klg" << "lgd"
<< "lgr" << "lgz" << "lo_" << "log" << "nmbd" << "plg" << "resp" << "ted"
<< "wlg");
const QStringList extListPerl = (QStringList()
<< "perl" << "ph" << "pl" << "pls" << "plx" << "pm" << "pod" << "t");
const QStringList extListPHP  = (QStringList()
<< "ph3" << "phl" << "php" << "php2" << "php3" << "php4" << "php5" << "phps"
<< "phtm" << "phtml");
const QStringList extListPython = (QStringList()
<< "epp" << "py" << "python" << "pyw");
const QStringList extListText = (QStringList()
<< "asc" << "ascii" << "bib" << "charset" << "cfg" << "csv" << "diz" << "ini"
<< "jis" << "latex" << "man" << "log" << "nfo" << "strings" << "tab" << "tex"
<< "text" << "txt" << "unx" << "utxt" << "zw");
const QStringList extListVerilog = (QStringList()
<< "v" << "verilog" << "vhd" << "vhdl");
const QStringList extListWeb = (QStringList()
<< "ahtm" << "ahtml" << "as" << "asax" << "asp" << "asp+" << "aspx" << "axd"
<< "cfml" << "cgi" << "fhtml" << "hsql" << "htc" << "htd" << "htm" << "html"
<< "htmls" << "inc" << "jsp" << "jst" << "php" << "lmv" << "mod" << "mht"
<< "mhtm" << "mhtml" << "mspx" << "mtml" << "sht" << "shtm" << "shtml" << "sml"
<< "ssi" << "stm" << "stml" << "thtml" << "ttml" << "wml" << "xht" << "xhtm"
<< "xhtml" << "xml");
const QStringList exeListWinx = (QStringList()
<< "chm" << "com" << "cpl" << "exe" << "hta" << "mdb" << "msc" << "msi"
<< "ocx" << "scr");
const QStringList extListWins = (QStringList()
<< "bat" << "cmd" << "js" << "jse" << "reg" << "vb" << "vbe" << "vbs" << "wsf");

struct Dext {
   Dext() {}
   Dext(QString d, QStringList e)
   :
   description(d),
   extensions(e) {}
   Dext(const Dext &dext) {
      description = dext.description;
      extensions = dext.extensions;
   }
   virtual ~Dext() {}

   QString description;
   QStringList extensions;
};

typedef QMap<QString, Dext> PresetMap;

QT_BEGIN_NAMESPACE
class QAbstractButton;
QT_END_NAMESPACE

namespace Ui {
   class Presets;
   class SettingsGlobal;
}

class SettingsGlobal : public QDialog {
Q_OBJECT
Q_DISABLE_COPY(SettingsGlobal)
public:
   explicit SettingsGlobal(QWidget *parent = 0);
   virtual ~SettingsGlobal();
   void addPreset(Ui::Presets *);
   void cancelRestore();
   inline bool check(quint64 box) const { return m_checkBoxes & box; }
   inline bool clear(quint64 box) { return m_checkBoxes &= ~box; }
   void chooseFont(QFont *font);
   bool confirm(quint64 box, const QString &title = "");
   inline QFont getHeadingFont() const { return m_headingFont; }
   inline QFont getMatchFont() const { return m_matchFont; }
   inline PresetMap &getPmRef() { return pm; }
   inline PresetMap *getPmPtr() { return &pm; }
   inline Ui::SettingsGlobal *getUi() const { return ui; }
   void init();
   void initListWidget();
   quint64 ripCheckBoxes();
   void saveInCaseCancel();
   void setPredefinedPresets();
   void showEvent(QShowEvent *event);
   inline bool userChangedHeadingFont() const { return m_headingFont != m_referenceFont; }
   inline bool userChangedMatchFont() const { return m_matchFont != m_referenceFont; }
protected:
   void changeEvent(QEvent *e);
private:
   int m_matches,
       m_syntax;
   quint64 m_checkBoxes;
   Ui::SettingsGlobal *ui;
   PresetMap pm;
   Presets *presets;
   QFont m_headingFont,
         m_matchFont,
         m_referenceFont;
   QString m_editor;
public slots:
   void on_restorePushButton_clicked();
private slots:
   void chooseHeadingFont();
   void chooseMatchFont();
   void on_settingsGlobalButtonBox_clicked(QAbstractButton *button);
};

#endif // SETTINGSGLOBAL_H
