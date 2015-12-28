/**************************************************************************//**
 * NAME:    find.h
 *
 * PURPOSE: Definitions for class Find.
 *
 * DATE:    July 4, 2010
*******************************************************************************/
#ifndef FIND_H
#define FIND_H

#include <QDir>
#include <QFlags>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QVector>

#include "settingsglobal.h"
#include "types.h"

const qreal FIND_PROGRESS_RANGE = 14000.00;  /// avg of my system drive and cygwin
const qint32 FIND_PROGRESS_RANGE_INT = 14000;

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

///< \todo
// a check torrent, pdf, msc, js, url, rtf, (lnk) (binary? but lots of ascii)
// b case insensitive?
// c make configurable?
const QStringList BINARY_EXTS = (QStringList()
<< "7z" << "a" << "au" << "avi" << "ax" << "bin" << "bmp" << "cab" << "chm"
<< "class" << "com" << "cpl" << "dll" << "doc" << "docx" << "exe" << "fon"
<< "gif" << "gz" << "hta" << "idb" << "jar" << "jpg" << "m" << "mdb" << "MID"
<< "mp3" << "mp4" << "mpg" << "msc" << "msi" << "msstyles" << "ncb" << "o"
<< "obj" << "ocx" << "pch" << "pdb" << "pfb" << "pnf" << "png" << "ppt"
<< "pptx" << "pst" << "pyc" << "pyo" << "rar" << "res" << "scr" << "sfx"
<< "suo" << "svg" << "swf" << "sys" << "tar" << "TIF" << "ttf" << "uue" << "vtg"
<< "wav" << "wma" << "wmv" << "wtv" << "xls" << "xlsx" << "zip" << "zipx");
const QStringList TEXT_EXTS = (QStringList()  // some duplication here
<< "awk" << "bat" << "c" << "cc" << "cmd" << "conf" << "cpp" << "cs"
<< "csproj" << "css" << "cxx" << "d" << "Debug" << "dep" << "dic" << "fig"
<< "glade" << "h" << "hh" << "hpp" << "html" << "html" << "hxx" << "ini"
<< "inf" << "java" << "jhtml" << "log" << "ltx" << "lyx" << "m" << "manifest"
<< "m3u" << "m3u8" << "md5" << "php" << "phpt" << "php3" << "php4" << "php5"
<< "pl" << "pro" << "py" << "pyw" << "qrc" << "rc" << "reg" << "Release"
<< "resx" << "settings" << "scm" << "shtml" << "sing" << "sln" << "snippet"
<< "sql" << "svn-base" << "tcc" << "tcl" << "tex" << "tlog" << "txt" << "theme"
<< "ui" << "user" << "vb" << "vbs" << "vbproj" << "vcproj" << "vpj" << "vpw"
<< "vpwhist" << "WMF" << "wpl" << "wsf" << "wsc" << "xaml" << "xhtml" << "xml"
<< "xrm-ms" << "xsd" << "xsl" << extListCcpp << extListJava << extListLog
<< extListPerl << extListPHP << extListPython << extListText << extListVerilog
<< extListWeb << extListWins);

class Parms;
class Settings;
class Tab;

class Find : public QObject {
Q_OBJECT
public:
   Find(Tab *tab);
	virtual ~Find();
   QFileInfoList complement(QFileInfoList &fil, const QDir &dir) const;
   void countDirs(const QDir &dir);
   void countDirsRecursively(const QDir &dir);
   QStringList dbgEntryList(const QStringList &files, const QDir &currentDir) const;
   QStringList dbgEntryInfoList(const QFileInfoList &fil) const;
   QFileInfoList filterBinary(const QFileInfoList &fil);
   QFileInfoList filterDateTime(const QFileInfoList &fil) const;
   QStringList filterNonDirs(const QStringList &dirs, const QDir &currentDir) const;
   QFileInfoList filterNonFiles(const QFileInfoList &fil) const;
   QFileInfoList filterSize(const QFileInfoList &fil) const;
   QFileInfoList filterWinOddities(const QFileInfoList &fil) const;
   QStringList filterShortcuts(const QStringList &dirs) const;
   void find(const QString &path);
   QFlags<QDir::Filter> flags() const;
   int findInit();
   QStringList getEntryList(const QDir &dir) const;
   QFileInfoList matchOnce(const QFileInfoList &fil);
   QFileInfoList resolveShortcuts(const QFileInfoList &fil) const;
   QStringList resolveShortcuts(const QStringList &_files, const QDir &_currentDir) const;
   void showProgress(int depthChanged);
   inline void showProgressBar();
private:
   bool m_stopDirCnt;   ///< mainwindow signals to stop presearch dir count
   int m_dirCount,      ///< progress bar ticks
       m_dirs,          ///< progress bar range
       m_progress;      ///< a "local static" for find() (see Notes)
   qint16 m_depth;      ///< depth tracker
   qint32 m_numFiles;   ///< total files processed
   Parms *p;            ///< find parameters
   Tab *tab;            ///< to check run state, send signals and access local settings
   Ui::Settings *sui;   ///< shortcut ptr to local settings ui
   QMap<int, int> m_dirsAtDepth;
   QTime m_timer;
public slots:
   void stopPreSearchDirCnt();
signals:
	void boxError(const QString &, const QMessageBox::Icon = QMessageBox::Warning, Error err = ERR_INVALID);
	void findProgressBar_setFormat(QString);
   void findProgressBar_setRange(int, int);
   void findProgressShow(bool);
   void findProgressBar_addValue(int);
   void findProgressBar_setValue(int);
   void showFiles(QFileInfoList files);
   void statusShow(const QString &, Operation op = FIND);
};

#endif
