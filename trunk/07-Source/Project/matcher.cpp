/**************************************************************************//**
 * NAME:    matcher.cpp
 *
 * PURPOSE: Count matching lines in files using threads (not Qt::Concurrent).
 *
 * DATE:    July 29, 2010
*******************************************************************************/
#include "debug.h"

#include <QFile>
#include <QStandardItemModel>
#include <QTime>

#include "atomicqueue.h"
#include "matcher.h"
#include "parms.h"
#include "settings.h"
#include "tab.h"

Matcher::Matcher  (Tab *_tab)
:
tab(_tab),
m_rx(tab->getParms()->getRegExp())
{
   ENTEX2("Matcher::Matcher", 1);
   connect(this, SIGNAL(matcherDone(bool)), tab, SLOT(matcherDone(bool)));
   connect(this, SIGNAL(statusShow(QString,Operation)), tab, SLOT(statusShow(QString,Operation)));
   connect(this, SIGNAL(updateMatchColumn(const WordCount *)), tab, SLOT(updateMatchColumn(const WordCount *)));

   m_timer.start();
}
/**************************************************************************//**
 * CountHits(): Count lines in files that contain at least one content.
 ******************************************************************************/
void Matcher::countHits  (WordCount *_wc) {
   ENTEX2("Matcher::countHits", 4);
   qd(10) << ": dequeued: " << *_wc;

   QHashIterator<QString, int> i(*_wc);
   while (i.hasNext()) {
      i.next();

      QString path = i.key();
      QFile file(path);

      if (!file.open(QIODevice::ReadOnly)) {
         qd(0) << tr("could not open %1").arg(path);  // permissions changed
         continue;
      }

      int matches = 0;
      QString line;
      QTextStream in(&file);
      while (!in.atEnd()) {
         if (tab->getAbortMatcher()) {
            qd(6) << "aborted counting matches on tab " << tab;
            break;
         }

         if (m_timer.elapsed() > 1000) {
            qd(6) << "counting matches [" << matches << "]...";
            m_timer.restart();
         }

         if (tab->getSettings()->getUi()->binaryCheckBox->isChecked())
            line = in.readLine();
         else
            line = ::makePrintable(in.readLine());  ///< expensive but have to sync with grep

         if (m_rx.indexIn(line) != -1)
            ++matches;
      }
      qd(4) << "inserting key-" << i.key() << "matches:" << matches;
      _wc->insert(i.key(), matches);  ///< now matches and row number mapped to filepath (LIFO)

      file.close();
   }

   emit updateMatchColumn(_wc);  ///< only update when done to avoid overloading main thread
}
/**************************************************************************//**
 * run(): loop and dequeue data as available.
 ******************************************************************************/
void Matcher::run  () {
   ENTEX2("Matcher::run", 10);

   AtomicQueue<WordCount *> *aq = tab->getAtomicQueue();  // shortcut

   forever {
      if (tab->getAbortMatcher()) {
         emit matcherDone(true);
         break;
      }

      if (!aq->isEmpty()) {
         countHits(aq->dequeue());
      }
      else if (!tab->getFindState()) {  // find is done and queue is empty
         qd(10) << "emitting matcher done, run state false...";
         emit matcherDone(true);
         break;
      }
      else if (m_timer.elapsed() > 1000) {
         qd(10) << "looping on queue, size: " << aq->size();
         m_timer.restart();
      }
   }
}
