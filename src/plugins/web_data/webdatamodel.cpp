#include "webdatamodel.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslegendinterface.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayersaveasdialog.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include <QDomDocument>
#include <QDomElement>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QTreeWidgetItem>

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";

WebDataModel::WebDataModel( QgisInterface* iface ): QStandardItemModel(), mCapabilitiesReply( 0 ), mIface( iface )
{
  QStringList headerLabels;
  headerLabels << tr( "Name" );
  headerLabels << tr( "Favorite" );
  headerLabels << tr( "Type" );
  headerLabels << tr( "In map" );
  headerLabels << tr( "Status" );
  headerLabels << tr( "CRS" );
  headerLabels << tr( "Formats" );
  headerLabels << tr( "Styles" );
  setHorizontalHeaderLabels( headerLabels );

  connect( this, SIGNAL( itemChanged( QStandardItem* ) ), this, SLOT( handleItemChange( QStandardItem* ) ) );

  //create cache layer directory if not already there
  QDir cacheDirectory = QDir( QgsApplication::qgisSettingsDirPath() + "/cachelayers" );
  if ( !cacheDirectory.exists() )
  {
    cacheDirectory.mkpath( QgsApplication::qgisSettingsDirPath() + "/cachelayers" );
  }

  loadFromXML();
}

WebDataModel::~WebDataModel()
{
  saveToXML();
}

void WebDataModel::addService( const QString& title, const QString& url, const QString& service )
{
  QString requestUrl = url;
  requestUrl.append( "REQUEST=GetCapabilities&SERVICE=" );
  requestUrl.append( service );
  if ( service.compare( "WFS", Qt::CaseInsensitive ) == 0 )
  {
    requestUrl.append( "&VERSION=1.0.0" );
  }
  QNetworkRequest request( requestUrl );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
  mCapabilitiesReply->setProperty( "title", title );
  mCapabilitiesReply->setProperty( "url", url );

  if ( service.compare( "WMS", Qt::CaseInsensitive ) == 0 )
  {
    connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( wmsCapabilitiesRequestFinished() ) );
  }
  else if ( service.compare( "WFS", Qt::CaseInsensitive ) == 0 )
  {
    connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( wfsCapabilitiesRequestFinished() ) );
  }
}

void WebDataModel::wmsCapabilitiesRequestFinished()
{
  if ( !mCapabilitiesReply )
  {
    return;
  }

  if ( mCapabilitiesReply->error() != QNetworkReply::NoError )
  {
    //QMessageBox::critical( 0, tr( "Error" ), tr( "Capabilities could not be retrieved from the server" ) );
    return;
  }

  QByteArray buffer = mCapabilitiesReply->readAll();

  QString capabilitiesDocError;
  QDomDocument capabilitiesDocument;
  if ( !capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
  {
    //QMessageBox::critical( 0, tr( "Error parsing capabilities document" ), capabilitiesDocError );
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
  if ( layerList.size() < 1 )
  {
    return;
  }

  //add parentItem

  QString serviceTitle = mCapabilitiesReply->property( "title" ).toString();
  QList<QStandardItem*> serviceTitleItems = findItems( serviceTitle );
  QStandardItem* wmsTitleItem = 0;
  if ( serviceTitleItems.size() < 1 )
  {
    wmsTitleItem = new QStandardItem( serviceTitle );
    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), wmsTitleItem );
  }
  else
  {
    wmsTitleItem = serviceTitleItems.at( 0 );
    wmsTitleItem->removeRows( 0, wmsTitleItem->rowCount() );
  }
  wmsTitleItem->setFlags( Qt::ItemIsEnabled );

  QString url = mCapabilitiesReply->property( "url" ).toString();

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

    QList<QStandardItem*> childItemList;
    //name
    QStandardItem* nameItem = new QStandardItem( name );
    nameItem->setData( url );
    childItemList.push_back( nameItem );
    //favorite
    QStandardItem* favoriteItem = new QStandardItem();
    favoriteItem->setCheckable( true );
    favoriteItem->setCheckState( Qt::Unchecked );
    childItemList.push_back( favoriteItem );
    //type
    QStandardItem* typeItem = new QStandardItem( "WMS" );
    childItemList.push_back( typeItem );
    //in map
    QStandardItem* inMapItem = new QStandardItem();
    inMapItem->setCheckable( true );
    inMapItem->setCheckState( Qt::Unchecked );
    childItemList.push_back( inMapItem );
    //status
    QStandardItem* statusItem = new QStandardItem( QIcon( ":/niwa/icons/online.png" ), tr( "online" ) );
    childItemList.push_back( statusItem );
    //crs
    QStandardItem* crsItem = new QStandardItem( crs );
    childItemList.push_back( crsItem );
    //formats
    QStandardItem* formatsItem = new QStandardItem( formatList );
    childItemList.push_back( formatsItem );
    //styles
    QStandardItem* stylesItem = new QStandardItem( style );
    childItemList.push_back( stylesItem );

    wmsTitleItem->appendRow( childItemList );
  }


  //mStatusLabel->setText( tr( "Ready" ) );
}

void WebDataModel::wfsCapabilitiesRequestFinished()
{
  if ( !mCapabilitiesReply )
  {
    return;
  }

  if ( mCapabilitiesReply->error() != QNetworkReply::NoError )
  {
    //QMessageBox::critical( 0, tr( "Error" ), tr( "Capabilities could not be retrieved from the server" ) );
    return;
  }

  QByteArray buffer = mCapabilitiesReply->readAll();

  QString capabilitiesDocError;
  QDomDocument capabilitiesDocument;
  if ( !capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
  {
    //QMessageBox::critical( 0, tr( "Error parsing capabilities document" ), capabilitiesDocError );
    return;
  }

  QDomNodeList featureTypeList = capabilitiesDocument.elementsByTagNameNS( WFS_NAMESPACE, "FeatureType" );
  if ( featureTypeList.size() < 1 )
  {
    return;
  }

  //add parentItem
  QString serviceTitle = mCapabilitiesReply->property( "title" ).toString();
  QList<QStandardItem*> serviceTitleItems = findItems( serviceTitle );
  QStandardItem* wfsTitleItem = 0;
  if ( serviceTitleItems.size() < 1 )
  {
    wfsTitleItem = new QStandardItem( serviceTitle );
    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), wfsTitleItem );
  }
  else
  {
    wfsTitleItem = serviceTitleItems.at( 0 );
    wfsTitleItem->removeRows( 0, wfsTitleItem->rowCount() );
  }
  wfsTitleItem->setFlags( Qt::ItemIsEnabled );
  QString url = mCapabilitiesReply->property( "url" ).toString();

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

    QList<QStandardItem*> childItemList;
    //name
    QStandardItem* nameItem = new QStandardItem( name );
    nameItem->setData( url );
    childItemList.push_back( nameItem );
    //favorite
    QStandardItem* favoriteItem = new QStandardItem();
    favoriteItem->setCheckable( true );
    favoriteItem->setCheckState( Qt::Unchecked );
    childItemList.push_back( favoriteItem );
    //type
    QStandardItem* typeItem = new QStandardItem( "WFS" );
    childItemList.push_back( typeItem );
    //in map
    QStandardItem* inMapItem = new QStandardItem();
    inMapItem->setCheckable( true );
    inMapItem->setCheckState( Qt::Unchecked );
    childItemList.push_back( inMapItem );
    //status
    QStandardItem* statusItem = new QStandardItem( QIcon( ":/niwa/icons/online.png" ), tr( "online" ) );
    childItemList.push_back( statusItem );
    //crs
    QStandardItem* srsItem = new QStandardItem( srs );
    childItemList.push_back( srsItem );
    wfsTitleItem->appendRow( childItemList );
  }
}

void WebDataModel::handleItemChange( QStandardItem* item )
{
  blockSignals( true );
  if ( item->column() == 1 )
  {
    if ( item->checkState() == Qt::Checked )
    {
      item->setIcon( QIcon( ":/niwa/icons/favourite.png" ) );
    }
    else
    {
      item->setIcon( QIcon() ); //unset icon
    }
  }
  blockSignals( false );
}

void WebDataModel::addEntryToMap( const QModelIndex& index )
{
  if ( !mIface )
  {
    return;
  }

  QString layername = layerName( index );
  QString type = serviceType( index );//wms / wfs ?
  QStandardItem* inMapItem = itemFromIndex( index.sibling( index.row(), 3 ) );
  if ( !inMapItem || !inMapItem->checkState() == Qt::Checked )
  {
    return;
  }
  bool offline = layerStatus( index ).compare( "offline", Qt::CaseInsensitive ) == 0;
  QStandardItem* statusItem = itemFromIndex( index.sibling( index.row(), 4 ) );
  if ( !statusItem )
  {
    return;
  }

  QgsMapLayer* mapLayer = 0;
  if ( type == "WMS" )
  {
    if ( offline )
    {
      mapLayer = mIface->addRasterLayer( statusItem->data().toString(), layername );
      QgsRasterLayer* rl = dynamic_cast<QgsRasterLayer*>( mapLayer );
      if ( rl )
      {
        rl->setRedBandName( rl->bandName( 1 ) );
        rl->setGreenBandName( rl->bandName( 2 ) );
        rl->setBlueBandName( rl->bandName( 3 ) );
        rl->setTransparentBandName( rl->bandName( 4 ) );
      }
    }
    else
    {
      QString url, format, crs;
      QStringList layers, styles;
      wmsParameterFromIndex( index, url, format, crs, layers, styles );
      mapLayer = mIface->addRasterLayer( url, layername, "wms", layers, styles, format, crs );
    }
  }
  else if ( type == "WFS" )
  {
    QString url = wfsUrlFromLayerIndex( index );
    if ( offline )
    {
      mapLayer =  mIface->addVectorLayer( statusItem->data().toString(), layername, "ogr" );
    }
    else
    {
      mapLayer = mIface->addVectorLayer( url, layername, "WFS" );
    }
    if ( !mapLayer )
    {
      return;
    }
  }
  inMapItem->setCheckState( Qt::Checked );

  if ( mapLayer )
  {
    inMapItem->setData( mapLayer->id() );
  }
}

void WebDataModel::removeEntryFromMap( const QModelIndex& index )
{
  //is entry already in map
  QStandardItem* inMapItem = itemFromIndex( index.sibling( index.row(), 3 ) );
  bool inMap = ( inMapItem && inMapItem->checkState() == Qt::Checked );
  if ( !inMap )
  {
    return;
  }

  QString layerId = inMapItem->data().toString();
  QgsMapLayerRegistry::instance()->removeMapLayers( QStringList() << layerId );
  inMapItem->setCheckState( Qt::Unchecked );
}

void WebDataModel::changeEntryToOffline( const QModelIndex& index )
{
  //bail out if entry already has offline status
  QString status = layerStatus( index );
  if ( status.compare( "offline", Qt::CaseInsensitive ) == 0 )
  {
    return;
  }

  //wms / wfs ?
  QString type = serviceType( index );
  QString layername = layerName( index );

  QString saveFilePath = QgsApplication::qgisSettingsDirPath() + "/cachelayers/";
  QDateTime dt = QDateTime::currentDateTime();
  QString layerId = layername + dt.toString( "yyyyMMddhhmmsszzz" );
  bool inMap = layerInMap( index );
  QString filePath;
  QStandardItem* inMapItem = itemFromIndex( index.sibling( index.row(), 3 ) );
  bool offlineOk = false;

  if ( type == "WFS" )
  {
    //create table <layername//url>
    QgsVectorLayer* wfsLayer = 0;
    if ( inMap )
    {
      if ( inMapItem )
      {
        wfsLayer = static_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( inMapItem->data().toString() ) );
      }
    }
    else
    {
      QString wfsUrl = wfsUrlFromLayerIndex( index );
      wfsLayer = new QgsVectorLayer( wfsUrl, layername, "WFS" );
    }

    filePath = saveFilePath + layerId + ".shp";
    const QgsCoordinateReferenceSystem& layerCRS = wfsLayer->crs();
    offlineOk = ( QgsVectorFileWriter::writeAsVectorFormat( wfsLayer, filePath,
                  "UTF-8", &layerCRS, "ESRI Shapefile" ) == QgsVectorFileWriter::NoError );
    if ( offlineOk && inMap )
    {
      QgsVectorLayer* offlineLayer = mIface->addVectorLayer( filePath, layername, "ogr" );
      if ( inMapItem )
      {
        exchangeLayer( inMapItem->data().toString(), offlineLayer );
        inMapItem->setData( offlineLayer->id() );
      }
    }

    if ( !inMap )
    {
      delete wfsLayer;
    }
  }
  else if ( type == "WMS" )
  {
    //get raster layer
    QgsRasterLayer* wmsLayer = 0;
    if ( inMap )
    {
      wmsLayer = static_cast<QgsRasterLayer*>( QgsMapLayerRegistry::instance()->mapLayer( inMapItem->data().toString() ) );
    }
    else
    {
      //get preferred style, crs, format
      QString url, format, crs;
      QString providerKey = "wms";
      QStringList layers, styles;
      wmsParameterFromIndex( index, url, format, crs, layers, styles );
      wmsLayer = new QgsRasterLayer( 0, url, layername, "wms", layers, styles, format, crs );
    }

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
      if ( inMap )
      {
        QgsRasterLayer* offlineLayer = mIface->addRasterLayer( filePath, layername );
        exchangeLayer( inMapItem->data().toString(), offlineLayer );
        inMapItem->setData( offlineLayer->id() );
      }
      else
      {
        delete wmsLayer;
      }
    }
  }

  if ( offlineOk )
  {
    QStandardItem* statusItem = itemFromIndex( index.sibling( index.row(), 4 ) );
    if ( statusItem )
    {
      statusItem->setText( "offline" );
      statusItem->setIcon( QIcon( ":/niwa/icons/offline.png" ) );
      statusItem->setData( filePath );
    }
  }
}

void WebDataModel::changeEntryToOnline( const QModelIndex& index )
{
  bool inMap = layerInMap( index );
  QString type = serviceType( index );
  QString layername = layerName( index );

  QStandardItem* inMapItem = itemFromIndex( index.sibling( index.row(), 3 ) );
  if ( !inMapItem )
  {
    return;
  }

  if ( inMap )
  {
    //generate new wfs/wms layer
    //exchange with layer in map
    QgsMapLayer* onlineLayer = 0;
    if ( type == "WFS" )
    {
      QString wfsUrl = wfsUrlFromLayerIndex( index );
      onlineLayer = mIface->addVectorLayer( wfsUrl, layername, "WFS" );
    }
    else if ( type == "WMS" )
    {
      //get preferred style, crs, format
      QString url, format, crs;
      QString providerKey = "wms";
      QStringList layers, styles;
      wmsParameterFromIndex( index, url, format, crs, layers, styles );

      //add to map
      onlineLayer = mIface->addRasterLayer( url, layername, providerKey, layers, styles, format, crs );
    }

    if ( onlineLayer )
    {
      exchangeLayer( inMapItem->data().toString(), onlineLayer );
      inMapItem->setData( onlineLayer->id() );
    }
  }

  QStandardItem* statusItem = itemFromIndex( index.sibling( index.row(), 4 ) );
  if ( !statusItem )
  {
    return;
  }

  QString offlineFileName = statusItem->data().toString();
  deleteOfflineDatasource( type, offlineFileName );

  statusItem->setText( "online" );
  statusItem->setIcon( QIcon( ":/niwa/icons/online.png" ) );
  statusItem->setData( "" );
}

QString WebDataModel::wfsUrlFromLayerIndex( const QModelIndex& index ) const
{
  QStandardItem* nameItem = itemFromIndex( index );
  if ( !nameItem )
  {
    return "";
  }

  QString url = nameItem->data().toString();
  QString layerName = nameItem->text();
  QStandardItem* srsItem = itemFromIndex( index.sibling( index.row(), 5 ) );
  QString srs = srsItem->text();

  url.append( "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" );
  url.append( layerName );
  if ( !srs.isEmpty() )
  {
    url.append( "&SRSNAME=" + srs );
  }
  return url;
}

void WebDataModel::wmsParameterFromIndex( const QModelIndex& index, QString& url, QString& format, QString& crs, QStringList& layers, QStringList& styles ) const
{
  QStandardItem* nameItem = itemFromIndex( index );
  if ( !nameItem )
  {
    return;
  }

  url = nameItem->data().toString();
  layers.clear();
  layers.append( nameItem->text() );
  url.prepend( "ignoreUrl=GetMap;GetFeatureInfo,url=" ); //ignore advertised urls per default

  //format: prefer png
  format.clear();
  QStandardItem* formatItem = itemFromIndex( index.sibling( index.row(), 6 ) );
  if ( formatItem )
  {
    QStringList formatList = formatItem->text().split( "," );
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
  }

  crs.clear();
  QStandardItem* crsItem = itemFromIndex( index.sibling( index.row(), 5 ) );
  if ( crsItem && mIface )
  {
    //CRS: prefer map crs
    QStringList crsList = crsItem->text().split( "," );
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
  }

  styles.clear();
  QStandardItem* stylesItem = itemFromIndex( index.sibling( index.row(), 7 ) );
  if ( stylesItem )
  {
    //take first style
    QString stylesString = stylesItem->text();
    if ( stylesString.isEmpty() )
    {
      styles.append( "" );
    }
    else
    {
      styles.append( stylesString.split( "," ).at( 0 ) );
    }
  }
}

QString WebDataModel::layerName( const QModelIndex& index ) const
{
  QStandardItem* nameItem = itemFromIndex( index );
  if ( nameItem )
  {
    return nameItem->text();
  }
  return QString();
}

QString WebDataModel::serviceType( const QModelIndex& index ) const
{
  //wms / wfs ?
  QStandardItem* typeItem = itemFromIndex( index.sibling( index.row(), 2 ) );
  if ( typeItem )
  {
    return typeItem->text();
  }
  return QString();
}

QString WebDataModel::layerStatus( const QModelIndex& index ) const
{
  QStandardItem* statusItem = itemFromIndex( index.sibling( index.row(), 4 ) );
  if ( statusItem )
  {
    return statusItem->text();
  }
  return QString();
}

bool WebDataModel::layerInMap( const QModelIndex& index ) const
{
  QStandardItem* inMapItem = itemFromIndex( index.sibling( index.row(), 3 ) );
  return ( inMapItem && inMapItem->checkState() == Qt::Checked );
}

bool WebDataModel::exchangeLayer( const QString& layerId, QgsMapLayer* newLayer )
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

void WebDataModel::deleteOfflineDatasource( const QString& serviceType, const QString& offlinePath )
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

QString WebDataModel::layerIdFromUrl( const QString& url, const QString& serviceType, bool online, QString layerName )
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

void WebDataModel::loadFromXML()
{
  QFile xmlFile( xmlFilePath() );
  if ( !xmlFile.exists() )
  {
    return;
  }

  if ( !xmlFile.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  QDomDocument doc;
  if ( !doc.setContent( &xmlFile ) )
  {
    return;
  }

  QDomNodeList serviceNodeList = doc.elementsByTagName( "service" );
  for ( int i = 0; i < serviceNodeList.size(); ++i )
  {
    QDomElement serviceElem = serviceNodeList.at( i ).toElement();
    QStandardItem* serviceItem = new QStandardItem( serviceElem.attribute( "serviceName" ) );
    invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), serviceItem );

    QDomNodeList layerNodeList = serviceElem.elementsByTagName( "layer" );
    QDomElement layerElem;
    QList<QStandardItem*> childItemList;

    for ( int j = 0; j < layerNodeList.size(); ++j )
    {
      childItemList.clear();
      layerElem = layerNodeList.at( j ).toElement();
      //name
      QString layername = layerElem.attribute( "name" );
      QStandardItem* nameItem = new QStandardItem( layername );
      nameItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      QString url = layerElem.attribute( "url" );
      nameItem->setData( url );
      childItemList.push_back( nameItem );
      //favourite
      QStandardItem* favItem = new QStandardItem();
      bool favChecked = layerElem.attribute( "favourite" ).compare( "1" ) == 0;
      favItem->setCheckState( favChecked ? Qt::Checked : Qt::Unchecked );
      favItem->setIcon( favChecked ? QIcon( ":/niwa/icons/favourite.png" ) : QIcon() );
      favItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable );
      childItemList.push_back( favItem );
      //type
      QString type = layerElem.attribute( "type" );
      QStandardItem* typeItem = new QStandardItem( type );
      typeItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      childItemList.push_back( typeItem );
      //in map
      QStandardItem* inMapItem = new QStandardItem();
      inMapItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      childItemList.push_back( inMapItem );
      //status
      QStandardItem* statusItem = new QStandardItem( layerElem.attribute( "status" ) );
      bool online = statusItem->text().compare( "online", Qt::CaseInsensitive ) == 0;
      QString filePath = layerElem.attribute( "filePath" );
      statusItem->setIcon( online ? QIcon( ":/niwa/icons/online.png" ) : QIcon( ":/niwa/icons/offline.png" ) );
      statusItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      statusItem->setData( filePath );
      childItemList.push_back( statusItem );
      if ( !online )
      {
        url = filePath;
      }
      QString layerId = layerIdFromUrl( url, type, online, layername );
      if ( !layerId.isEmpty() )
      {
        inMapItem->setCheckState( Qt::Checked );
        inMapItem->setData( layerId );
      }
      else
      {
        inMapItem->setCheckState( Qt::Unchecked );
      }
      //crs
      QStandardItem* crsItem = new QStandardItem( layerElem.attribute( "crs" ) );
      crsItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      childItemList.push_back( crsItem );
      //formats
      if ( layerElem.hasAttribute( "formats" ) )
      {
        QStandardItem* formatsItem = new QStandardItem( layerElem.attribute( "formats" ) );
        formatsItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
        childItemList.push_back( formatsItem );
      }
      //styles
      if ( layerElem.hasAttribute( "styles" ) )
      {
        QStandardItem* stylesItem = new QStandardItem( layerElem.attribute( "styles" ) );
        stylesItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
        childItemList.push_back( stylesItem );
      }
      //layers
      if ( layerElem.hasAttribute( "layers" ) )
      {
        QStandardItem* layersItem = new QStandardItem( layerElem.attribute( "layers" ) );
        layersItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
        childItemList.push_back( layersItem );
      }
      serviceItem->appendRow( childItemList );
    }
  }
}

void WebDataModel::saveToXML() const
{
  QDomDocument doc;
  QDomElement webDataElem = doc.createElement( "webdata" );
  doc.appendChild( webDataElem );

  QStandardItem* rootItem = invisibleRootItem();
  QStandardItem* serviceItem = 0;
  for ( int i = 0; i < rootItem->rowCount(); ++i )
  {
    serviceItem = rootItem->child( i );
    if ( !serviceItem )
    {
      continue;
    }
    QDomElement serviceElem = doc.createElement( "service" );
    serviceElem.setAttribute( "serviceName", serviceItem->text() );
    webDataElem.appendChild( serviceElem );

    for ( int j = 0; j < serviceItem->rowCount(); ++j )
    {
      QDomElement layerElem = doc.createElement( "layer" );
      serviceElem.appendChild( layerElem );
      //name
      QStandardItem* nameItem = serviceItem->child( j, 0 );
      if ( nameItem )
      {
        layerElem.setAttribute( "name", nameItem->text() );
        layerElem.setAttribute( "url", nameItem->data().toString() );
      }
      //favourite
      QStandardItem* favItem = serviceItem->child( j, 1 );
      if ( favItem )
      {
        layerElem.setAttribute( "favourite", ( favItem->checkState() == Qt::Checked ) ? "1" : "0" );
      }
      //type
      QStandardItem* typeItem = serviceItem->child( j, 2 );
      if ( typeItem )
      {
        layerElem.setAttribute( "type", typeItem->text() );
      }
      //in map
      QStandardItem* inMapItem = serviceItem->child( j, 3 );
      if ( inMapItem )
      {
        layerElem.setAttribute( "layerId", inMapItem->data().toString() );
      }
      //status
      QStandardItem* statusItem = serviceItem->child( j, 4 );
      if ( statusItem )
      {
        layerElem.setAttribute( "status", statusItem->text() );
        layerElem.setAttribute( "filePath", statusItem->data().toString() );
      }
      //crs
      QStandardItem* crsItem = serviceItem->child( j, 5 );
      if ( crsItem )
      {
        layerElem.setAttribute( "crs", crsItem->text() );
      }
      //formats
      QStandardItem* formatsItem = serviceItem->child( j, 6 );
      if ( formatsItem )
      {
        layerElem.setAttribute( "formats", formatsItem->text() );
      }
      //styles
      QStandardItem* stylesItem = serviceItem->child( j, 7 );
      if ( stylesItem )
      {
        layerElem.setAttribute( "styles", stylesItem->text() );
      }
      //layers
      QStandardItem* layersItem = serviceItem->child( j, 8 );
      if ( layersItem )
      {
        layerElem.setAttribute( "layers", layersItem->text() );
      }
    }

  }

  QFile outFile( xmlFilePath() );
  if ( outFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream outStream( &outFile );
    doc.save( outStream, 2 );
  }
}

QString WebDataModel::xmlFilePath() const
{
  QFileInfo fi( QgsApplication::qgisUserDbFilePath() );
  QString path = fi.absolutePath() + "/webdata.xml";
  return path;
}

