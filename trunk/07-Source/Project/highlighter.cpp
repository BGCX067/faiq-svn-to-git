/**************************************************************************//**
 * NAME:      highlighter.cpp
 *
 * PURPOSE:   Provides highlighting for matched expressions in QTextEdit
 *            object displaying matched lines.
 *
 * DATE:      September 28, 2009
 *
 * NOTES:     Will sometimes break on non-printable QChars in the target
 *            QString.  Make sure to remove the offending QChars.
*******************************************************************************/
#include "highlighter.h"
#include "util.h"
/**************************************************************************//**
 * Highlighter(): Construct.
 ******************************************************************************/
Highlighter::Highlighter  (QTextDocument *parent, const QRegExp &rx)
:
QSyntaxHighlighter(parent),
m_rx(rx)
{
   initFormat();
}
/**************************************************************************//**
 * Highlighter(): Construct.
 ******************************************************************************/
Highlighter::Highlighter  (QTextDocument *parent, const QString &_text,
                           QRegExp::PatternSyntax _syntax,
                           Qt::CaseSensitivity _cs)
:
QSyntaxHighlighter(parent)
{
   m_rx.setPattern(_text);
   m_rx.setPatternSyntax(_syntax);
   m_rx.setCaseSensitivity(_cs);

   initFormat();
}
/**************************************************************************//**
 * Highlighter(): Destruct.
 ******************************************************************************/
Highlighter::~Highlighter  () {
}
/**************************************************************************//**
 * ChangeFormat: Helper function.
 ******************************************************************************/
void Highlighter::changeFormat  (QBrush _foreground, int _fontWeight,
                                                     int _length, int _start) {
   m_format.setForeground(_foreground);
   m_format.setFontWeight(_fontWeight);
   setFormat(_start, _length, m_format);
}
/**************************************************************************//**
 * HighlightBlock(): Workhorse.
 ******************************************************************************/
void Highlighter::highlightBlock  (const QString &_text) {
   int hack = _text.indexOf(">"),  // line number form is: n>
       index = _text.indexOf(m_rx);
   while (index >= 0) {
      int length = m_rx.matchedLength();
      if (index > hack)  // only highlight after line number
         setFormat(index, length, m_format);
      index = _text.indexOf(m_rx, index + length);
   }
}
/**************************************************************************//**
 * InitFormat: Initialize the format.
 ******************************************************************************/
void Highlighter::initFormat  () {
   QBrush brush(COLOR_HIGHLIGHT);
   m_format.setForeground(brush);
   m_format.setFontWeight(QFont::Bold);
}
