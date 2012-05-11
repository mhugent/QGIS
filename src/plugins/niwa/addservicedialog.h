#ifndef ADDSERVICEDIALOG_H
#define ADDSERVICEDIALOG_H

#include "ui_addservicedialogbase.h"

class AddServiceDialog: public QDialog, private Ui::AddServiceDialogBase
{
    public:
    AddServiceDialog( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~AddServiceDialog();
    QString name() const;
    QString service() const;
    QString url() const;
};

#endif // ADDSERVICEDIALOG_H
