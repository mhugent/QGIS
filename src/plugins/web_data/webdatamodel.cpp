#include "webdatamodel.h"
#include "qgsnetworkaccessmanager.h"
#include <QDomDocument>
#include <QDomElement>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTreeWidgetItem>

WebDataModel::WebDataModel(): QStandardItemModel(), mCapabilitiesReply( 0 )
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
  QStandardItem* wmsTitleItem = new QStandardItem( mCapabilitiesReply->property( "title" ).toString() );
  wmsTitleItem->setFlags( Qt::ItemIsEnabled );
  invisibleRootItem()->setChild( invisibleRootItem()->rowCount(), wmsTitleItem );

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
