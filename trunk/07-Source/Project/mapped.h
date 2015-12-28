/******************************************************************************
 * NAME:    mapped.h
 *
 * PURPOSE: Definitions for mapped.cpp.
 *
 * DATE:    June 29, 2010
*******************************************************************************/
#ifndef MAPPED_H
#define MAPPED_H

#if 0
#include "debug.h"

#include <QMutex>
#include <QRegExp>
#include <QString>

#include "types.h"

class Mapped;
class Tab;

class Mapped {
public:
   Mapped(const Tab *tab, const QRegExp &rx, const QString &abspath);

	static WordCount countHits(const Mapped & mapped);
	static void reduceHits(WordCount &result, const WordCount &w);

	inline QString getAbspath() const { return m_abspath; }
   inline QRegExp getRegExp() const { return m_rx; }
   inline const Tab * const getTab() const { return m_tab; }
private:
   const Tab *m_tab;  ///< tab owning map calculations
   QRegExp m_rx;
   QString m_abspath;

   static QMutex mutex;
};
#endif
#endif // MAPPED_H
