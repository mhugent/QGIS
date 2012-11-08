#include "webdatamodel.h"
#include "qgisinterface.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsnetworkaccessmanager.h"
#include <QDomDocument>
#include <QDomElement>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTreeWidgetItem>

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
}

WebDataModel::~WebDataModel()
{
}

void WebDataModel::addService( const QString& title, const QString& url, const QString& service )
{
  QString requestUrl = url;
  requestUrl.append( "REQUEST=GetCapabilities&SERVICE=" );
  requestUrl.append( service );
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
  qWarning( buffer.data() );

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

void WebDataModel::addEntryToMap( const QModelIndex& index )
{
  if ( !mIface )
  {
    return;
  }

  QString layerName;
  QStandardItem* nameItem = itemFromIndex( index );
  if ( nameItem )
  {
    layerName = nameItem->text();
  }

  //is entry already in map
  QStandardItem* inMapItem = itemFromIndex( index.sibling( index.row(), 3 ) );
  bool inMap = ( inMapItem && inMapItem->checkState() == Qt::Checked );
  if ( inMap )
  {
    return;
  }

  //wms / wfs ?
  QString type;
  QStandardItem* typeItem = itemFromIndex( index.sibling( index.row(), 2 ) );
  if ( typeItem )
  {
    type = typeItem->text();
  }

  if ( type == "WMS" )
  {
    QString url, format, crs;
    QStringList layers, styles;
    wmsParameterFromIndex( index, url, format, crs, layers, styles );
    mIface->addRasterLayer( url, layerName, "wms", layers, styles, format, crs );
  }

  //wmsParameterFromIndex

  //
}

void WebDataModel::removeEntryFromMap( const QModelIndex& index )
{

}

void WebDataModel::changeEntryToOffline( const QModelIndex& index )
{

}

void WebDataModel::changeEntryToOnline( const QModelIndex& index )
{

}

QString WebDataModel::wfsUrlFromLayerIndex( const QModelIndex& index ) const
{
  return QString( "" ); //soon...
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


