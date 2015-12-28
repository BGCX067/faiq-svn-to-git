/**************************************************************************//**
 * NAME:    mapped.cpp
 *
 * PURPOSE: Functor parameters for QtConcurrent::mappedReduced()
 *
 * DATE:    June 29, 2010
*******************************************************************************/
#if 0
#include <QString>
#include <QFile>
#include <QRegExp>
#include <QIODevice>
#include <QTextStream>
#include <QThread>

#include "mapped.h"
#include "tab.h"

QMutex Mapped::mutex;
/**************************************************************************//**
 * Mapped(): Create a Mapped object.
 ******************************************************************************/
Mapped::Mapped  (const Tab * const _tab, const QRegExp &_rx, const QString &_abspath)
:
m_tab(_tab),
m_rx(_rx),
m_abspath(_abspath)
{}
/**************************************************************************//**
 * CountHits(): Count lines in file that contain at least one pattern (m_text).
 ******************************************************************************/
WordCount Mapped::countHits  (const Mapped &_mapped) {
   ENTEX2("Mapped::countHits", 4);

	QString path = _mapped.getAbspath();
	QFile file(path);

   const QRegExp crx(_mapped.getRegExp());  // const performance tweak

	int m = 0;
	WordCount wc;
	wc[path] = 0;

	if (file.open(QIODevice::ReadOnly)) {
		QString line;
		QTextStream in(&file);
		while (!in.atEnd()) {
			if (_mapped.getTab()->getAbortMapped()) {
				qd(6) << "aborted counting hits on " << _mapped.getTab();
				break;
			}
         if (++m > 100000) {
            m = 0;
            qd(6) << "thread " << QThread::currentThread() << " counting hits...";
         }

			line = in.readLine();

			if (crx.indexIn(line, 0) != -1)
				wc[path] += 1;
		}
	}
	else
		fprintf(stderr, "could not open %s", path.toStdString().c_str());

	return wc;
}
/**************************************************************************//**
 * ReduceHits(): reduce hit maps to one map to return to caller.
 ******************************************************************************/
void Mapped::reduceHits  (WordCount &result, const WordCount &w) {
	QMapIterator<QString, int> i(w);
	while (i.hasNext()) {
		i.next();
		result[i.key()] += i.value();
	}
}
#endif
