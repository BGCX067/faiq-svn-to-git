/*****************************************************************************/
/*! \file : types.h
 *
 * PURPOSE: Provides application-wide type definitions.
 *
 * DATE:    September 28, 2009
*******************************************************************************/
#ifndef TYPES_H
#define TYPES_H

#include <QFuture>
#include <QFutureWatcher>
#include <QMultiHash>
#include <QString>
#include <QStringList>

const int nullptr = 0;  // for null pointer initialization (should be a keyword in future C++)

typedef unsigned long long ull;
typedef QMultiHash<QString, int> WordCount;
typedef QFuture<WordCount> fwc;
typedef QFutureWatcher<void> fw;

enum Browse {SEARCH, CONTEXT, FOLDERS};  // help keep mru dir on different ::browse()
enum Error {ERR_INVALID, ERR_FILE};
enum Layout {HORIZONTAL, VERTICAL};  // gui stacked widget index 0 is horiz, 1 is vert
enum Operation {FIND, GREP, MAIN, MATCHER, MIDDLE, TIP};  // see slot Tab::showStatus()
enum State {QUICK, ADVANCED};
enum TargetRadio {ALSO, ONLY, SKIP};

template <typename A>
class AC {  // handy auto cleanup class for early exits
public:
   inline AC(A * const _p)
	:
	m_abort(false),
	p(_p)
	{}
   inline virtual ~AC() {
      if (!m_abort && p != nullptr)
			delete p;
	}
   inline void abort() { m_abort = true; }
private:
	bool m_abort;
   A *p;
};

struct Line {
   Line() {
      lineno = 0;
      filepath = QString();
      line = QString();
   }
   Line(int _lineno, const QString &_filepath = "", const QString &_line = "")
	:
   lineno(_lineno),
   filepath(_filepath),
	line(_line)
   {}
   Line(const Line &other) {
      lineno = other.lineno;
      filepath = other.filepath;
      line = other.line;
   }
   virtual ~Line() {}
   Line & operator=(const Line &rhs) {
      lineno = rhs.lineno;
      filepath = rhs.filepath;
      line = rhs.line;
      return *this;
   }
public:
   int lineno;
   QString filepath;
	QString line;
};

class Sem {
public:
	Sem() { m_count = 0; }
	inline int getCount() const { return m_count; }
	inline void down() { --m_count; }
	inline void reset() { m_count = 0; }
	inline void up() { ++m_count; }
private:
	int m_count;
};

template <typename T>
class Singleton : private T {
public:
   static T *instance();
private:
	Singleton() {}
	virtual ~Singleton() {}
};
template <typename T>
T *Singleton<T>::instance() {
	static Singleton<T> obj;
	return &obj;
}
#endif
