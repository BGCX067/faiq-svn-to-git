/**************************************************************************//**
 * NAME:    presets.h
 *
 * PURPOSE: Defines class Presets.
 *
 * DATE:    December 12, 2009
*******************************************************************************/
#ifndef PRESETS_H
#define PRESETS_H

#include <QValidator>
#include <QWidget>

#include "settings.h"

#include "ui_presets.h"

namespace Ui {
   class Presets;
}

class Presets : public QWidget {
Q_OBJECT
public:
   explicit Presets(QWidget *parent = 0);
   virtual ~Presets();
   Ui::Presets *getUi() const { return ui; }
	void okApply();
protected:
    void changeEvent(QEvent *);
private:
   Ui::Presets *ui;
   QValidator *validator;
private slots:
   void on_buttonBox_clicked(QAbstractButton *);
};

#endif // PRESETS_H
