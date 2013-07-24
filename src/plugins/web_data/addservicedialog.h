#ifndef ADDSERVICEDIALOG_H
#define ADDSERVICEDIALOG_H

#include "ui_addservicedialogbase.h"

class AddServiceDialog: public QDialog, private Ui::AddServiceDialogBase
{
  public:
    AddServiceDialog( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~AddServiceDialog();

    void setName( const QString& name );
    QString name() const;
    void setService( const QString& service );
    QString service() const;
    void setUrl( const QString& url );
    QString url() const;

    void enableServiceTypeSelection( bool enable );
};

#endif // ADDSERVICEDIALOG_H
