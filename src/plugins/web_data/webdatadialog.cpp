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
}

void WebDataDialog::on_mRemoveFromMapButton_clicked()
{
  QItemSelectionModel * selectModel = mLayersTreeView->selectionModel();
  QModelIndexList selectList = selectModel->selectedRows( 0 );
  if ( selectList.size() > 0 )
  {
    mModel.removeEntryFromMap( selectList.at( 0 ) );
  }
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
#if 0
  //get current entry
  QTreeWidgetItem* item = mLayerTreeWidget->currentItem();
  if ( !item )
  {
    return;
  }

  bool layerInMap = ( item->checkState( 3 ) == Qt::Checked );
  QString serviceType = item->text( 1 );
  QString layername = item->text( 0 );

  if ( layerInMap )
  {
    //generate new wfs/wms layer
    //exchange with layer in map
    QgsMapLayer* onlineLayer = 0;
    if ( serviceType == "WFS" )
    {
      QString wfsUrl = wfsUrlFromLayerItem( item );
      onlineLayer = mIface->addVectorLayer( wfsUrl, layername, "WFS" );
    }
    else if ( serviceType == "WMS" )
    {
      //get preferred style, crs, format
      QString url, format, crs;
      QString providerKey = "wms";
      QStringList layers, styles;
      wmsParameterFromItem( item, url, format, crs, layers, styles );

      //add to map
      onlineLayer = mIface->addRasterLayer( url, item->text( 0 ), providerKey, layers, styles, format, crs );
    }

    if ( onlineLayer )
    {
      exchangeLayer( item->data( 3, Qt::UserRole ).toString(), onlineLayer );
      item->setData( 3, Qt::UserRole, onlineLayer->id() );
    }
  }

  QString offlineFileName = item->data( 2, Qt::UserRole ).toString();
  deleteOfflineDatasource( serviceType, offlineFileName );

  item->setText( 2, "online" );
  item->setIcon( 2, QIcon( ":/niwa/icons/online.png" ) );
  item->setData( 2, Qt::UserRole, "" );
  mChangeOfflineButton->setEnabled( true );
  mChangeOnlineButton->setEnabled( false );
#endif //0
}

void WebDataDialog::on_mChangeOfflineButton_clicked()
{
#if 0
  //get current entry
  QTreeWidgetItem* item = mLayerTreeWidget->currentItem();
  if ( !item )
  {
    return;
  }

  //get current layer details
  QString layername = item->text( 0 );
  QString url = item->data( 0, Qt::UserRole ).toString();
  QString serviceType = item->text( 1 );
  QString saveFilePath = QgsApplication::qgisSettingsDirPath() + "/cachelayers/";
  QDateTime dt = QDateTime::currentDateTime();
  QString layerId = layername + dt.toString( "yyyyMMddhhmmsszzz" );
  QString filePath;

  bool layerInMap = ( item->checkState( 3 ) == Qt::Checked );
  bool offlineOk = false;

  if ( serviceType == "WFS" )
  {
    //create table <layername//url>
    QgsVectorLayer* wfsLayer = 0;
    if ( layerInMap )
    {
      wfsLayer = static_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( item->data( 3, Qt::UserRole ).toString() ) );
    }
    else
    {
      QString wfsUrl = wfsUrlFromLayerItem( item );
      wfsLayer = new QgsVectorLayer( wfsUrl, layername, "WFS" );
    }

    filePath = saveFilePath + layerId + ".shp";
    const QgsCoordinateReferenceSystem& layerCRS = wfsLayer->crs();
    offlineOk = ( QgsVectorFileWriter::writeAsVectorFormat( wfsLayer, filePath,
                  "UTF-8", &layerCRS, "ESRI Shapefile" ) == QgsVectorFileWriter::NoError );
    if ( offlineOk && layerInMap )
    {
      QgsVectorLayer* offlineLayer = mIface->addVectorLayer( filePath, layername, "ogr" );
      exchangeLayer( item->data( 3, Qt::UserRole ).toString(), offlineLayer );
      item->setData( 3, Qt::UserRole, offlineLayer->id() );
    }

    if ( !layerInMap )
    {
      delete wfsLayer;
    }
  }
  else if ( serviceType == "WMS" )
  {
    //get raster layer
    QgsRasterLayer* wmsLayer = 0;
    if ( layerInMap )
    {
      wmsLayer = static_cast<QgsRasterLayer*>( QgsMapLayerRegistry::instance()->mapLayer( item->data( 3, Qt::UserRole ).toString() ) );
    }
    else
    {
      //get preferred style, crs, format
      QString url, format, crs;
      QString providerKey = "wms";
      QStringList layers, styles;
      wmsParameterFromItem( item, url, format, crs, layers, styles );
      wmsLayer = new QgsRasterLayer( 0, url, layername, "wms", layers, styles, format, crs );
    }

    //call save as dialog
    filePath = saveFilePath + "/" + layerId;
    QgsRasterLayerSaveAsDialog d( wmsLayer->dataProvider(),  mIface->mapCanvas()->extent() );
    d.hideFormat();
    d.hideOutput();
    if ( d.exec() == QDialog::Accepted )
    {
      QgsRasterFileWriter fileWriter( filePath /*d.outputFileName()*/ );
      if ( d.tileMode() )
      {
        fileWriter.setTiledMode( true );
        fileWriter.setMaxTileWidth( d.maximumTileSizeX() );
        fileWriter.setMaxTileHeight( d.maximumTileSizeY() );
      }

      QProgressDialog pd( 0, tr( "Abort..." ), 0, 0 );
      pd.setWindowModality( Qt::WindowModal );
      fileWriter.writeRaster( wmsLayer->dataProvider(), d.nColumns(), d.outputRectangle(), &pd );

      filePath += ( "/" + layerId + ".vrt" );
      offlineOk = true;
      if ( layerInMap )
      {
        QgsRasterLayer* offlineLayer = mIface->addRasterLayer( filePath, layername );
        exchangeLayer( item->data( 3, Qt::UserRole ).toString(), offlineLayer );
        item->setData( 3, Qt::UserRole, offlineLayer->id() );
      }
      else
      {
        delete wmsLayer;
      }
    }
  }

  if ( offlineOk )
  {
    item->setText( 2, "offline" );
    item->setIcon( 2, QIcon( ":/niwa/icons/offline.png" ) );
    item->setData( 2, Qt::UserRole, filePath );
    mChangeOfflineButton->setEnabled( false );
    mChangeOnlineButton->setEnabled( true );
    mReloadButton->setEnabled( true );
  }
#endif //0
}

#if 0
#include "addservicedialog.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslegendinterface.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterlayersaveasdialog.h"
#include "qgsvectorfilewriter.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include <QDomDocument>
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QSettings>

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";

WebDataDialog::WebDataDialog( QgisInterface* iface, QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ), mCapabilitiesReply( 0 ), mIface( iface ),
    mNIWAServicesRequestFinished( false )
{
  setupUi( this );
  insertServices();
  loadFromSettings();
  //create cache layer directory if not already there
  QDir cacheDirectory = QDir( QgsApplication::qgisSettingsDirPath() + "/cachelayers" );
  if ( !cacheDirectory.exists() )
  {
    cacheDirectory.mkpath( QgsApplication::qgisSettingsDirPath() + "/cachelayers" );
  }
}

WebDataDialog::~WebDataDialog()
{
  saveToSettings();
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

void WebDataDialog::on_mAddPushButton_clicked()
{
  AddServiceDialog d( this );
  if ( d.exec() == QDialog::Accepted )
  {
    setServiceSetting( d.name(), d.service(), d.url() );
  }
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

void WebDataDialog::on_mConnectPushButton_clicked()
{
  QString url = serviceURLFromComboBox();
  url.append( "REQUEST=GetCapabilities&SERVICE=" );

  int currentIndex = mServicesComboBox->currentIndex();
  if ( currentIndex == -1 )
  {
    return;
  }
  QString serviceType = mServicesComboBox->itemData( currentIndex ).toString();
  url.append( serviceType );
  if ( serviceType == "WFS" )
  {
    url.append( "&VERSION=1.0.0" );
  }

  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

  if ( serviceType == "WFS" )
  {
    connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( wfsCapabilitiesRequestFinished() ) );
  }
  else if ( serviceType == "WMS" )
  {
    connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( wmsCapabilitiesRequestFinished() ) );
  }
  else if ( serviceType == "WCS" )
  {
    //todo...
  }
  connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleDownloadProgress( qint64, qint64 ) ) );
}

void WebDataDialog::on_mAddLayerToListButton_clicked()
{
  QTreeWidgetItem* cItem = mDatasourceLayersTreeWidget->currentItem();
  if ( !cItem )
  {
    return;
  }
  QString name = cItem->text( 0 );
  QString type = cItem->text( 2 );
  QString abstract = cItem->text( 3 );
  QString crs = cItem->text( 4 );
  QString style = cItem->text( 5 );
  QString version = cItem->data( 2, Qt::UserRole ).toString();
  QString formatList = cItem->text( 6 );

  QTreeWidgetItem* newItem = new QTreeWidgetItem();
  newItem->setText( 0, name );
  newItem->setToolTip( 0, name );
  newItem->setText( 1, type );
  newItem->setText( 2, tr( "online" ) );
  newItem->setIcon( 2, QIcon( ":/niwa/icons/online.png" ) );
  newItem->setCheckState( 3, Qt::Unchecked );
  newItem->setText( 4, abstract );
  newItem->setToolTip( 4, abstract );
  newItem->setText( 5, crs );
  newItem->setToolTip( 5, crs );
  newItem->setText( 6, style );
  newItem->setToolTip( 6, style );
  newItem->setText( 7, formatList );
  newItem->setToolTip( 7, formatList );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  //add url as user data to name column
  QString serviceURL = serviceURLFromComboBox();

  newItem->setData( 0, Qt::UserRole, serviceURL );
  newItem->setData( 1, Qt::UserRole, version );

  mLayerTreeWidget->addTopLevelItem( newItem );
  mLayerTreeWidget->setCurrentItem( newItem );
  mChangeOfflineButton->setEnabled( true );
  mChangeOnlineButton->setEnabled( false );
  mAddToMapButton->setEnabled( true );
  mRemoveFromMapButton->setEnabled( false );
}

void WebDataDialog::on_mAddToMapButton_clicked()
{
  if ( !mIface )
  {
    return;
  }

  QTreeWidgetItem* item = mLayerTreeWidget->currentItem();
  if ( !item )
  {
    return;
  }

  //WMS/WFS?
  QString serviceType = item->text( 1 );
  QString url = item->data( 0, Qt::UserRole ).toString();
  QString layerName = item->text( 0 );
  bool online = ( item->text( 2 ) == tr( "online" ) );

  //online / offline
  if ( serviceType == "WFS" )
  {
    QString requestURL = wfsUrlFromLayerItem( item );
    QgsVectorLayer* vl = 0;
    if ( online )
    {
      vl = mIface->addVectorLayer( requestURL, layerName, "WFS" );
    }
    else
    {
      QString filePath = item->data( 2, Qt::UserRole ).toString();
      vl = mIface->addVectorLayer( item->data( 2, Qt::UserRole ).toString(), layerName, "ogr" );
    }

    if ( !vl )
    {
      return;
    }
    item->setData( 3, Qt::UserRole, vl->id() );
  }
  else if ( serviceType == "WMS" )
  {
    QgsRasterLayer* rl = 0;
    if ( online )
    {
      //get preferred style, crs, format
      QString url, format, crs;
      QString providerKey = "wms";
      QStringList layers, styles;
      wmsParameterFromItem( item, url, format, crs, layers, styles );

      //add to map
      rl = mIface->addRasterLayer( url, item->text( 0 ), providerKey, layers, styles, format, crs );
    }
    else
    {
      rl = mIface->addRasterLayer( item->data( 2, Qt::UserRole ).toString(), layerName );
      rl->setRedBandName( rl->bandName( 1 ) );
      rl->setGreenBandName( rl->bandName( 2 ) );
      rl->setBlueBandName( rl->bandName( 3 ) );
      rl->setTransparentBandName( rl->bandName( 4 ) );
    }

    if ( !rl )
    {
      return;
    }
    item->setData( 3, Qt::UserRole, rl->id() );
  }

  item->setCheckState( 3, Qt::Checked );
  mAddToMapButton->setEnabled( false );
  mRemoveFromMapButton->setEnabled( true );
}

void WebDataDialog::on_mRemoveFromMapButton_clicked()
{
  QTreeWidgetItem* item = mLayerTreeWidget->currentItem();
  if ( !item )
  {
    return;
  }

  QString layerId = item->data( 3, Qt::UserRole ).toString();
  if ( layerId.isEmpty() )
  {
    return;
  }

  QgsMapLayerRegistry::instance()->removeMapLayers( QStringList() << layerId );
  item->setCheckState( 3, Qt::Unchecked );
  item->setData( 3, Qt::UserRole, "" );
  mAddToMapButton->setEnabled( true );
  mRemoveFromMapButton->setEnabled( false );
}



void WebDataDialog::deleteOfflineDatasource( const QString& serviceType, const QString& offlinePath )
{
  if ( serviceType == "WFS" )
  {
    QgsVectorFileWriter::deleteShapeFile( offlinePath );
  }
  else if ( serviceType == "WMS" )
  {
    //remove directory and content
    QDir rasterFileDir( offlinePath );
    QFileInfoList rasterFileList = rasterFileDir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot );
    QFileInfoList::iterator it = rasterFileList.begin();
    for ( ; it != rasterFileList.end(); ++it )
    {
      QFile::remove( it->absoluteFilePath() );
    }
    rasterFileDir.rmdir( offlinePath );
  }
}

void WebDataDialog::on_mLayerTreeWidget_currentItemChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous )
{
  Q_UNUSED( previous )
  if ( !current )
  {
    return;
  }

  bool inMap = false;
  if ( current->checkState( 3 ) == Qt::Checked )
  {
    inMap = true;
  }
  mAddToMapButton->setEnabled( !inMap );
  mRemoveFromMapButton->setEnabled( inMap );

  bool online = false;
  if ( current->text( 2 ) == tr( "online" ) )
  {
    online = true;
  }
  mChangeOfflineButton->setEnabled( online );
  mChangeOnlineButton->setEnabled( !online );
  mReloadButton->setEnabled( !online );
}

void WebDataDialog::on_mRemoveFromListButton_clicked()
{
  QTreeWidgetItem* item = mLayerTreeWidget->currentItem();
  if ( !item )
  {
    return;
  }

  bool online = false;
  if ( item->text( 2 ) == tr( "online" ) )
  {
    online = true;
  }

  if ( !online ) //if offline: delete datasource
  {
    QString serviceType = item->text( 1 );
    QString offlinePath = item->data( 2, Qt::UserRole ).toString();
    deleteOfflineDatasource( serviceType, offlinePath );
  }
  delete( item );
}

void WebDataDialog::on_mReloadButton_clicked()
{
  //online: no action required
  QTreeWidgetItem* item = mLayerTreeWidget->currentItem();
  if ( !item )
  {
    return;
  }

  if ( item->text( 2 ) == tr( "online" ) )
  {
    return;
  }

  //offline: store old dataset
  QString saveFilePath = QgsApplication::qgisSettingsDirPath() + "/cachelayers/";
  QString serviceType = item->text( 1 );
  QString oldDataset = item->data( 2, Qt::UserRole ).toString();
  QString oldLayerId = item->data( 3, Qt::UserRole ).toString();
  QString layerName = item->text( 0 );
  QDateTime dt = QDateTime::currentDateTime();
  QString layerId = layerName + dt.toString( "yyyyMMddhhmmsszzz" );
  bool offlineOk = false;
  QString filePath;

  //create online dataset (WFS/WMS) and save to new source
  if ( serviceType == "WFS" )
  {
    QString wfsUrl = wfsUrlFromLayerItem( item );
    QgsVectorLayer* wfsLayer = new QgsVectorLayer( wfsUrl, layerName, "WFS" );
    filePath = saveFilePath + layerId + ".shp";
    const QgsCoordinateReferenceSystem& layerCRS = wfsLayer->crs();
    offlineOk = ( QgsVectorFileWriter::writeAsVectorFormat( wfsLayer, filePath,
                  "UTF-8", &layerCRS, "ESRI Shapefile" ) == QgsVectorFileWriter::NoError );
    if ( offlineOk )
    {
      QgsVectorLayer* offlineLayer = mIface->addVectorLayer( filePath, layerName, "ogr" );
      exchangeLayer( oldLayerId, offlineLayer );
      item->setData( 3, Qt::UserRole, offlineLayer->id() );

      //delete old files
      QgsVectorFileWriter::deleteShapeFile( oldDataset );
    }
    delete wfsLayer;
  }
  else if ( serviceType == "WMS" )
  {
    //get preferred style, crs, format
    QString url, format, crs;
    QString providerKey = "wms";
    QStringList layers, styles;
    wmsParameterFromItem( item, url, format, crs, layers, styles );
    QgsRasterLayer* wmsLayer = new QgsRasterLayer( 0, url, layerName, "wms", layers, styles, format, crs );

    //call save as dialog
    filePath = saveFilePath + "/" + layerId;
    QgsRasterLayerSaveAsDialog d( wmsLayer->dataProvider(),  mIface->mapCanvas()->extent() );
    d.hideFormat();
    d.hideOutput();
    if ( d.exec() == QDialog::Accepted )
    {
      QgsRasterFileWriter fileWriter( filePath );
      if ( d.tileMode() )
      {
        fileWriter.setTiledMode( true );
        fileWriter.setMaxTileWidth( d.maximumTileSizeX() );
        fileWriter.setMaxTileHeight( d.maximumTileSizeY() );
      }

      QProgressDialog pd( 0, tr( "Abort..." ), 0, 0 );
      pd.setWindowModality( Qt::WindowModal );
      fileWriter.writeRaster( wmsLayer->dataProvider(), d.nColumns(), d.outputRectangle(), &pd );

      filePath += ( "/" + layerId + ".vrt" );
      offlineOk = true;
      QgsRasterLayer* offlineLayer = mIface->addRasterLayer( filePath, layerName );
      exchangeLayer( item->data( 3, Qt::UserRole ).toString(), offlineLayer );
      item->setData( 3, Qt::UserRole, offlineLayer->id() );
      delete wmsLayer;

      //delete old files
      //remove directory and content
      QFileInfo oldDatasetInfo( oldDataset );
      QDir rasterFileDir = oldDatasetInfo.dir();
      QFileInfoList rasterFileList = rasterFileDir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot );
      QFileInfoList::iterator it = rasterFileList.begin();
      for ( ; it != rasterFileList.end(); ++it )
      {
        QFile::remove( it->absoluteFilePath() );
      }
      rasterFileDir.rmdir( rasterFileDir.absolutePath() );
    }
  }

  item->setData( 2, Qt::UserRole, filePath );
}

void WebDataDialog::NIWAServicesRequestFinished()
{
  mNIWAServicesRequestFinished = true;
}

void WebDataDialog::wfsCapabilitiesRequestFinished()
{
  if ( mCapabilitiesReply->error() != QNetworkReply::NoError )
  {
    QMessageBox::critical( 0, tr( "Error" ), tr( "Capabilities could not be retrieved from the server" ) );
    return;
  }

  mDatasourceLayersTreeWidget->clear();

  QByteArray buffer = mCapabilitiesReply->readAll();
  //QString capabilitiesString( buffer );
  //qWarning( capabilitiesString.toLocal8Bit().data() );

  QString capabilitiesDocError;
  QDomDocument capabilitiesDocument;
  if ( !capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
  {
    QMessageBox::critical( 0, tr( "Error parsing capabilities document" ), capabilitiesDocError );
    return;
  }

  QDomNodeList featureTypeList = capabilitiesDocument.elementsByTagNameNS( WFS_NAMESPACE, "FeatureType" );
  for ( unsigned int i = 0; i < featureTypeList.length(); ++i )
  {
    QString name, title, abstract, srs;
    QDomElement featureTypeElem = featureTypeList.at( i ).toElement();

    //Name
    QDomNodeList nameList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Name" );
    if ( nameList.length() > 0 )
    {
      name = nameList.at( 0 ).toElement().text();
    }
    //Title
    QDomNodeList titleList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Title" );
    if ( titleList.length() > 0 )
    {
      title = titleList.at( 0 ).toElement().text();
    }
    //Abstract
    QDomNodeList abstractList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Abstract" );
    if ( abstractList.length() > 0 )
    {
      abstract = abstractList.at( 0 ).toElement().text();
    }
    //SRS
    QDomNodeList srsList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "SRS" );
    if ( srsList.length() > 0 )
    {
      srs = srsList.at( 0 ).toElement().text();
    }

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText( 0, name );
    item->setToolTip( 0, name );
    item->setText( 1, title );
    item->setText( 2, "WFS" );
    item->setText( 3, abstract );
    item->setToolTip( 3, abstract );
    item->setText( 4, srs );
    item->setToolTip( 4, srs );
    mDatasourceLayersTreeWidget->addTopLevelItem( item );
  }
  mStatusLabel->setText( tr( "Ready" ) );
}

void WebDataDialog::wmsCapabilitiesRequestFinished()
{
  if ( mCapabilitiesReply->error() != QNetworkReply::NoError )
  {
    QMessageBox::critical( 0, tr( "Error" ), tr( "Capabilities could not be retrieved from the server" ) );
    return;
  }

  mDatasourceLayersTreeWidget->clear();

  QByteArray buffer = mCapabilitiesReply->readAll();

  QString capabilitiesDocError;
  QDomDocument capabilitiesDocument;
  if ( !capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
  {
    QMessageBox::critical( 0, tr( "Error parsing capabilities document" ), capabilitiesDocError );
    return;
  }

  QString version = capabilitiesDocument.documentElement().attribute( "version" );

  //get format list for GetMap
  QString formatList;
  QDomElement capabilityElem = capabilitiesDocument.documentElement().firstChildElement( "Capability" );
  QDomElement requestElem = capabilityElem.firstChildElement( "Request" );
  QDomElement getMapElem = requestElem.firstChildElement( "GetMap" );
  QDomNodeList formatNodeList = getMapElem.elementsByTagName( "Format" );
  for ( int i = 0; i < formatNodeList.size(); ++i )
  {
    if ( i > 0 )
    {
      formatList.append( "," );
    }
    formatList.append( formatNodeList.at( i ).toElement().text() );
  }

  //get name, title, abstract of all layers (style / CRS? )
  QDomNodeList layerList = capabilitiesDocument.elementsByTagName( "Layer" );
  for ( unsigned int i = 0; i < layerList.length(); ++i )
  {
    QString name, title, abstract, crs, style;
    QDomElement layerElem = layerList.at( i ).toElement();
    QDomNodeList nameList = layerElem.elementsByTagName( "Name" );
    if ( nameList.size() > 0 )
    {
      name = nameList.at( 0 ).toElement().text();
    }
    QDomNodeList titleList = layerElem.elementsByTagName( "Title" );
    if ( titleList.size() > 0 )
    {
      title = titleList.at( 0 ).toElement().text();
    }
    QDomNodeList abstractList = layerElem.elementsByTagName( "Abstract" );
    if ( abstractList.size() > 0 )
    {
      abstract = abstractList.at( 0 ).toElement().text();
    }

    //CRS in WMS 1.3
    QDomNodeList crsList = layerElem.elementsByTagName( "CRS" );
    for ( int i = 0; i < crsList.size(); ++i )
    {
      QString crsName = crsList.at( i ).toElement().text();
      if ( i > 0 )
      {
        crsName.prepend( "," );
      }
      crs.append( crsName );
    }
    //SRS in WMS 1.1.1
    QDomNodeList srsList = layerElem.elementsByTagName( "SRS" );
    for ( int i = 0; i < srsList.size(); ++i )
    {
      QString srsName = srsList.at( i ).toElement().text();
      if ( i > 0 )
      {
        srsName.prepend( "," );
      }
      crs.append( srsName );
    }
    QDomNodeList styleList = layerElem.elementsByTagName( "Style" );
    for ( int i = 0; i < styleList.size(); ++i )
    {
      QString styleName = styleList.at( i ).toElement().firstChildElement( "Name" ).text();
      if ( i > 0 )
      {
        styleName.prepend( "," );
      }
      style.append( styleName );
    }
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText( 0, name );
    item->setToolTip( 0, name );
    item->setText( 1, title );
    item->setText( 2, "WMS" );
    item->setData( 2, Qt::UserRole, version );
    item->setText( 3, abstract );
    item->setToolTip( 3, abstract );
    item->setText( 4, crs );
    item->setToolTip( 4, crs );
    item->setText( 5, style );
    item->setToolTip( 5, style );
    item->setText( 6, formatList );
    item->setToolTip( 6, formatList );
    mDatasourceLayersTreeWidget->addTopLevelItem( item );
  }
  mStatusLabel->setText( tr( "Ready" ) );
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

QString WebDataDialog::wfsUrlFromLayerItem( QTreeWidgetItem* item ) const
{
  QString wfsUrl;
  if ( !item )
  {
    return wfsUrl;
  }

  //WMS/WFS?
  QString serviceType = item->text( 1 );
  QString url = item->data( 0, Qt::UserRole ).toString();
  QString layerName = item->text( 0 );
  QString srs = item->text( 5 );

  wfsUrl = url + "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" + layerName;
  if ( !srs.isEmpty() )
  {
    wfsUrl.append( "&SRSNAME=" + srs );
  }
  return wfsUrl;
}

void WebDataDialog::wmsParameterFromItem( QTreeWidgetItem* item, QString& url, QString& format, QString& crs, QStringList& layers, QStringList& styles ) const
{
  if ( !item )
  {
    return;
  }

  url = item->data( 0, Qt::UserRole ).toString();
  url.prepend( "ignoreUrl=GetMap;GetFeatureInfo,url=" ); //ignore advertised urls per default

  //format: prefer png
  QStringList formatList = item->text( 7 ).split( "," );
  if ( formatList.size() > 0 )
  {
    if ( formatList.contains( "image/png" ) )
    {
      format = "image/png";
    }
    else
    {
      format = formatList.at( 0 );
    }
  }

  //CRS: prefer map crs
  QStringList crsList = item->text( 5 ).split( "," );
  if ( crsList.size() > 0 )
  {
    crs = crsList.at( 0 );
    QgsMapCanvas* canvas = mIface->mapCanvas();
    if ( canvas )
    {
      QgsMapRenderer* renderer = canvas->mapRenderer();
      if ( renderer )
      {
        const QgsCoordinateReferenceSystem& destCRS = renderer->destinationCrs();
        QString authId = destCRS.authid();
        if ( crsList.contains( authId ) )
        {
          crs = authId;
        }
      }
    }

  }

  layers.append( item->text( 0 ) );

  //take first style
  QString stylesString = item->text( 6 );
  if ( stylesString.isEmpty() )
  {
    styles.append( "" );
  }
  else
  {
    styles.append( stylesString.split( "," ).at( 0 ) );
  }
}

bool WebDataDialog::exchangeLayer( const QString& layerId, QgsMapLayer* newLayer )
{
  if ( !mIface )
  {
    return false;
  }

  QgsMapLayer* oldLayer = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  if ( !oldLayer || !newLayer )
  {
    return false;
  }

  if ( newLayer->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* newVectorLayer = static_cast<QgsVectorLayer*>( newLayer );
    QgsVectorLayer* oldVectorLayer = static_cast<QgsVectorLayer*>( oldLayer );

    //write old style
    QDomDocument vectorQMLDoc;
    QDomElement qmlRootElem = vectorQMLDoc.createElement( "qgis" );
    vectorQMLDoc.appendChild( qmlRootElem );
    QString errorMessage;
    if ( !oldVectorLayer->writeSymbology( qmlRootElem, vectorQMLDoc, errorMessage ) )
    {
      return false;
    }

    //set old style to new layer
    if ( !newVectorLayer->readSymbology( qmlRootElem, errorMessage ) )
    {
      return false;
    }
  }
  else if ( newLayer->type() == QgsMapLayer::RasterLayer )
  {
    //set Red/Green/Blue/Alpha channels
    QgsRasterLayer* newRasterLayer = static_cast<QgsRasterLayer*>( newLayer );
    if ( newRasterLayer->dataProvider() && newRasterLayer->dataProvider()->name() != "wms" )
    {
      newRasterLayer->setRedBandName( newRasterLayer->bandName( 1 ) );
      newRasterLayer->setGreenBandName( newRasterLayer->bandName( 2 ) );
      newRasterLayer->setBlueBandName( newRasterLayer->bandName( 3 ) );
      newRasterLayer->setTransparentBandName( newRasterLayer->bandName( 4 ) );
    }
  }

  //move new layer next to the old one
  //remove old layer
  QgsLegendInterface* legendIface = mIface->legendInterface();
  if ( legendIface )
  {
    legendIface->moveLayer( newLayer, oldLayer );
    legendIface->refreshLayerSymbology( newLayer );
  }

  QgsMapLayerRegistry::instance()->removeMapLayers( QStringList() << layerId );

  return false;
}

void WebDataDialog::loadFromSettings()
{
  QSettings s;
  QStringList layerNameList = s.value( "NIWA/layerNameList" ).toStringList();
  QStringList serviceTypeList = s.value( "NIWA/serviceTypeList" ).toStringList();
  QStringList onlineList = s.value( "NIWA/onlineList" ).toStringList();
  QStringList inMapList = s.value( "NIWA/inMapList" ).toStringList();
  QStringList abstractList = s.value( "NIWA/abstractList" ).toStringList();
  QStringList crsList = s.value( "NIWA/crsList" ).toStringList();
  QStringList styleList = s.value( "NIWA/styleList" ).toStringList();
  QStringList formatList = s.value( "NIWA/formatList" ).toStringList();
  QStringList urlList = s.value( "NIWA/urlList" ).toStringList();
  QStringList filenameList = s.value( "NIWA/filenameList" ).toStringList();
  QStringList layerIdList = s.value( "NIWA/layerIdList" ).toStringList();

  mLayerTreeWidget->clear();
  for ( int i = 0; i < layerNameList.size(); ++i )
  {
    QTreeWidgetItem* newItem = new QTreeWidgetItem();
    newItem->setText( 0, layerNameList.at( i ) );
    newItem->setText( 1, serviceTypeList.at( i ) );
    newItem->setText( 2, onlineList.at( i ) );
    if ( newItem->text( 2 ) == tr( "online" ) )
    {
      newItem->setIcon( 2, QIcon( ":/niwa/icons/online.png" ) );
    }
    else
    {
      newItem->setIcon( 2, QIcon( ":/niwa/icons/offline.png" ) );
    }

    newItem->setData( 0, Qt::UserRole, urlList.at( i ) );
    newItem->setData( 2, Qt::UserRole, filenameList.at( i ) );

    //find out if layer is in map
    QString layerTestUrl;
    if ( newItem->text( 2 ) == tr( "online" ) )
    {
      layerTestUrl = newItem->data( 0, Qt::UserRole ).toString();
    }
    else
    {
      layerTestUrl = newItem->data( 2, Qt::UserRole ).toString();
    }
    QString layerId = layerIdFromUrl( layerTestUrl,  serviceTypeList.at( i ), newItem->text( 2 ) == tr( "online" ), layerNameList.at( i ) );
    if ( layerId.isEmpty() )
    {
      newItem->setCheckState( 3, Qt::Unchecked );
    }
    else
    {
      newItem->setCheckState( 3, Qt::Checked );
      newItem->setData( 3, Qt::UserRole, layerId );
    }

    newItem->setText( 4, abstractList.at( i ) );
    newItem->setText( 5, crsList.at( i ) );
    newItem->setText( 6, styleList.at( i ) );
    newItem->setText( 7, formatList.at( i ) );
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mLayerTreeWidget->addTopLevelItem( newItem );
  }
}

void WebDataDialog::saveToSettings()
{
  QSettings s;
  QStringList layerNameList, serviceTypeList, onlineList, inMapList, abstractList, crsList, styleList, formatList,
  urlList, filenameList, layerIdList;

  QTreeWidgetItem* currentItem = 0;
  for ( int i = 0; i < mLayerTreeWidget->topLevelItemCount(); ++i )
  {
    currentItem = mLayerTreeWidget->topLevelItem( i );
    layerNameList.append( currentItem->text( 0 ) );
    serviceTypeList.append( currentItem->text( 1 ) );
    onlineList.append( currentItem->text( 2 ) );
    if ( currentItem->checkState( 3 ) == Qt::Checked )
    {
      inMapList.append( "t" );
    }
    else
    {
      inMapList.append( "f" );
    }
    abstractList.append( currentItem->text( 4 ) );
    crsList.append( currentItem->text( 5 ) );
    styleList.append( currentItem->text( 6 ) );
    formatList.append( currentItem->text( 7 ) );
    urlList.append( currentItem->data( 0, Qt::UserRole ).toString() );
    filenameList.append( currentItem->data( 2, Qt::UserRole ).toString() );
    layerIdList.append( currentItem->data( 3, Qt::UserRole ).toString() );
  }

  s.setValue( "NIWA/layerNameList", layerNameList );
  s.setValue( "NIWA/serviceTypeList", serviceTypeList );
  s.setValue( "NIWA/onlineList", onlineList );
  s.setValue( "NIWA/inMapList", inMapList );
  s.setValue( "NIWA/abstractList", abstractList );
  s.setValue( "NIWA/crsList", crsList );
  s.setValue( "NIWA/styleList", styleList );
  s.setValue( "NIWA/formatList", formatList );
  s.setValue( "NIWA/urlList", urlList );
  s.setValue( "NIWA/filenameList", filenameList );
  s.setValue( "NIWA/layerIdList", layerIdList );
}

QString WebDataDialog::layerIdFromUrl( const QString& url, const QString& serviceType, bool online, QString layerName )
{
  const QMap<QString, QgsMapLayer*>& layerMap = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::const_iterator layerIt = layerMap.constBegin();
  for ( ; layerIt != layerMap.constEnd(); ++layerIt )
  {
    const QgsMapLayer* layer = layerIt.value();
    if ( !online )
    {
      if ( layer && QFileInfo( layer->source() ) == QFileInfo( url ) )
      {
        return layer->id();
      }
    }
    else if ( serviceType == "WFS" )
    {
      QString layerSource = layer->source();
      QString layerUrl = url;
      if ( layerSource.startsWith( layerUrl )
           && layerSource.contains( "TYPENAME=" + layerName, Qt::CaseInsensitive ) )
      {
        return layer->id();
      }
    }
    else //for online WMS, we need to additionally consider the layer name
    {
      QString testUrl = url;
      testUrl.chop( 1 );
      if ( layer->source().contains( testUrl ) ) //sometimes url contains '?' or '&' at the end
      {
        const QgsRasterLayer* rlayer = dynamic_cast<const QgsRasterLayer*>( layer );
        if ( rlayer )
        {
          if ( rlayer->layers().join( "" ) == layerName )
          {
            return layer->id();
          }
        }
      }
    }
  }
  return QString();
}
#endif //0
