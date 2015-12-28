/**************************************************************************//**
 * NAME:       util.h
 *
 * PURPOSE:    Header for util.cpp plus shared Searchmonkey constants.
 *
 * DATE:       September 6, 2009
*******************************************************************************/
#ifndef UTIL_H
#define UTIL_H

#include "debug.h"

#define QT_USE_FAST_CONCATENATION
#define QT_USE_FAST_OPERATOR_PLUS

#include <limits.h>
#include <string>

#include <QChar>
#include <QFileInfoList>
#include <QMessageBox>
#include <QRegExp>
#include <QString>
#include <QVector>

#include "types.h"

using namespace std;
using std::string;

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QComboBox;
class QDir;
class QFile;
class QGroupBox;
class QLabel;
class QListWidget;
class QObject;
class QStringList;
QT_END_NAMESPACE

const bool PROMPT = true;
const bool NOPROMPT = false;

const int GREP_BEGIN = -1,  /// beginning of file grep
          GREP_EOF   = -2;  /// end of file grep

const qint16 MAX_DIRECTORY_DEPTH = 240;  // mapped or shared drive (dflt 255)
const qint16 MAX_SEARCH_TABS = 128;      // ""

const quint32 GEN_TAB    = 1U << 0;  // General tab of Settings GUI
const quint32 GEN_IGNORE = 1U << 1;  // ...
const quint32 GEN_CSTYLE = 1U << 2;  // ...
const quint32 GEN_MASK = UINT_MAX;

const quint64 KB = Q_UINT64_C(1024);               // 1024
const quint64 MB = Q_UINT64_C(1048576);            // 1024x1024
const quint64 GB = Q_UINT64_C(1073741824);         // 1024x1024x1024
const quint64 TB = Q_UINT64_C(1099511627776);      // 1024x1024x1024x1024
const quint64 PB = Q_UINT64_C(1125899906842624);   // 1024x1024x1024x1024x1024

const string SM_CR = string (1, (char) 13);
const string SM_LF = string (1, (char) 10);
const string SM_SP = string (1, (char) 32);

const Qt::GlobalColor COLOR_TEXT = Qt::black;
const Qt::GlobalColor COLOR2_HIGHLIGHT = Qt::red;
const QColor COLOR_HIGHLIGHT = QColor(220,0,3);    // darker, less annoying red
const QColor COLOR_HYPERLINK = QColor(6,105,205);  // RGB hyperlink blue

const QString CONFIG_FILE = "./.smrc";

const QString SM_ICON = ":/pixmaps/searchmonkey-300x300.png";

extern
int file_encoding(const unsigned char *buf, size_t nbytes, unsigned long **ubuf,
                  size_t *ulen, QString *code, QString *code_mime,
                  QString *type);

bool boxConfirm(const QString &s, const QString &title = "", QAbstractButton *again = nullptr,
					 const QMessageBox::Icon = QMessageBox::Warning);
void boxError(const QString &s, const QMessageBox::Icon = QMessageBox::Warning);
QString browse(QWidget *_this, Browse b = SEARCH);
QString bytes2units(qint64);
QChar charS(bool cond);
QString chop(QString s, int n);
int countHits(const QRegExp &rx, const QString &text);
bool fileRemove(QString path, bool prompt = true, QString *s = nullptr);
QStringList filepath2abspath(const QFileInfoList &fil);
QStringList filepath2abspath(const QStringList &files, const QDir &dir);
QString getLine(QFile &_file);
QRegExp::PatternSyntax int2syntax(int _index);
int intMax(qint64);
void list2ListWidget(const QStringList &sl, QListWidget *lw);
QStringList listIntersect(const QStringList &list1, const QStringList &list2);
QFileInfoList listSubtract(const QFileInfoList &list1, const QFileInfoList &list2);
QStringList listSubtract(const QStringList &list1, const QStringList &list2);
void listWidget2List(QListWidget *lw, QStringList *sl);
QString makePrintable(const QString &_s);
QString ms2units(quint64);
void* parseDelim(const char *str, const char *delims, char ***flds, int *n_found);
qint64 positionAfterLine(QFile &, const QString &);
QStringList starDotAdd(const QStringList &sl);
QStringList starDotRemove(const QStringList &sl);
char *strip(char *pz, const char *g, int *n = nullptr);
string *strip(string* ps, const string& g, int* n = nullptr);
qint64 units2bytes(const QString &);
void updateComboBox(QComboBox *comboBox);
bool validDir(const QString& dir);
bool validRx(const QRegExp &rx);
bool validRx(QRegExp *rx);
bool validRx(const QString &s, QRegExp::PatternSyntax syntax);
bool validRx(const QStringList &sl, QRegExp::PatternSyntax syntax);

/**************************************************************************//**
 * GetAvailIdx(): get next avail (null) ptr from vector.
 *
 * Returns: index of next available pointer (may realloc)
 ******************************************************************************/
template <typename T>
int getAvailIdx  (QVector<T *> &_v, int _realloc = 32) {
   ENTEX("::getAvailIdx");

   int idx = 0,
       size = _v.size();
   for (; idx < size; ++idx)
      if (_v[idx] == nullptr)
         break;

   if (idx == size) {
		_v.resize(size + _realloc);
		qd(2) << "resize() occurred from " << size << " to " << _v.size();
		return getAvailIdx(_v, _realloc);  // realloc invalidates all indecies so start over
	}
   // idx now points at the first available index
	return idx;
}
#endif
