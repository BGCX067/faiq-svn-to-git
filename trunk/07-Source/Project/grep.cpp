/**************************************************************************//**
 * NAME:    grep.cpp
 *
 * PURPOSE: Grep file.
 *
 * DATE:    July 11, 2010
*******************************************************************************/
#include "debug.h"

#include <QFile>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QTime>

#include "atomicqueue.h"
#include "grep.h"
#include "parms.h"
#include "settings.h"
#include "settingsglobal.h"
#include "tab.h"
#include "presets/folders.h"
#include "presets/presets.h"
#include "util.h"

#include "ui_settingsglobal.h"
/**************************************************************************//**
 * Grep(): construct a Grep object.
 ******************************************************************************/
Grep::Grep  (const QString &path, Tab *_tab)
:
m_cancel(false),
m_bytes(0),
tab(_tab),
m_rx(tab->getParms()->getRegExp()),
m_path(path)
{
   ENTEX2("Grep::Grep", 3);

	connect(this, SIGNAL(boxError(QString,QMessageBox::Icon)), tab, SLOT(boxError(QString,QMessageBox::Icon)));
	connect(this, SIGNAL(grepProgressBar_setFormat(QString)), tab, SLOT(on_grepProgressBar_setFormat(QString)));
	connect(this, SIGNAL(grepProgressBar_setRange(int,int)), tab, SLOT(on_grepProgressBar_setRange(int,int)));
	connect(this, SIGNAL(grepProgressBar_setValue(int)), tab, SLOT(on_grepProgressBar_setValue(int)));
   connect(this, SIGNAL(grepResults(QList<Line>)), tab, SLOT(storeLines(QList<Line>)));
   connect(this, SIGNAL(statusShow(QString,Operation)), tab, SLOT(statusShow(QString,Operation)));
}
/**************************************************************************//**
 * ~Grep(): destruct.
 ******************************************************************************/
 Grep::~Grep  () {
   ENTEX2("Grep::~Grep", 0);
}
/**************************************************************************//**
 * GrepInit(): initialize grep operation.
 ******************************************************************************/
void Grep::grepInit  () {
	grep();
}
/**************************************************************************//**
 * Grep(): perform a grep operation then signal Tab with the list of Line
 *         objects.  Batching is available by varying the size of the list.
 *         The beginning and end of file are marked with special Line objects.
 *	\todo
 * fill viewport only, then fill as user scrolls down (googlesque)
 ******************************************************************************/
void Grep::grep  () {
   ENTEX("Grep::grep");

   qd(10) << "BEGIN grep on-" << m_path;
	QFile file(m_path);

   if (file.size() < 1LL)  ///< in case an empty file actually gets here
      return;

   if (!file.open(QIODevice::ReadOnly)) {
      if (!Singleton<SettingsGlobal>::instance()->getUi()->ignoreCheckBox->isChecked())
         ::boxError(tr("Could not open '%1'").arg(m_path));
		return;
	}

   bool canceled = false,
        progressShow = file.size() > GREP_MIN_PROGRESS;
   int hits     = 0,
       lineno   = 0,
       numLines = 0,
       prevHits = 0,
       rangeMax = ::intMax(file.size());
   qint64 bytes = 0LL,
          progress = 0LL;
   Ui::Settings * sui = tab->getSettings()->getUi();  // shortcut
   QList<Line> lines;
   QString line;
   QTextStream in(&file);

   lines << Line(GREP_BEGIN, m_path);  // begin package
   emit grepResults(lines);
   lines.clear();

   if (progressShow) {
      emit grepProgressBar_setRange(0, rangeMax);
      emit grepProgressBar_setFormat("%p% Scanning");
   }

   QTime timer;
   timer.start();
   while (!in.atEnd()) {
      if (m_cancel) {
         if (progressShow)
            emit grepProgressBar_setValue(0);
         canceled = true;
         break;
      }
      ++lineno;

      if (sui->binaryCheckBox->isChecked())
         line = in.readLine();
      else
         line = ::makePrintable(in.readLine());  ///< makes highlighter always work

      if (Singleton<SettingsGlobal>::instance()->getUi()->whitespaceCheckBox->isChecked())
         line = line.trimmed();

      if (m_rx.indexIn(line) != -1) {  ///< very fast loop

         int maxLines = tab->getSettings()->getUi()->linesSpinBox->value();
         if (++numLines > maxLines  &&  maxLines)
            break;

         prevHits = hits;
         hits += ::countHits(m_rx, line);

         int maxHits = sui->matchesSpinBox->value();
         //qd(0) << "maxHits:" << maxHits;
         if (maxHits  &&  hits > maxHits) {
            line = truncateLine(m_rx, line, maxHits - prevHits);  /// don't exceed max hits
            hits = maxHits;
         }

         bytes += line.size();

         lines << Line(lineno, m_path, line);
         if (true) {  ///< \todo here we can batch lines based on quantity or time
            emit grepResults(lines);
            statusShow(hits, numLines, timer.elapsed(), bytes);
				lines.clear();
         }

         if (maxHits  &&  hits >= maxHits)
            break;
      }
      if (progressShow)
         emit grepProgressBar_setValue(::intMax(progress += line.size()));

   }  // while()
   emit grepProgressBar_setRange(0,0);  // prevent hung progress bars when display finishes before thread, see Tab::showLine()

   if (canceled)
      qd(0) << "***Thread Preempted***";
   qd(9) << tr("grepped %1 bytes of %2 in %3 ms").arg(bytes).arg(file.size()).arg(timer.elapsed());

   if (lines.size()) {  ///< flush the list
      emit grepResults(lines);
      statusShow(hits, numLines, timer.elapsed(), bytes);
   }

   lines.clear();

   lines << Line(GREP_EOF, m_path);  // eof package
   emit grepResults(lines);

   if (progressShow)
      emit grepProgressBar_setValue(rangeMax);

   m_bytes = ::intMax(bytes);

}  // grep()
/**************************************************************************//**
 * StatusShow(): Show specific grep status.
 ******************************************************************************/
 void Grep::statusShow  (int hits, int numLines, int time, qint64 bytes) {
   const char *m = (numLines == 1) ? "in %1 line" : "in %1 lines";
   QString s =	tr("%1 matches ").arg(hits) +
               tr(m).arg(numLines) +
               tr(" (%1").arg(::bytes2units(bytes)) +
               tr(" in %1)").arg(ms2units(time));

   emit statusShow(s, GREP);
}
/**************************************************************************//**
 * TruncateLine(): Truncate line to contain, at most, specified number of hits.
 *
 * Parms         : _rx   - regular expression
 *                 _line - line with regular expression hits
 *                 _max  - max number of hits to remain after truncation
 *
 * Returns       : a line with, at most, the specified number of hits
 ******************************************************************************/
 QString Grep::truncateLine  (const QRegExp &_rx, QString _line, int _max) {
   if (_line.isEmpty())
      return QString();

   register int hits = 0,
                index = _line.indexOf(_rx),
                n = _line.length() + 1;    // guarantee truncation

   if (_max < 1)
      return _line.remove(index, n);

   while (index >= 0) {
      int length = _rx.matchedLength();
      if (++hits >= _max)
         return _line.remove(index + length, n);
      index = _line.indexOf(_rx, index + length);
   }
   return _line;
}
