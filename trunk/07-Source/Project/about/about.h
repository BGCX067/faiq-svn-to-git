/******************************************************************************
 * NAME:    about.h
 *
 * PURPOSE: Class definition for about widget.
 *
 * DATE:    January 21, 2010
*******************************************************************************/
#ifndef ABOUT_H
#define ABOUT_H

#include <QtGlobal>
#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE
class QEvent;
class QWidget;
QT_END_NAMESPACE

namespace Ui {
	class About;
}

class About : public QDialog {
	 Q_OBJECT
	 Q_DISABLE_COPY(About)
public:
	 explicit About(QWidget *parent = 0);
	 virtual ~About();
	 Ui::About *ui();
protected:
	 virtual void changeEvent(QEvent *e);
private:
	 Ui::About *m_ui;
};

#endif // ABOUT_H
