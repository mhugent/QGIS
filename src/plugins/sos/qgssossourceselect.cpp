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
#include "qgisinterface.h"
#include <QMessageBox>
#include <QUrl>

QgsSOSSourceSelect::QgsSOSSourceSelect( QgisInterface* iface, QWidget* parent, Qt::WFlags fl ):
    QDialog( parent, fl ), mIface( iface ), mCapabilities( 0 )
{
  setupUi( this );
  populateConnectionList();

  QPushButton* mAddButton = new QPushButton( tr( "&Add" ) );
  mButtonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addLayer() ) );
}

QgsSOSSourceSelect::~QgsSOSSourceSelect()
{
  delete mCapabilities;
}

void QgsSOSSourceSelect::on_mConnectButton_clicked()
{
  QgsOWSConnection connection( "SOS", mConnectionsComboBox->currentText() );
  delete mCapabilities;
  mCapabilities = new QgsSOSCapabilities( connection.uri().encodedUri() );
  connect( mCapabilities, SIGNAL( gotCapabilities() ), this, SLOT( gotCapabilities() ) );
  mCapabilities->requestCapabilities();
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

void QgsSOSSourceSelect::gotCapabilities()
{
  mOfferingsTreeWidget->clear();
  if ( !mCapabilities )
  {
    return;
  }

  //check network error
  const QString& networkError = mCapabilities->networkError();
  if ( !networkError.isEmpty() )
  {
    QMessageBox::critical( this, tr( "Network error" ), networkError );
  }

  //check xml error
  const QString& xmlError = mCapabilities->xmlError();
  if ( !xmlError.isEmpty() )
  {

  }

  const QStringList* observablePropertyList = mCapabilities->observableProperties();
  if ( observablePropertyList )
  {
    QStringList::const_iterator pIt = observablePropertyList->constBegin();
    for ( ; pIt != observablePropertyList->constEnd(); ++pIt )
    {
      mOfferingsTreeWidget->addTopLevelItem( new QTreeWidgetItem( QStringList() << *pIt ) );
    }
  }
}

void QgsSOSSourceSelect::addLayer()
{
  QgsOWSConnection connection( "SOS", mConnectionsComboBox->currentText() );
  QString getFeatureOfInterestUrl = connection.uri().param( "url" ) + "SERVICE=SOS&request=GetFeatureOfInterest&Version=2.0.0&datasource=0"; //NIWA SOS needs datasource parameter

  QString observedPropertiesString;
  QList<QTreeWidgetItem*> selectedItemList = mOfferingsTreeWidget->selectedItems();
  for ( int i = 0; i < selectedItemList.size(); ++i )
  {
    if ( i > 0 )
    {
      observedPropertiesString.append( "," );
    }
    observedPropertiesString.append( selectedItemList.at( i )->text( 0 ) );
  }
  if ( !selectedItemList.isEmpty() )
  {
    getFeatureOfInterestUrl += ( QString( "&observedProperty=" ) + QString( QUrl::toPercentEncoding( observedPropertiesString.toLocal8Bit() ) ) );
  }

  QString sosLayerName = observedPropertiesString.split( "/" ).last();

  if ( mIface )
  {
    mIface->addVectorLayer( getFeatureOfInterestUrl, sosLayerName, "SOS" );
  }
  return;
}

