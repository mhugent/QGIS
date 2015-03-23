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

void AddServiceDialog::setName( const QString& name )
{
  mNameLineEdit->setText( name );
}

QString AddServiceDialog::name() const
{
  return mNameLineEdit->text();
}

void AddServiceDialog::setService( const QString& service )
{
  if ( service.compare( "WMS", Qt::CaseInsensitive ) == 0 )
  {
    mWMSRadioButton->setChecked( true );
  }
  else if ( service.compare( "WFS", Qt::CaseInsensitive ) == 0 )
  {
    mWFSRadioButton->setChecked( true );
  }
  else if ( service.compare( "WCS", Qt::CaseInsensitive ) == 0 )
  {
    mWCSRadioButton->setChecked( true );
  }
}

QString AddServiceDialog::service() const
{
  QString serviceString;
  if ( mWMSRadioButton->isChecked() )
  {
    serviceString = "WMS";
  }
  else if ( mWFSRadioButton->isChecked() )
  {
    serviceString = "WFS";
  }
  else if ( mWCSRadioButton->isChecked() )
  {
    serviceString = "WCS";
  }
  return serviceString;
}

void AddServiceDialog::setUrl( const QString& url )
{
  mUrlLineEdit->setText( url );
}

QString AddServiceDialog::url() const
{
  return mUrlLineEdit->text();
}

void AddServiceDialog::enableServiceTypeSelection( bool enable )
{
  mWMSRadioButton->setEnabled( enable );
  mWFSRadioButton->setEnabled( enable );
  mWCSRadioButton->setEnabled( enable );
}

