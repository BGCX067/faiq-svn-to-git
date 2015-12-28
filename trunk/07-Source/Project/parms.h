/**************************************************************************//**
 * NAME:    parms.h
 *
 * PURPOSE: Class definition for find parameter class.
 *
 * DATE:    July 5, 2010
 \todo
 a button states - search running?
 b layout state
 c quick/adv
 d visible dock widgets?
 e settings!!!
*******************************************************************************/
#ifndef PARMS_H
#define PARMS_H

#include <QDateTime>
#include <QObject>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "util.h"

const bool SM_INITPARMS = true;

QT_BEGIN_NAMESPACE
class QFont;
class QWidget;
QT_END_NAMESPACE

class MainWindow;
class Settings;
class SettingsGlobal;
class Tab;

namespace Ui {
  class MainWindow;
}

class Parms {
public:
   Parms(Tab *tab, MainWindow *mw);
   virtual ~Parms() {}

   inline bool operator()(quint64 checkBox) { return checkBox & m_checkBoxes; }

   bool caseSensitive() const;
   Qt::CaseSensitivity caseSensitivity() const;
   inline const QRegExp &getRegExp() const { return m_rx; }
   bool modeDateTime() const;
	bool modeInvert() const;
   bool modeShortcut() const;
   bool modeSize() const;
	bool modeSkipHidden() const;
   bool modeSymLink() const;
   void burnContext();
   void ripContext(bool init = false);
   void ripGlobalSettings();
   void setCheckBoxes();
   void setFilelist();
   void updateTabLabel();
   bool validate();

   bool dateTimeEnabled,
		  dateTime2Enabled,
		  sizeEnabled,
		  size2Enabled,
		  sizeUnitsEnabled,
		  sizeUnits2Enabled,
		  skipHidden,
        searchSubs,
        followShortcuts,  // Windows
        followSymlinks,   // Linux...
		  visLogHistory,
		  visResultsBrowser,
		  visSearchDialog;
	int dateTimeIdx,
		 replaceIdx,
		 sizeIdx,
		 sizeUnitsIdx,
		 sizeUnits2Idx,
       syntaxIdx;
   quint32 findMatches;
   quint64 m_checkBoxes;     // bitmap
   Layout layout;            // enum
   MainWindow *mw;
   State state;              // enum
   Tab *tab;
   Ui::MainWindow *ui;
	Qt::CaseSensitivity kase; // enum
   QDateTime m_createStamp,
             dateTime,
             dateTime2;
   QFont *m_headingFont,
         *m_matchFont;
   QRegExp m_rx;
	QString content,
           dateTimeTxt,
           m_editor,
           filenames,  // literal string from lineedit
           mainStatus, // main window lower left
			  path,
			  replace,
			  size,
			  size2,
			  sizeTxt,
			  sizeUnits,
           sizeUnits2;
   QStringList filelist;  // literal string parsed into list
};

#endif // PARMS_H
