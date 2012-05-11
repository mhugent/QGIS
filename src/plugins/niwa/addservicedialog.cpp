#include "addservicedialog.h"

AddServiceDialog::AddServiceDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f )
{
    setupUi( this );
    //todo: select the last service type
    mWFSRadioButton->setChecked( true );
}

AddServiceDialog::~AddServiceDialog()
{
}

QString AddServiceDialog::name() const
{
    return mNameLineEdit->text();
}

QString AddServiceDialog::service() const
{
    QString serviceString;
    if( mWMSRadioButton->isChecked() )
    {
        serviceString = "WMS";
    }
    else if( mWFSRadioButton->isChecked() )
    {
        serviceString = "WFS";
    }
    else if( mWCSRadioButton->isChecked() )
    {
        serviceString = "WCS";
    }
    return serviceString;
}

QString AddServiceDialog::url() const
{
    return mUrlLineEdit->text();
}

