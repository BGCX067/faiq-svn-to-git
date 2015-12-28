/**************************************************************************//**
 * NAME:      util.cpp
 *
 * PURPOSE:   Searchmonkey reusable and miscellaneous functions.
 *
 * DATE:      September 6, 2009
*******************************************************************************/
#include "debug.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QMessageBox>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "util.h"
/**************************************************************************//**
 * BoxConfirm(): Popup confirmation with option to not show again.
 *
 * Returns     : true if user presses 'OK ', false if user presses 'Cancel'
 ******************************************************************************/
bool boxConfirm  (const QString &_s, const QString &_title, QAbstractButton *again,
                  const QMessageBox::Icon _icon) {
   QMessageBox msgBox;

   if (again != nullptr)
      msgBox.addButton(again, QMessageBox::ApplyRole);

   msgBox.setWindowModality(Qt::NonModal);
	msgBox.setText(_s);
	msgBox.setIcon(_icon);
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
   msgBox.setWindowIcon(QIcon(SM_ICON));
   if (!_title.isEmpty())
      msgBox.setWindowTitle(_title);

   //
   // This is kind of a hack. Probably best to add checkbox to a QDialog.
   //
   bool ret = true;
   forever {
      switch(msgBox.exec()) {
         case QMessageBox::Ok:
            break;
         case QMessageBox::Cancel:
            ret = false;
            break;
         default:  // checkbox clicked
            continue;
            // Q_ASSERT(0);  // unlikely
      }
      break;
   }

   if (again != nullptr)
      msgBox.removeButton(again);

   return ret;
}
/**************************************************************************//**
 * BoxError(): Pop up an error message box.  (Default - warning icon.)
 ******************************************************************************/
void boxError  (const QString &_s, const QMessageBox::Icon _icon) {
   ENTEX("::boxError");
	QMessageBox msgBox;
	msgBox.setText(_s);
	msgBox.setIcon(_icon);
   msgBox.setWindowIcon(QIcon(SM_ICON));
   qd(3) << "calling exec() on message box...";
	msgBox.exec();
}
QString tr  (const char *s) {
   return QObject::tr(s);
}
/**************************************************************************//**
 * Browse(): Advanced browse for target directory.
 *
 * Parms   : _this - parent
 *           b - keep mru dir for different browse operations
 *
 * Returns : user-selected directory, otherwise empty string
 ******************************************************************************/
QString browse  (QWidget *_this, Browse b) {
   ENTEX("::browse");

   QString mruSetting = tr("MainWindow/mruDir") + (b == SEARCH  ? "" :
                                                            b == CONTEXT ? tr("1")
                                                                         : tr("2"));
   QString parentSetting = tr("MainWindow/mruParent") + (b == SEARCH ? "" :
                                                                  b == CONTEXT ? tr("1")
                                                                               : tr("2"));
   QList<QUrl> urls;
   urls << QUrl::fromLocalFile(QDesktopServices::storageLocation(
                               QDesktopServices::DesktopLocation))
        << QUrl::fromLocalFile(QDesktopServices::storageLocation(
                               QDesktopServices::DocumentsLocation))
        << QUrl::fromLocalFile(QDesktopServices::storageLocation(
                               QDesktopServices::HomeLocation))
        << QUrl::fromLocalFile(QDir::rootPath())
        << QUrl::fromLocalFile(QDir::currentPath());

   QSettings settings;
   QString mru = settings.value(mruSetting, "").toString();
   QString parent = settings.value(parentSetting, "").toString();

   QFileDialog dialog(_this, QObject::tr("Select Directory"), !parent.isEmpty()
      ? parent
      : QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
   dialog.setSidebarUrls(urls);
   dialog.setFileMode(QFileDialog::Directory);
   dialog.setViewMode(QFileDialog::Detail);
   dialog.setOptions(QFileDialog::ShowDirsOnly);
   qd(3) << "selecting file-" << mru;
   dialog.selectFile(mru);

   QStringList fileNames;
   if (dialog.exec())
      fileNames = dialog.selectedFiles();

   QString directory;
   if (!fileNames.isEmpty())
      directory = fileNames[0];
   if (!directory.isEmpty()) {
      QDir dir(directory);  // save parent dir so it lists dir we want
      QString dbg;
      settings.setValue(parentSetting, dbg = dir.cd("..") ? dir.absolutePath()
                                                                   : directory);
      qd(3) << "set parent-" << dbg;
      settings.setValue(mruSetting, directory);
   }
   return directory;
}
/**************************************************************************//**
 * bytes2units(): Convert bytes to B, KB, MB, GB, ...
 *
 * parms        : _size - bytes
 *
 * returns      : '0 B' if _size < 0; otherwise, nice string like '724 KB'
 ******************************************************************************/
QString bytes2units  (qint64 _size) {

	quint64 size = _size < 0 ? Q_UINT64_C(0) : static_cast<ull>(_size);
	(void)_size;

	char s[16];
	for (int i = 0; i < 16; s[i++]='\0') ;

	if (size < KB)
      sprintf (s, "%d B ", static_cast<int>(size));
	else if (size < MB) {
		sprintf (s, "%1.1lf KB ", static_cast<double>(size) / KB);
		//
		// enforce N KB not N.0 KB
		//
      /*string _s(s);
		string::size_type loc = _s.find(".0");
		if (loc != string::npos) {
			_s.erase(loc, 2);
			sprintf (s, "%s", _s.c_str());
      }*/
	}
	else if (size < GB)
		sprintf (s, "%1.1lf MB ", static_cast<double>(size) / MB);
	else if (size < TB)
		sprintf (s, "%1.1lf GB ", static_cast<double>(size) / GB);
	else if (size < PB)
		sprintf (s, "%1.1lf TB ", static_cast<double>(size) / TB);
	else
		sprintf (s, "%1.1lf PB ", static_cast<double>(size) / PB);

	return QString(s);
}
/**************************************************************************//**
 * charS() : make a string plural.
 *
 * Parms   : cond - condition for plurality.
 *
 * Returns : 's' if cond true, '\0' otherwise
 ******************************************************************************/
QChar charS  (bool cond) {
  return cond ? QChar('s') : QChar('\0');
}
/**************************************************************************//**
 * Chop(): wrapper with return val for: void QString::chop().
 *
 * Parms: s - string to be chopped
 *        n - chars to chop off end of s
 *
 * Returns: chopped QString
 ******************************************************************************/
QString chop  (QString s, int n) {
	s.chop(n);
	return s;
}
/**************************************************************************//**
 * CountHits(): Count occurrences of regex in chunk of text.
 *
 * Parms      : _rx   - regular expression
 *              _text - text which may contain pattern
 *
 * Returns    : number of occurrences
 ******************************************************************************/
int countHits  (const QRegExp &_rx, const QString &_text) {
   ENTEX("::countHits");
   register int hits = 0,
                index = _text.indexOf(_rx);
   while (index >= 0) {
      ++hits;
      int length = _rx.matchedLength();
      index = _text.indexOf(_rx, index + length);
   }
   qd(11) << "returning " << hits << " hits";
   return hits;
}
/**************************************************************************//**
 * FileRemove(): remove file.
 *
 * Parms       : s - optional ptr to hold error msg
 *
 * Returns     : true on success, false otherwise
 ******************************************************************************/
bool fileRemove  (QString path, bool prompt, QString *s) {
   ENTEX("::fileRemove");

   QFile file(path);

   if (!file.exists(path)) {
      ::boxError(QObject::tr("File %1 does not exist: cannot remove").arg(path));
      return false;
   }

   if (prompt && !::boxConfirm(QObject::tr("Delete file %1 from disk?").arg(path)))
      return false;

   if (!file.remove(path)) {
      ::boxError(QObject::tr("Could not remove file %1").arg(path));
      if (s != nullptr)
         *s = file.errorString();
      qd(0) << "QFile::remove() failed:" << file.errorString();
      return false;
   }

   return true;
}
/**************************************************************************//**
 * Filepath2abspath(): convert list of QFileInfo objects to absolute paths.
 *
 * Parms             : fil - list of QFileInfo objects
 *
 * Returns           : list of canonical filepaths
 ******************************************************************************/
QStringList filepath2abspath  (const QFileInfoList &fil) {
   QStringList files;
   for (register int i = 0, sz = fil.size(); i < sz; ++i)
      files << fil[i].canonicalFilePath();

   return files;
}
/**************************************************************************//**
 * Filepath2abspath(): convert list of filenames to absolute paths.
 *
 * Parms             : _files - list of filenames
 *                     _dir - a directory
 *
 * Returns           : list of filepaths i.e. filenames w/ prepended directory
 ******************************************************************************/
QStringList filepath2abspath  (const QStringList &_files, const QDir &_dir) {
	QStringList files;
	register int size = _files.size();
	for (register int i = 0; i < size; ++i)
		files << _dir.absoluteFilePath(_files[i]);

	return files;
}
/**************************************************************************//**
 * GetLine(): get line from searchmonkey config file.
 *
 * Parms    : _file - config file
 *
 * Returns  : line as QString
 ******************************************************************************/
QString getLine  (QFile &_file) {
	char buf[512];
	memset (buf, '\0', sizeof(buf));
	QString line;
	while (!_file.atEnd()) {
		_file.readLine(buf, sizeof(buf));
		line = QString(buf);
		line = line.trimmed();
		if (line.isEmpty()  ||  line[0] == '#')
			continue;
		else
			break;
	}
	return line;
}
/**************************************************************************//**
 * int2syntax(): convert uint to QRegExp::PatternSyntax without relying on the
 *               actual values of the enumerated type.  We are relying on the
 *               order of elements in the combobox (see the *.ui file):
 *               0 - RegExp
 *               1 - RegExp2
 *               2 - Wildcard
 *               3 - Fixed
 *
 * parms       : index - position in syntaxComboBox
 *
 * returns     : QRegExp::PatternSyntax
 ******************************************************************************/
QRegExp::PatternSyntax int2syntax  (int index) {
   switch (index) {
      case +0 : return QRegExp::RegExp;
      case +1 : return QRegExp::RegExp2;
      case +2 : return QRegExp::Wildcard;
      case +3 : return QRegExp::FixedString;
      case -1 : return QRegExp::RegExp;      // empty or no selection
      default : Q_ASSERT(0);
   }
   return QRegExp::RegExp;
}
/**************************************************************************//**
 * IntMax(): converter for quint64 to int using limits.h header.
 *
 * Parms   : i - qint64
 *
 * Returns : i as int if i < INT_MAX, otherwise INT_MAX
 *           i as int if i > INT_MIN, otherwise INT_MIN
 ******************************************************************************/
 int intMax  (qint64 i) {
   if (i > 0LL) {
      if (i < static_cast<qint64>(INT_MAX))
         return static_cast<int>(i);
      else
         return INT_MAX;
   }
   else if (i < 0LL) {
      if (i > static_cast<qint64>(INT_MIN))
         return static_cast<int>(i);
      else
         return INT_MIN;
   }
   else return 0;
 }
/**************************************************************************//**
 * List2ListWidget(): populate list widget from string list.
 ******************************************************************************/
void list2ListWidget  (const QStringList &sl, QListWidget *lw) {
	if (sl.isEmpty() || !lw)
		return;

	lw->clear();
	lw->addItems(sl);
}
/**************************************************************************//**
 * ListIntersect(): find the intersection of two QString lists.
 *
 * Parms          : list1
 *                  list2
 *
 * Returns        : intersection QString list
 ******************************************************************************/
QStringList listIntersect  (const QStringList &list1, const QStringList &list2){

   if (list1.empty() || list2.empty())
      return QStringList();

   if (list1 == list2)
      return list1;

   QStringList intersection;
   for (register int i = 0, sz = list1.size(); i < sz; ++i) {
      if (list2.contains(list1.at(i)))
         intersection << list1.at(i);
   }
   return intersection;
}
/**************************************************************************//**
 * ListSubtract(): subtract one QFileInfo list from another: (list1 - list2)
 *
 * Parms         : list1 - minuend
 *                 list2 - subtrahend
 *
 * Returns       : difference QFileInfo list
 ******************************************************************************/
QFileInfoList listSubtract  (const QFileInfoList &list1, const QFileInfoList &list2) {

   if (list2.empty())
      return QFileInfoList(list1);

   if (list1.empty()  ||  list1 == list2)
      return QFileInfoList();

   QFileInfoList difference;
   for (register int i = 0, sz = list1.size(); i < sz; ++i) {
      if (list2.indexOf(list1.at(i)) == -1)
         difference << list1.at(i);
   }
   return difference;
}
/**************************************************************************//**
 * ListSubtract(): subtract one QString list from another: (list1 - list2)
 *
 * Parms         : list1 - minuend
 *                 list2 - subtrahend
 *
 * Returns       : difference QString list
 ******************************************************************************/
QStringList listSubtract  (const QStringList &list1,const QStringList &list2) {

	if (list2.empty())
		return QStringList(list1);

   if (list1.empty()  ||  list1 == list2)
		return QStringList();

	QStringList difference;
   for (register int i = 0, sz = list1.size(); i < sz; ++i) {
      if (list2.indexOf(list1.at(i)) == -1)
         difference << list1.at(i);
	}
	return difference;
}
/**************************************************************************//**
 * ListWidget2List(): populate string list from list widget.
 ******************************************************************************/
void listWidget2List  (QListWidget *lw, QStringList *sl) {
	if (!lw || !sl)
		return;

	sl->clear();

	int count = lw->count();
	for (int i = 0; i < count; ++i) {
		QListWidgetItem * item = lw->item(i);
		QString data = (item->data(Qt::DisplayRole)).toString();
		*sl << data;
	}
}
/**************************************************************************//**
 * MakePrintable(): Reduce a raw QString to only printable QChars.
 *
 * Parms          : _s - raw QString
 *
 * Returns        : printable QString
 ******************************************************************************/
 QString makePrintable  (const QString &_s) {
   QString s;
   s.resize(_s.size());
   register int j = 0;
   for (register int i = 0, sz = _s.size(); i < sz; ++i) {
      if (_s.at(i).isPrint())
         s[j++] = _s.at(i);
   }
   s.resize(j);
   return s;
 }
/**************************************************************************//**
 * ms2units(): Convert milliseconds to common macro units w/ integer rounding.
 *             Form: 'n <units> [n <units>]'
 *
 * parms     : _ms - milliseconds (non-negative)
 *
 * returns   : handy string like '1 min 2 sec', '24 sec', or '123 ms'
 ******************************************************************************/
QString ms2units  (quint64 _ms) {

#define scul(x) static_cast<unsigned long>(x)

	const quint64 sec   = Q_UINT64_C(1000);          // ms in sec
	const quint64 min   = Q_UINT64_C(60000);         // ms in min
	const quint64 hour  = Q_UINT64_C(3600000);       // ms in hour
	const quint64 day   = Q_UINT64_C(86400000);      // ms in day
	const quint64 month = Q_UINT64_C(2592000000);    // ms in month
	const quint64 year  = Q_UINT64_C(31104000000);   // ms in year

	char s[32];
	for (int i = 0; i < 32; s[i++]='\0') ;

	if (_ms < sec)
      sprintf (s, "%lu ms",             scul(_ms));
	else if (_ms < min)
      sprintf (s, "%lu sec",            scul(_ms / sec));
	else if (_ms < hour)
      sprintf (s, "%lu min %lu sec",    scul(_ms / min),
                                        scul((_ms % min)   / sec));
	else if (_ms < day)
      sprintf (s, "%lu hour %lu min",   scul(_ms / hour),
                                        scul((_ms % hour)  / min));
	else if (_ms < month)
      sprintf (s, "%lu day %lu hour",   scul(_ms / day),
                                        scul((_ms % day)   / hour));
	else if (_ms < year)
      sprintf (s, "%lu month %lu day",  scul(_ms / month),
                                        scul((_ms % month) / day));
	else
      sprintf (s, "%lu year %lu month", scul(_ms / year),
                                        scul((_ms % year)  / month));
	return QString(s);
}
/***************************************************************************//**
* FUNCTION:	ParseDelim()
*
* PURPOSE:  Separates a string into fields by setting delimiters to null.
*
* INPUT:	   str     - ptr to string to be parsed
*			   delims  - ptr to string of possible delims
*
* OUTPUT:	flds    - buffer to hold parsed fields
*			   n_found - fields parsed (optional)
*
* COMMENTS:	Allocates memory to hold both the pointers to the parsed fields, as
*			   well as the input string itself.  This buffer is returned, and it is
*			   the calling application's responsibilty to free the memory.  The
*			   size of memory equals:
*
*				n_found * sizeof (char *) + strlen (str) + 1 - xmem
*
* RETURNS:	flds - buffer holding parsed fields
*******************************************************************************/
void* parseDelim  (const char* str, const char* delims, char*** flds,
						 int* n_found) {

	int	fields	= 1,			// fields parsed
			xmem	   = 0;			// accumulate extra mem from stripped chars

	/*------------------------------------------------------------------------*
		Inititialize pfix, optional number found parm; validate pointers; copy
		subject string into marching pointer; and save the start address.
	 *------------------------------------------------------------------------*/

	n_found ? *n_found = 0 : 0;

	if (!(delims && *delims) || !flds || !(str && *str))
		return (0);

	char *  s,							// marching ptr
		  *  start;						// sentinel ptr

	if (!(start = s = strdup (str))) {
		qDebug() << "strdup(" << str << ") failed";
		return (0);
	}

	/*------------------------------------------------------------------------*
		Remove trailing CR, LF and SP.
	 *------------------------------------------------------------------------*/

	int	tmp;

	strip (s, string (SM_CR + SM_LF + SM_SP).c_str(), &tmp);

	xmem += tmp;

	/*------------------------------------------------------------------------*
		Parse.  The position pointer, s, points to the next segment to process.
	 *------------------------------------------------------------------------*/

	bool	done = false;

	while (!done) {

		if ((s = strpbrk (s, delims))) {

			/*----------------------------------------------------------------*
				  Strip trailing blanks from each field.
			 *----------------------------------------------------------------*/

			char * p = s;

			if (p > start  &&  *(p - 1) == ' ') {

				while (p - 1 >= start  &&  *(p - 1) == ' ')
					p--;

				if (s - p) {

					xmem += s - p;

					s = (char *) memmove (p, s, strlen (s) + 1);
				}
			}

			*s++ = '\0';		// define field

			fields++;
		}

		else done = true;

	}	// while

	/*------------------------------------------------------------------------*
		Allocate the exact amount of memory needed.  The input string now has
		embedded nulls to delineate the fields.  Copy this string into the lower
		part of the allocated buffer.  Align the respective pointers upstream in
		the same buffer.  Return the entire buffer.
	 *------------------------------------------------------------------------*/

	int	space;

	tmp = strlen (str) + 1 - xmem;

	if ((*flds = (char **) malloc (space = fields * sizeof (char *) + tmp))) {

		memset (*flds, '\0', space);

		memcpy (s = (char *) *flds + fields * sizeof (char *), start, tmp);

		for (int i = 0; i < fields; i++) {

			(*flds)[i] = s;

			s = strchr (s, '\0') + 1;
		}

		n_found ? *n_found = fields : 0;
	}
	else {

		*flds = (char **) 0;
	}

	free (start);

	return ((void *) *flds);

}	// ParseDelim()
/***************************************************************************//**
* FUNCTION:     positionAtLine()
*
* PURPOSE:      Position file ptr to end of line equal to given string.
*
* INPUT:        _file - file
*               _s    - string
*
* RETURNS:      file ptr position
******************************************************************************/
qint64 positionAfterLine(QFile &_file, const QString &_string) {
      ENTEX("::positionAfterLine");

		char buf[512];
		bool found = false;

		while (!_file.atEnd()) {
			memset(buf, '\0', sizeof(buf));
			_file.readLine(buf, sizeof(buf));
			QString s(buf);
			s = s.trimmed();
         qd(11) << "s-" << s;
			if (s[0] == '#')
				continue;
			if ((found = s == _string))
				break;
		}
		if (!found)
			_file.write(("\n" + _string + "\n").toStdString().c_str());

		return _file.pos();
}
/**************************************************************************//**
 * StarDotAdd: prepend "*." to QStringList of non-wildcard extensions.
 ******************************************************************************/
QStringList starDotAdd  (const QStringList &_sl) {
   QStringList sl;
   for (int i = 0, sz = _sl.size(); i < sz; ++i)
      sl << ("*." + _sl.at(i));
   return sl;
}
/**************************************************************************//**
 * StarDotRemove: remove "*." from QStringList of wildcard extensions.
 ******************************************************************************/
QStringList starDotRemove  (const QStringList &_sl) {
   QStringList sl;
   for (int i = 0, sz = _sl.size(); i < sz; ++i)
      sl << QString(_sl.at(i)).remove("*.");
   return sl;
}
/***************************************************************************//**
* FUNCTION: strip()
*
* PURPOSE:  Strip trailing garbage from a string.
*
* INPUT:    ps - string address
*           g  - garbage
*           n  - length change (optional)
*
* OUTPUT:   ps - stripped
*
* RETURNS:  stripped string address
******************************************************************************/
string* strip  (string* ps, const string& g, int* n) {

	if (!ps || g.empty())
		return (ps);

	// while the last element in the string is garbage, erase it

	int     n_ = 0;
	size_t  i;

	while ((i = (*ps).size() - 1) > 0  &&

			  g.find_first_of (string (1, (*ps)[i])) != string::npos) {

		(*ps).erase (i);

		++n_;
	}

	n ? *n = n_ : n_;

	return (ps);

}
char * strip  (char * pz, const char * g, int * n) {

	if (!(pz || *pz) || !(g || *g))
		return (pz);

		  /*--------------------------------------------------------------------*
			 (1) pass constructed string ptr and garbage ref to string version
			 (2) dereference returned string ptr
			 (3) call resultant string's member c_str() conversion function
			 (4) copy returned buffer into original buffer (always exceeds
				  or equals size of stripped buffer)
			*--------------------------------------------------------------------*/

	string s_(pz);

	return (strcpy (pz, (*strip (&s_, string (g), n)).c_str()));

}  // strip()
/**************************************************************************//**
 * units2bytes(): Convert B, KB, MB, GB, TB, PB to bytes.
 *
 * parms        : _units : string - 'N <units>' eg. 123 KB
 *
 * returns      : -1 on error, otherwise bytes
 ******************************************************************************/
qint64 units2bytes  (const QString &_units) {
   ENTEX("::units2bytes");

	if (_units.isEmpty() || _units.isNull())
		return (-1);

	float mag;
	char buf[_units.size()];  // more than big enough

	sscanf(_units.toStdString().c_str(), "%f %s", &mag, buf);
	QString z = QString(QObject::tr("units2bytes(): mag- %1 units-'%2'")).arg(mag).arg(buf);
   qd(11) << z;

	(void)_units;

	if (mag < 0)
		return (-1);

	QString units(buf);

	if (units == "B")
		return static_cast<qint64>(mag);
	if (units == "KB")
		return static_cast<qint64>(mag * KB);
	if (units == "MB")
		return static_cast<qint64>(mag * MB);
	if (units == "GB")
		return static_cast<qint64>(mag * GB);
	if (units == "TB")
		return static_cast<qint64>(mag * TB);
	if (units == "PB")
		return static_cast<qint64>(mag * PB);

	return -1;
}
/**************************************************************************//**
 * UpdateComboBox(): update a combobox's history.
 ******************************************************************************/
void updateComboBox  (QComboBox *comboBox) {
   if (comboBox->findText(comboBox->currentText()) == -1)
      comboBox->addItem(comboBox->currentText());
}
/**************************************************************************//**
 * ValidDir(): Validate a Qstring as an existing dir.
 ******************************************************************************/
bool validDir  (const QString &dir) {
   QFileInfo fi(dir);
   return fi.isDir();
}
/**************************************************************************//**
 * ValidRx(): Validate a QRegExp as a valid regular expression.
 ******************************************************************************/
bool validRx  (const QRegExp &rx) {
   return rx.isValid();
}
/**************************************************************************//**
 * ValidRx(): Validate a QRegExp as a valid regular expression.
 ******************************************************************************/
bool validRx  (QRegExp *rx) {
   return rx->isValid();
}
/**************************************************************************//**
 * ValidRx(): Validate a Qstring as a valid regular expression.
 ******************************************************************************/
bool validRx  (const QString &s, QRegExp::PatternSyntax syntax) {
   QRegExp rx(s);
   rx.setPatternSyntax(syntax);
   return rx.isValid();
}
/**************************************************************************//**
 * ValidRx(): Validate a QstringList as a valid regular expression.
 ******************************************************************************/
bool validRx  (const QStringList &_sl, QRegExp::PatternSyntax _syntax) {
   QList<QString>::const_iterator i = _sl.begin();
   while (i != _sl.end()) {
      if (!validRx(*(i++), _syntax))
         return false;
   }
   return true;
}
