/******************************************************************************
 * NAME:    about.cpp
 *
 * PURPOSE: Implementation of about widget.
 *
 * DATE:    January 21, 2010
*******************************************************************************/
#include "about.h"

#include "ui_about.h"

About::About  (QWidget *parent)
:
QDialog(parent),
m_ui(new Ui::About)
{
	m_ui->setupUi(this);
}
About::~About  () {
	delete m_ui;
}
void About::changeEvent  (QEvent *e) {
	if (e->type()== QEvent::LanguageChange)
		m_ui->retranslateUi(this);
}
Ui::About *About::ui  () {
	return m_ui;
}
