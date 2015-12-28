#ifndef MATCHER_H
#define MATCHER_H
#include "debug.h"

#include <QObject>
#include <QRegExp>
#include <QRunnable>
#include <QString>
#include <QTime>

#include "types.h"

QT_BEGIN_NAMESPACE
class QStandardItemModel;
QT_END_NAMESPACE

class Tab;

class Matcher : public QObject, public QRunnable {
Q_OBJECT
public:
   explicit Matcher(Tab *tab);
   virtual ~Matcher() { ENTEX2("Matcher::~Matcher", 1); }
   void run();
private:
   void countHits(WordCount *wc);

   Tab *tab;
   QRegExp m_rx;
   QTime m_timer;
signals:
   void matcherDone(bool done);
   void statusShow(QString s, Operation op = MATCHER);
   void updateMatchColumn(const WordCount *wc);
};

#endif // MATCHER_H
