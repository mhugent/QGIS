/***************************************************************************
                          qgssossourceselect.cpp  -  description
                          ------------------------------------
    begin                : June 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssossourceselect.h"
#include "qgsowsconnection.h"
#include "qgssoscapabilities.h"
#include "qgsnewhttpconnection.h"
#include <QMessageBox>

QgsSOSSourceSelect::QgsSOSSourceSelect( QWidget* parent, Qt::WFlags fl ): QDialog( parent, fl ), mCapabilities( 0 )
{
  setupUi( this );
  populateConnectionList();
}

QgsSOSSourceSelect::~QgsSOSSourceSelect()
{
  delete mCapabilities;
}

void QgsSOSSourceSelect::on_mConnectButton_clicked()
{

}

void QgsSOSSourceSelect::on_mNewButton_clicked()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-sos/" );
  nc.setWindowTitle( tr( "Create a new SOS connection" ) );

  if ( nc.exec() == QDialog::Accepted )
  {
    populateConnectionList();
  }
}

void QgsSOSSourceSelect::on_mEditButton_clicked()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-sos/", mConnectionsComboBox->currentText() );
  nc.setWindowTitle( tr( "Modify SOS connection" ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    //emit connectionsChanged();
  }
}

void QgsSOSSourceSelect::on_mDeleteButton_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( mConnectionsComboBox->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    QgsOWSConnection::deleteConnection( "SOS", mConnectionsComboBox->currentText() );
    mConnectionsComboBox->removeItem( mConnectionsComboBox->currentIndex() );
  }
}

void QgsSOSSourceSelect::populateConnectionList()
{
  mConnectionsComboBox->clear();

  QStringList keys = QgsOWSConnection::connectionList( "SOS" );
  QStringList::const_iterator it = keys.constBegin();
  for ( ; it != keys.constEnd(); ++it )
  {
    mConnectionsComboBox->addItem( *it );
  }

  bool buttonsEnabled = ( keys.constBegin() != keys.constEnd() );
  mConnectButton->setEnabled( buttonsEnabled );
  mEditButton->setEnabled( buttonsEnabled );
  mDeleteButton->setEnabled( buttonsEnabled );

  //set last used connection
  QString selectedConnection = QgsOWSConnection::selectedConnection( "SOS" );
  int index = mConnectionsComboBox->findText( selectedConnection );
  if ( index != -1 )
  {
    mConnectionsComboBox->setCurrentIndex( index );
  }
}

