/**************************************************************************//**
 * NAME:    find.cpp
 *
 * PURPOSE: Find files based on criteria, one directory at a time.
 *
 * DATE:    July 4, 2010
 \todo
 -----
 a send sigs by value? (we don't know when receiving thread will pick up event)
*******************************************************************************/
#include "debug.h"

#include <QFile>
#include <QFileInfoList>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QTimer>

#include <QtGlobal>

#include "find.h"
#include "mainwindow.h"
#include "parms.h"
#include "settings.h"
#include "settingsglobal.h"
#include "tab.h"
#include "util.h"
#include "presets/folders.h"
#include "presets/presets.h"

#include "ui_presets.h"
#include "ui_settings.h"
#include "ui_settingsglobal.h"
/**************************************************************************//**
 * Find(): construct a Find object.
 ******************************************************************************/
Find::Find  (Tab *_tab)
:
m_stopDirCnt(false),
m_dirCount(0),
m_dirs(0),
m_progress(0),
m_depth(0),
m_numFiles(0),
p(_tab->getParms()),  // shortcut
tab(_tab),
sui(tab->getSettings()->getUi())  // shortcut
{
   ENTEX2("Find::Find", 0);
   connect(this, SIGNAL(boxError(QString,QMessageBox::Icon,Error)), tab, SLOT(boxError(QString,QMessageBox::Icon,Error)));
	connect(this, SIGNAL(findProgressBar_setFormat(QString)), tab, SLOT(on_findProgressBar_setFormat(QString)));
   connect(this, SIGNAL(findProgressBar_setRange(int,int)), tab, SLOT(on_findProgressBar_setRange(int,int)));
   connect(this, SIGNAL(findProgressShow(bool)), tab, SLOT(on_findProgressShow(bool)));
   connect(this, SIGNAL(findProgressBar_addValue(int)), tab, SLOT(on_findProgressBar_addValue(int)));
   connect(this, SIGNAL(findProgressBar_setValue(int)), tab, SLOT(on_findProgressBar_setValue(int)));
   connect(this, SIGNAL(showFiles(QFileInfoList)), tab, SLOT(showFiles(QFileInfoList)));
   connect(this, SIGNAL(statusShow(QString,Operation)), tab, SLOT(statusShow(QString,Operation)));
   connect(tab->getMw(), SIGNAL(stopPreSearchDirCnt()), this, SLOT(stopPreSearchDirCnt()));

   m_timer.start();
}
/**************************************************************************//**
 * ~Find(): destruct.
 ******************************************************************************/
 Find::~Find  () {
   ENTEX2("Find::~Find", 0);
}
/**************************************************************************//**
 * Complement(): find complement of the given set of files.
 * \todo-test this function (inverted search)
 ******************************************************************************/
QFileInfoList Find::complement  (QFileInfoList &_files, const QDir &_dir) const{

   QFileInfoList files = _dir.entryInfoList(QStringList("*"), QDir::Files | flags(), QDir::Reversed);

#ifdef Q_OS_WIN32
   files = filterWinOddities(files);
   if (p->modeShortcut())  // todo-add support (if needed) for win7 handling various links
      files = resolveShortcuts(files);  // conversion only (no inversion)
#endif
   files = filterNonFiles(files);  // I don't trust entryInfoList()

   return listSubtract(files, _files);
}
/**************************************************************************//**
 * DbgEntryList(): Output a file list for detailed debug analysis.
 ******************************************************************************/
QStringList Find::dbgEntryList  (const QStringList &files,
                                 const QDir &currentDir) const {
   ENTEX("Find::dbgEntryList");
   QStringList dbg;
   for (int i = 0; i < files.size(); i++) {
      QFileInfo fi(currentDir.absoluteFilePath(files[i]));

      if (fi.isDir() && fi.isFile())
         qd(3) << "item " << fi.canonicalFilePath() << " is a file AND a dir";

      QString s = fi.canonicalFilePath();
      bool b = fi.isSymLink();
      s += (QString("{") + (b ? "1" : "0")) + "}";
      if (b)
         s += "[" + fi.symLinkTarget() + "]";

      dbg << s;
      s.clear();
   }

   return dbg;
}
/**************************************************************************//**
 * DbgEntryInfoList(): Output a QFileInfo list for detailed debug analysis.
 ******************************************************************************/
QStringList Find::dbgEntryInfoList  (const QFileInfoList &fil) const {
   ENTEX("Find::dbgEntryInfoList");

   QStringList dbg;
   for (int i = 0; i < fil.size(); i++) {
      if (fil[i].isDir() && fil[i].isFile())
         qd(3) << "item " << fil[i].canonicalFilePath() << " is a file AND a dir";

      QString s = fil[i].canonicalFilePath();
      bool b = fil[i].isSymLink();
      s += (QString("{") + (b ? "1" : "0")) + "}";
      if (b)
         s += "[" + fil[i].symLinkTarget() + "]";

      dbg << s;
      s.clear();
   }

   return dbg;
}
/**************************************************************************//**
 * CountDirs(): count number of dirs in dir.
 ******************************************************************************/
void Find::countDirs  (const QDir &dir) {
   ENTEX("Find::countDirs");

   QStringList dirs = getEntryList(dir);

   m_dirs += dirs.size();
}
/**************************************************************************//**
 * CountDirsRecursively(): recursively count number of dirs in dir.
 ******************************************************************************/
void Find::countDirsRecursively  (const QDir &dir) {
   ENTEX("Find::countDirsRecursively");

   if (m_stopDirCnt || !tab->getFindState())
      return;

   QStringList dirs = getEntryList(dir);

   int sz = dirs.size();

   for (int i = 0; i < sz; i++)
      countDirsRecursively(QDir(dir.absoluteFilePath(dirs[i])));

   m_dirs += sz;
}
/**************************************************************************//**
 * FilterBinary(): filter binary files from list of files.
 ******************************************************************************/
QFileInfoList Find::filterBinary  (const QFileInfoList &fil) {
   ENTEX("Find::filterBinary");

   unsigned char buf[256 * 1024];  ///< amount to determine encoding \todo-performance
   unsigned long *ubuf = 0;
   size_t ulen;
   QString code,
           code_mime,
           type;
   QFileInfoList subset;

   memset(buf, '\0', sizeof(buf));

   for (register int i = 0, sz = fil.size(); i < sz; ++i) {

      if (!fil[i].exists())  // file got deleted
         continue;

      // Filter based on extension.
      if (sui->extensionRadioButton->isChecked()) {
         if (BINARY_EXTS.contains(fil[i].suffix(), Qt::CaseInsensitive))
            continue;
         if (TEXT_EXTS.contains(fil[i].suffix(), Qt::CaseInsensitive)) {
            subset << fil[i];
            continue;
         }
      }

      QFile file(fil[i].canonicalFilePath());

      // Filter based on content.
      if (!file.open(QIODevice::ReadOnly)) {
         QString s(tr("Could not open for reading: %1").arg(fil[i].absoluteFilePath()));
         qd(0) << s;
         emit boxError(s);
         continue;
      }

      qint64 nbytes = file.read(reinterpret_cast<char *>(buf), sizeof(buf));
      if (nbytes == 0) {
         qd(4) << "empty file-" << fil[i].absoluteFilePath();
         if (p->content.isEmpty())
            subset << fil[i];  // go ahead and put empty files in the list unless searching content
         continue;
      }
      if (nbytes == -1) {
         QString s(tr("Could not read from %1").arg(fil[i].absoluteFilePath()));
         qd(0) << s;
         emit boxError(s);
         continue;
      }

      int rc;
      if (!(rc = ::file_encoding(buf, nbytes, &ubuf, &ulen, &code, &code_mime, &type))) {
         qd(4) << "binary file-" << fil[i].absoluteFilePath();
      }
      else if (rc == 1) {
         qd(4) << "text file-" << fil[i].absoluteFilePath() << " code-" << code << " mime-" << code_mime;
         subset << fil[i];
      }
      else {
         emit boxError(tr("Out of memory. Fatal allocation error..."));
         QTimer::singleShot(4000, QCoreApplication::instance(), SLOT(quit()));
      }

      if (ubuf)
         free(ubuf);

   }  // for()

   return subset;

}  // filterBinary()
/**************************************************************************//**
 * FilterDateTime(): filter file list based on date/time constraints.
 * \todo-assuming, for example, that tr("Accessed") is in fact a substring of,
 * say, tr("Accessed between...") but this may not happen with tr(), so change
 * to use comparison between entire strings.
 ******************************************************************************/
QFileInfoList Find::filterDateTime  (const QFileInfoList &fil) const {
	QDateTime dt,
				 dt1 = p->dateTime,
				 dt2 = p->dateTime2;
	QString option = p->dateTimeTxt;
   QFileInfoList subset;

   bool accessed = option.indexOf(tr("Accessed")) != -1,
        created  = option.indexOf(tr("Created"))  != -1,
        after    = option.indexOf(tr("After"))    != -1,
        before   = option.indexOf(tr("Before"))   != -1;

   for (register int i = 0, sz = fil.size(); i < sz; ++i) {

      if (!fil[i].exists())  // file got deleted
         continue;

      dt = accessed ? fil[i].lastRead() : created ? fil[i].created() :fil[i].lastModified();

		if (after) {
			if (dt >= dt1)
            subset << fil[i];
		}
		else if (before) {
			if (dt <= dt1)
            subset << fil[i];
		}
		else {
			if (dt >= dt1  &&  dt <= dt2)  // between
            subset << fil[i];
		}
	}
   return subset;
}
/**************************************************************************//**
 * FilterNonDirs(): filter non-directories from directory list.
 ******************************************************************************/
QStringList Find::filterNonDirs  (const QStringList &_dirs, const QDir &_currentDir) const {
   ENTEX("Find::filterNonDirs");
   QStringList subset;
   for (register int i = 0, sz = _dirs.size(); i < sz; ++i) {
      QString abspath = _currentDir.absoluteFilePath(_dirs[i]);
      QFileInfo fi(abspath);
      if (!fi.exists() || !fi.isDir())
         continue;
      subset << _dirs[i];
   }
   return subset;
}
/**************************************************************************//**
 * FilterNonFiles(): filter non-files from file list.
 ******************************************************************************/
QFileInfoList Find::filterNonFiles  (const QFileInfoList &fil) const {
   ENTEX("Find::filterNonFiles");
   QFileInfoList subset;
   for (register int i = 0, sz = fil.size(); i < sz; ++i) {
      if (!fil[i].exists() || !fil[i].isFile())
         continue;
      subset << fil[i];
   }
   return subset;
}
/**************************************************************************//**
 * FilterSize(): filter file list based on size constraints.
 * \todo-same as todo for filterDateTime() using tr("Larger") and
 * tr("Larger than...").
 ******************************************************************************/
QFileInfoList Find::filterSize  (const QFileInfoList &fil) const {
   ENTEX("Find::filterSize");
   QFileInfoList subset;
	QString option = p->sizeTxt;
	bool between = option.indexOf("Between") != -1,
		  larger  = option.indexOf("Larger")  != -1,
		  smaller = option.indexOf("Smaller") != -1;
   qint64 sz,
   sz1 = units2bytes(QString("%1 %2").arg(p->size).arg(chop(p->sizeUnits, 4))), // chop 'ytes'
   sz2 = between ?
         units2bytes(QString("%1 %2").arg(p->size2).arg(chop(p->sizeUnits2, 4))) : -1;

   for (register int i = 0, size = fil.size(); i < size; ++i) {

      if (!fil[i].exists())  // file got deleted
         continue;

      sz = fil[i].size();

		if (larger) {
			if (sz >= sz1)
            subset << fil[i];
		}
		else if (smaller) {
			if (sz <= sz1)
            subset << fil[i];
		}
		else {
			if (sz >= sz1  &&  sz <= sz2)  // between
            subset << fil[i];
		}
	}
   return subset;
}
/**************************************************************************//**
 * FilterWinOddities(): filter file list based on Windows oddities:
 *
 * a) Qt reports regular folders with name ending in ".lnk" as files.
 *       action - remove ALL objects ending w/ ".lnk" w/ empty or dead target
 * b) Qt reports folders beginning with '.' and having 'H' attribute as files.
 *       action - remove these objects
 *
 * Note: QFileInfo::symLinkTarget() may SIGSEGV on shortcut with nonsensical or
 *       empty target name (as viewed in Windows Explorer Properties).
 *
 * Platform: Windows only.
 ******************************************************************************/
QFileInfoList Find::filterWinOddities  (const QFileInfoList &fil) const {
   ENTEX2("Find::filterWinOddities", 3);

   QFileInfoList subset;
   for (register int i = 0, sz = fil.size(); i < sz; ++i) {

      if (fil[i].isSymLink())  {
         QString target = fil[i].symLinkTarget();
         QFileInfo fi(target);
         if (target.isEmpty() || !fi.exists())  // see (a) above
            continue;
      }

      QFile file(fil[i].absoluteFilePath());
      if ((!fil[i].fileName().isEmpty() && fil[i].fileName()[0] == '.') &&
            fil[i].isHidden()                                               &&
           !file.open(QIODevice::ReadOnly))  // make sure it's a folder (see (b) above)
         continue;

      subset << fil[i];
   }
   return subset;
}
/**************************************************************************//**
 * FilterShortcuts(): filter Windows shortcuts from directory list.
 ******************************************************************************/
QStringList Find::filterShortcuts  (const QStringList &_dirs) const {
   ENTEX("Find::filterShortcuts");

   QStringList subset;
   for (register int i = 0, size = _dirs.size(); i < size; ++i) {
      int idx = _dirs[i].lastIndexOf('.');
      if (idx != -1  &&  _dirs[i].mid(idx) == ".lnk") {
         qd(4) << "skipping shortcut " << _dirs[i];
         continue;
      }
      subset << _dirs[i];
   }
   return subset;
}
/**************************************************************************//**
 * find(): find and display one directory at a time recursively.
 * \todo - separate flag for inverting only on datetime restrictions?
 * \todo - better test inversion
 *    Local Statics
 *    -------------
 *    Whether it is Qt or C++ or the fact that this function is recursive,
 *    they stick around until the program ends, such that even if destroyed
 *    and re-instantiated any local statics persist.
 *
 * Some arcane notes about link-type entities and shortcuts using
 * QDir::entryList((QDir::Files|QDir::AllDirs) | QDir::NoDotAndDotDot | QDir::System):
 *    Windows Shortcuts
 *    -----------------
 *    a) if by some chance a foldername ends with ".lnk" it is both reported as
 *       a file that cannot be opened and a folder containing "gibberish" (see
 *       below).  It is also considered a symbolic link without a target.  Sim-
 *       ilarly, any non-shortcut file renamed to have a .lnk extension is al-
 *       ways considered a symbolic link without a target and is always rep-
 *       orted as a file (greppable) and a directory (containing gibberish).
 *    b) shortcuts that have been renamed so that the ".lnk" extension no
 *       longer exists are treated as binary files.
 *    c) if QDir::NoSymLinks specified valid folder shortcuts are ignored and
 *       invalid folder shortcuts contain gibberish, producing file open errors
 *       for each "file"; otherwise, valid folder shortcuts are inserted into a
 *       filepath which cannot be opened and invalid folder shortcuts, again,
 *       are reported to contain gibberish.  Broken folder shortcuts are always
 *       returned as both files and dirs (containing gibberish) but valid folder
 *       shortcuts are never returned as files yet are returned as directories
 *       unless QDir::NoSymLinks is specified.
 *    d) if QDir::NoSymLinks specified then valid shortcuts to files are
 *       considered binary files but otherwise ignored and broken shortcuts to
 *       files are considered binary files and also directories containing
 *       gibberish; otherwise, the .lnk files are considered binary files such
 *       that valid shortcuts to files are not considered directories, so the
 *       associated targets are irrelevant, whereas invalid shortcuts to files
 *       are considered directories containing gibberish flagging corresponding
 *       file open errors.
 *
 *    gibberish = Windows reports the files and folders at the top level of the
 *                root directory to be the files "contained" by broken short-
 *                cuts and other oddities.
 *
 * Conclusion:  by default, allow .lnk files as files which may possibly be
 *              grepped (except for valid folder shortcuts which are never
 *              reported as files by Qt and, if modeShortcut() is true, valid
 *              file shortcuts being replaced by valid targets); disallow .lnk
 *              files as directories because they are too ill-begotten (unless
 *              modeShortcut() is true where valid folder shortcuts are re-
 *              placed by valid targets). [A regular folder, whether empty or
 *              not, that ends in ".lnk" is considered by Qt to be a broken
 *              shortcut and will not be traversed; these oddities are also
 *              reported as files and will trip an error dialog as they cannot
 *              be opened as files -- this is more efficient than checking
 *              every file to see if it is a shortcut without a target.]
 ******************************************************************************/
void Find::find  (const QString &_path) {
   ENTEX("Find::find");

   if (!tab->getFindState())
		return;

   QFileInfo fi(_path);  // expensive but cannot let a non-dir get through
   if(!fi.exists() || !fi.isDir())
      return;

   QDir currentDir(_path);

	qd(3) << "current dir-" << currentDir.absolutePath();

   int cnt = currentDir.count() - 2;  // not . or ..
   m_numFiles += cnt;
   QString s;
   if (cnt > 256)
      s = tr("  [%1 files]").arg(cnt);
   emit statusShow(_path + s);

	(void)_path;

	// get list of files in this dir with wildcard glob
   QFileInfoList files = currentDir.entryInfoList(p->filelist, QDir::Files | flags(), QDir::Reversed);
   qd(3) << "files- " << dbgEntryInfoList(files);

#ifdef Q_OS_WIN32
   files = filterWinOddities(files);
   if (p->modeShortcut())  // todo-add support (if needed) for win7 handling various links
      files = resolveShortcuts(files);
#endif
   files = filterNonFiles(files);  // I don't trust entryInfoList()

   if (sui->binaryCheckBox->isChecked())
      files = filterBinary(files);

	if (p->modeDateTime())
      files = filterDateTime(files);

	if (p->modeSize())
      files = filterSize(files);

   if (tab->getSettings()->skipFilterSz())
      files = tab->getSettings()->filterSkip(files);

   if (p->modeInvert())
      files = complement(files, currentDir);

   if (files.size()  &&  m_depth >= tab->getSettings()->getDepthMin()) {
		if (!p->content.isEmpty()) {
         files = matchOnce(files);  // filter file list based on regexp
		}
      if (files.size())
         emit showFiles(files);  // send matched files in this dir to tab widget
	}

	// get list of dirs in this dir to process recursively
   QStringList dirs = getEntryList(currentDir);

   qd(4) << "dirs-" << dbgEntryList(dirs, currentDir);

   int depthChanged = dirs.size();

   if (depthChanged)
      ++m_depth;

   showProgress(depthChanged);

   if (m_depth > tab->getSettings()->getDepthMax()) {
		--m_depth;
      return;
	}

   if (p->searchSubs)
      for (int i = 0, sz = dirs.size(); i < sz; ++i)
			find(currentDir.absoluteFilePath(dirs[i]));  // recurse

	if (depthChanged)
		--m_depth;

}  // find()
/**************************************************************************//**
 * FindInit(): initialize a find operation.
 ******************************************************************************/
int Find::findInit  () {
   ENTEX("Find::findInit");

   if (p->m_checkBoxes & CB_ACCURATE) {

      if (p->searchSubs) {
         emit statusShow(tr("Counting folders for accurate progress bar, Ctrl-S to skip..."));
         countDirsRecursively(QDir(p->path));
         emit statusShow(tr(""));
      }
      else
         countDirs(QDir(p->path));

      qd(3) << "found " << m_dirs << " dirs";

      if (m_stopDirCnt) {
         m_stopDirCnt = false;
         p->m_checkBoxes &= ~CB_ACCURATE;  // revert to inaccurate search
         emit findProgressBar_setRange(0, FIND_PROGRESS_RANGE_INT);
      }
      else
         emit findProgressBar_setRange(0, m_dirs);
   }
   else
      emit findProgressBar_setRange(0, FIND_PROGRESS_RANGE_INT);

   emit findProgressBar_setFormat("%p %");

   qd(0) << "calling find() with path-" << p->path;
   find(p->path);

   return m_numFiles;
}
/**************************************************************************//**
 * Flags(): set flags for getting file/dir entry list.
 ******************************************************************************/
QFlags<QDir::Filter> Find::flags  () const {
   ENTEX("Find::flags");

	QFlags<QDir::Filter> flags = QDir::NoDotAndDotDot | QDir::System;

	if (p->caseSensitive())
		flags |= QDir::CaseSensitive;

#ifdef Q_OS_WIN32
   if (!p->modeShortcut())  // if (don't follow shortcuts)...
      flags |= QDir::NoSymLinks;
#else
   if (!p->modeSymLink())  // if (don't follow)...
      flags |= QDir::NoSymLinks;
#endif

   if (!p->modeSkipHidden())
		flags |= QDir::Hidden;

	return flags;
}
/**************************************************************************//**
 * GetEntryList(): get dirs from dir.
 *
 * Notes         : QDir::entryList() brings up all kinds of Windows junk that
                   must be filtered.  See find().
 ******************************************************************************/
QStringList Find::getEntryList  (const QDir &dir) const {
   ENTEX("Find::getEntryList");

   QStringList dirs = dir.entryList(QDir::AllDirs | flags(), QDir::Reversed);

   dirs = filterNonDirs(dirs, dir); // expensive but the System flag returns broken links

#ifdef Q_OS_WIN32
   if (p->modeShortcut())  // todo-add support for win7 handling various links
      dirs = resolveShortcuts(dirs, dir);  // conversion (no inversion)
   else
      dirs = filterShortcuts(dirs);
#endif

   if (tab->getSettings()->getFolders()->dirFilterSz())
      dirs = tab->getSettings()->getFolders()->filterFolders(dirs);

   return dirs;
}
/**************************************************************************//**
 * MatchOnce(): filter via regexp - ensure at least one content match in file.
 *
 * \todo-hangs in winxp on a *.pst file when MS Outlook is running -- fix this
 * \todo-should be able to use qtconcurrent::mappedReduced() here
 ******************************************************************************/
QFileInfoList Find::matchOnce  (const QFileInfoList &fil) {
   ENTEX("Find::matchOnce");

   QFileInfoList foundFiles;

   for (register int i = 0, sz = fil.size(); i < sz; ++i) {

      if (!tab->getFindState())
			break;

      QFile file(fil[i].canonicalFilePath());

		if (file.open(QIODevice::ReadOnly)) {
			QTextStream in(&file);
			while (!in.atEnd()) {
            if (!tab->getFindState())
					break;

            if (p->m_rx.indexIn(in.readLine()) != -1) {
               foundFiles << fil[i];
					break;
				}
			}
		}
      else emit boxError(QString(tr("Could not open '%1'.")).arg(fil[i].canonicalFilePath()));

      file.close();
   }

   return foundFiles;

}  // matchOnce()
/**************************************************************************//**
 * ResolveShortcuts(): convert shortcuts in the list to actual targets; remove
 *                     shortcuts with empty or non-existent targets.
 *
 * Platform: Windows only.
 ******************************************************************************/
QFileInfoList Find::resolveShortcuts  (const QFileInfoList &fil) const {
   ENTEX("Find::resolveShortcuts");
   QFileInfoList subset;

   for (register int i = 0, sz = fil.size(); i < sz; ++i) {

      if (fil[i].isSymLink()) {
         QString target  = fil[i].symLinkTarget();

         if (target.isEmpty())
            continue;

         QFileInfo fi(target);
         if (!fi.exists()) {
            qd(3) << "tossing broken shortcut to-" << target;
            continue;
         }

         subset << fi;  // replace valid shortcut with target
      }
      else subset << fil[i];
   }
   return subset;
}
/**************************************************************************//**
 * ResolveShortcuts(): convert shortcuts in the list to actual targets; remove
 *                     shortcuts with empty or non-existent targets.
 *
 * Platform: Windows only.
 ******************************************************************************/
QStringList Find::resolveShortcuts  (const QStringList &_files,
                                     const QDir &_currentDir) const {
   ENTEX("Find::resolveShortcuts");
   QStringList subset;

   for (register int i = 0, sz = _files.size(); i < sz; ++i) {
      QFileInfo fi(_currentDir.absoluteFilePath(_files[i]));

      if (fi.isSymLink()) {
         QString target  = fi.symLinkTarget();

         if (target.isEmpty())
            continue;

         QFileInfo fi(target);
         if (!fi.exists()) {
            qd(3) << "tossing broken shortcut to-" << target;
            continue;
         }

         subset << target;
      }
      else subset << _files[i];
   }
   return subset;
}
/**************************************************************************//**
 * ShowProgress(): show progress without sending main thread too many signals.
 ******************************************************************************/
void Find::showProgress  (int _depthChanged) {
   ENTEX2("Find::showProgress", 4);

   static const int chunk = m_dirs / 10 == 0 ? 1 : m_dirs / 10;
   static const int frequency = 16;  // increase for more responsive gui, less accurate progress bar

   ++m_dirCount;

   if (p->m_checkBoxes & CB_ACCURATE) {
      if (m_dirCount % chunk) {  // 10 progress bar chunks
         if (m_timer.elapsed() > 2000) {
            showProgressBar();
            emit findProgressBar_setValue(m_dirCount);
         }
      }
   }
   else {
      if (_depthChanged) {
         qd(4) << "setting depth key: " << m_depth << " to: " << _depthChanged;
         m_dirsAtDepth[m_depth] = _depthChanged;
      }
      else {  // empty leaf level dir
         //
         // The top level dir has a weight of 1 at level 0.  The sum of the
         // weights of all leaf level dirs also equals 1.  So, we only tick
         // the progress bar for every empty dir.  The weight of a dir at
         // leaf level n is the reciprocal of a product, this product being
         // the number of dirs in each dir from level n up to level 0.  The
         // range is an estimate.
         //
         qreal product = 1.00;
         for (int i = 1; i <= m_depth; ++i)
            product *= m_dirsAtDepth[i];
         qd(4) << "calculated product: " << product;
         int x = qRound(FIND_PROGRESS_RANGE / product);
         qd(4) << "calculated x: " << x << " (" << (qreal)(FIND_PROGRESS_RANGE/product) << ")";

         if (x)
            m_progress += x;

         // 48 is roughly (number of dirs on system / screen resolution) * 8 for progress in 8-pixel chunks
         // qd(0) << "m_dirCount:" << m_dirCount << "m_progress:" << m_progress;
         if (m_dirCount % frequency == 0  &&  m_progress > 48) {
            if (m_timer.elapsed() > 2000) {
               showProgressBar();
               emit findProgressBar_addValue(m_progress);
               m_progress = 0;
            }
         }
      }
   }
}  // showProgress()
/**************************************************************************//**
 * ShowProgressBar(): show the progress bar.
 ******************************************************************************/
void Find::showProgressBar  () {
   ENTEX("Find::showProgressBar");
   if (!tab->findProgressVisible()) {
      qd(0) << "emitting findProgressShow(true)";
      emit findProgressShow(true);
   }
}
/**************************************************************************//**
 * StopPreSearchDirCount(): stop accurate progress bar pre dir count.  [slot]
 ******************************************************************************/
void Find::stopPreSearchDirCnt() {
   ENTEX("Find::stopPreSearchDirCnt");
   m_stopDirCnt = true;
   // we're done with this now
   disconnect(tab->getMw(), SIGNAL(stopPreSearchDirCnt()), this, SLOT(stopPreSearchDirCnt()));
}
