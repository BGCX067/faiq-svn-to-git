/**************************************************************************//**
 * NAME:      highlighter.h
 *
 * PURPOSE:   Provides highlighting for matched expressions in QTextEdit
 *            object displaying matched lines.
 *
 * DATE:      September 28, 2009
*******************************************************************************/
#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H
#include "debug.h"

#include <QBrush>
#include <QRegExp>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class Highlighter : public QSyntaxHighlighter {
Q_OBJECT
public:
   Highlighter(QTextDocument *parent, const QRegExp &_rx);
   Highlighter(QTextDocument *parent = 0, const QString &_text = "",
               QRegExp::PatternSyntax _syntax = QRegExp::RegExp,
               Qt::CaseSensitivity _cs = Qt::CaseInsensitive);
   virtual ~Highlighter();
   void changeFormat(QBrush foreground, int fontWeight, int length, int start = 0);
   inline void init() {}
   void initFormat();
protected:
   void highlightBlock(const QString &text);
private:
   QRegExp m_rx;
   QTextCharFormat m_format;
};

#endif // HIGHLIGHTER_H
