#ifndef NIWAPLUGINDIALOG_H
#define NIWAPLUGINDIALOG_H

#include "ui_niwaplugindialogbase.h"

class NiwaPluginDialog: public QDialog, private Ui::NiwaPluginDialogBase
{
    Q_OBJECT
  public:
    NiwaPluginDialog( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~NiwaPluginDialog();
};

#endif // NIWAPLUGINDIALOG_H
