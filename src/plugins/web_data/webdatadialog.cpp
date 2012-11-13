#include "webdatadialog.h"
#include "addservicedialog.h"
#include "qgsnetworkaccessmanager.h"
#include <QDomDocument>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>

WebDataDialog::WebDataDialog( QgisInterface* iface, QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ), mIface( iface ),
    mModel( iface ), mNIWAServicesRequestFinished( false )
{
  setupUi( this );
  insertServices();
  mLayersTreeView->setModel( &mModel );
  connect( mLayersTreeView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( adaptLayerButtonStates() ) );
}

WebDataDialog::~WebDataDialog()
{
}

void WebDataDialog::on_mConnectPushButton_clicked()
{
  QString url = serviceURLFromComboBox();
  int currentIndex = mServicesComboBox->currentIndex();
  QString serviceType = mServicesComboBox->itemData( currentIndex ).toString();
  QString serviceTitle = mServicesComboBox->currentText();
  mModel.addService( serviceTitle, url, serviceType );
}

void WebDataDialog::on_mAddToMapButton_clicked()
{
  QItemSelectionModel * selectModel = mLayersTreeView->selectionModel();
  QModelIndexList selectList = selectModel->selectedRows( 0 );
  if ( selectList.size() > 0 )
  {
    mModel.addEntryToMap( selectList.at( 0 ) );
  }
  adaptLayerButtonStates();
}

void WebDataDialog::on_mRemoveFromMapButton_clicked()
{
  QItemSelectionModel * selectModel = mLayersTreeView->selectionModel();
  QModelIndexList selectList = selectModel->selectedRows( 0 );
  if ( selectList.size() > 0 )
  {
    mModel.removeEntryFromMap( selectList.at( 0 ) );
  }
  adaptLayerButtonStates();
}

QString WebDataDialog::serviceURLFromComboBox()
{
  int currentIndex = mServicesComboBox->currentIndex();
  if ( currentIndex == -1 )
  {
    return "";
  }
  QString serviceType = mServicesComboBox->itemData( currentIndex ).toString();

  //make service url
  QSettings settings;
  QString url = settings.value( QString( "/Qgis/connections-" ) + serviceType.toLower() + "/" + mServicesComboBox->currentText() + QString( "/url" ) ).toString();
  if ( !url.endsWith( "?" ) && !url.endsWith( "&" ) )
  {
    if ( url.contains( "?" ) )
    {
      url.append( "&" );
    }
    else
    {
      url.append( "?" );
    }
  }
  return url;
}

void WebDataDialog::on_mRemovePushButton_clicked()
{
  int currentIndex = mServicesComboBox->currentIndex();
  if ( currentIndex == -1 )
  {
    return;
  }

  QString serviceType = mServicesComboBox->itemData( currentIndex ).toString();
  QString name = mServicesComboBox->itemText( currentIndex );

  QSettings s;
  s.remove( "/Qgis/connections-" + serviceType.toLower() + "/" + name );
  insertServices();
}

void WebDataDialog::on_mEditPushButton_clicked()
{
  int currentIndex = mServicesComboBox->currentIndex();
  if ( currentIndex == -1 )
  {
    return;
  }

  QString serviceType = mServicesComboBox->itemData( currentIndex ).toString();
  QString name = mServicesComboBox->itemText( currentIndex );

  QSettings s;
  QString url = serviceURLFromComboBox();

  AddServiceDialog d;
  d.setName( name );
  d.setUrl( url );
  d.setService( serviceType );
  d.enableServiceTypeSelection( false );
  if ( d.exec() == QDialog::Accepted )
  {
    setServiceSetting( d.name(), d.service(), d.url() );
  }
}

void WebDataDialog::on_mAddPushButton_clicked()
{
  AddServiceDialog d( this );
  if ( d.exec() == QDialog::Accepted )
  {
    setServiceSetting( d.name(), d.service(), d.url() );
  }
}

void WebDataDialog::on_mAddNIWAServicesButton_clicked()
{
  addServicesFromHtml( "https://www.niwa.co.nz/ei/feeds/report" );
}

void WebDataDialog::on_mAddLINZServicesButton_clicked()
{
  //ask user about the LINZ key
  QString key = QInputDialog::getText( 0, tr( "Enter your personal LINZ key" ), tr( "Key:" ) );
  if ( key.isNull() )
  {
    return;
  }

  //add WFS
  setServiceSetting( "LINZ WFS", "WFS", "http://wfs.data.linz.govt.nz/" + key + "/wfs" );

  //add WMS
  setServiceSetting( "LINZ WMS", "WMS", "http://wms.data.linz.govt.nz/" + key + "/r/wms" );
}


void WebDataDialog::setServiceSetting( const QString& name, const QString& serviceType, const QString& url )
{
  if ( name.isEmpty() || url.isEmpty() || serviceType.isEmpty() )
  {
    return;
  }

  //filter out / from name
  QString serviceName = name;
  serviceName.replace( "/", "_" );

  QSettings s;
  s.setValue( "/Qgis/connections-" + serviceType.toLower() + "/" + serviceName + "/url", url );
  insertServices();
}

void WebDataDialog::insertServices()
{
  mServicesComboBox->clear();
  insertServices( "WFS" );
  insertServices( "WMS" );
}

void WebDataDialog::insertServices( const QString& service )
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-" + service.toLower() );
  QStringList keys = settings.childGroups();
  QStringList::const_iterator it = keys.constBegin();
  for ( ; it != keys.constEnd(); ++it )
  {
    mServicesComboBox->addItem( *it, service );
  }
}

void WebDataDialog::addServicesFromHtml( const QString& url )
{
  //get html page with QgsNetworkAccessManager

  mNIWAServicesRequestFinished = false;
  QNetworkRequest request( url );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, SIGNAL( finished() ), this, SLOT( NIWAServicesRequestFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleDownloadProgress( qint64, qint64 ) ) );

  while ( !mNIWAServicesRequestFinished )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
  }

  QByteArray response = reply->readAll();

  //debug
  //QString responseString( response );
  //qWarning( responseString.toLocal8Bit().data() );
  reply->deleteLater();

  QDomDocument htmlDoc;
  QString errorMsg;
  int errorLine;
  int errorColumn;
  if ( !htmlDoc.setContent( response, false, &errorMsg, &errorLine, &errorColumn ) )
  {
    QMessageBox::critical( 0, tr( "Failed to parse XHTML file" ), tr( "Error parsing the xhtml from %1: %2 on line %3, column %4" ).arg( url ).arg( errorMsg ).arg( errorLine ).arg( errorColumn ) );
    return;
  }

  QDomNodeList tbodyNodeList = htmlDoc.elementsByTagName( "tbody" );
  for ( int i = 0; i < tbodyNodeList.size(); ++i )
  {
    QDomNodeList trNodeList = tbodyNodeList.at( i ).toElement().elementsByTagName( "tr" );
    for ( int j = 0; j < trNodeList.size(); ++j )
    {
      QDomNodeList tdNodeList = trNodeList.at( j ).toElement().elementsByTagName( "td" );
      if ( tdNodeList.size() > 4 )
      {
        QString name = tdNodeList.at( 0 ).toElement().text();
        QString url = tdNodeList.at( 4 ).toElement().firstChildElement( "a" ).text();
        QString service = tdNodeList.at( 3 ).toElement().text();
        setServiceSetting( name, service, url );
      }
    }
  }
  mStatusLabel->setText( tr( "Ready" ) );
}

void WebDataDialog::NIWAServicesRequestFinished()
{
  mNIWAServicesRequestFinished = true;
}

void WebDataDialog::handleDownloadProgress( qint64 progress, qint64 total )
{
  QString progressMessage;
  if ( total != -1 )
  {
    progressMessage = tr( "%1 of %2 bytes downloaded" ).arg( progress ).arg( total );
  }
  else
  {
    progressMessage = tr( "%1 bytes downloaded" ).arg( progress );
  }
  mStatusLabel->setText( progressMessage );
}

void WebDataDialog::on_mChangeOnlineButton_clicked()
{
  QItemSelectionModel * selectModel = mLayersTreeView->selectionModel();
  QModelIndexList selectList = selectModel->selectedRows( 0 );
  if ( selectList.size() > 0 )
  {
    mModel.changeEntryToOnline( selectList.at( 0 ) );
  }
  adaptLayerButtonStates();
}

void WebDataDialog::on_mChangeOfflineButton_clicked()
{
  QItemSelectionModel * selectModel = mLayersTreeView->selectionModel();
  QModelIndexList selectList = selectModel->selectedRows( 0 );
  if ( selectList.size() > 0 )
  {
    mModel.changeEntryToOffline( selectList.at( 0 ) );
  }
  adaptLayerButtonStates();
}

void WebDataDialog::adaptLayerButtonStates()
{
  QItemSelectionModel * selectModel = mLayersTreeView->selectionModel();
  QModelIndexList selectList = selectModel->selectedRows( 0 );
  if ( selectList.size() > 0 )
  {
    QString status = mModel.layerStatus( selectList.at( 0 ) );
    mChangeOfflineButton->setEnabled( status.compare( "offline", Qt::CaseInsensitive ) != 0 );
    mChangeOnlineButton->setEnabled( status.compare( "offline", Qt::CaseInsensitive )  == 0 );
    bool inMap = mModel.layerInMap( selectList.at( 0 ) );
    mAddToMapButton->setEnabled( !inMap );
    mRemoveFromMapButton->setEnabled( inMap );
  }
}
