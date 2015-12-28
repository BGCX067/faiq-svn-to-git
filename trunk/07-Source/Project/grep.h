/**************************************************************************//**
 * NAME:    grep.h
 *
 * PURPOSE: Definitions for class Grep.
 *
 * DATE:    July 11, 2010
*******************************************************************************/
#ifndef GREP_H
#define GREP_H

#include <QDir>
#include <QFlags>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include "types.h"

const qint64 GREP_MIN_PROGRESS = 65536LL;  // 64 KB

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

//template <class T> class AtomicQueue;
class Parms;
class Settings;
class Tab;

namespace Ui {
   class SettingsGlobal;
}

class Grep : public QObject {
Q_OBJECT
public:
   Grep(const QString &path, Tab *tab);
   virtual ~Grep();
   void cancel() { m_cancel = true; }
   inline int getBytes() const { return m_bytes; }
	void grep();
   void grepInit();
   void statusShow(int, int, int, qint64);
   QString truncateLine(const QRegExp &rx, QString line, int max);
private:
   bool m_cancel;
   int m_bytes;
	Tab *tab;
   QRegExp m_rx;
	QString m_path;
signals:
	void boxError(const QString &, const QMessageBox::Icon = QMessageBox::Warning, Error err = ERR_INVALID);
	void grepProgressBar_setFormat(QString);
	void grepProgressBar_setRange(int, int);
	void grepProgressBar_setValue(int);
	void grepProgressDialog_setValue(int);
	void grepProgressDialog_setLabelText(QString);
   void grepResults(QList<Line>);
   void statusShow(const QString &, Operation op = GREP);
};

#endif
