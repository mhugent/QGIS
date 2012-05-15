#include "niwaplugindialog.h"
#include "addservicedialog.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include <QDomDocument>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";
static const QString WMS_NAMESPACE = "http://www.opengis.net/wms";

NiwaPluginDialog::NiwaPluginDialog( QgisInterface* iface, QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ), mCapabilitiesReply( 0 ), mIface( iface )
{
  setupUi( this );
  insertServices();
}

NiwaPluginDialog::~NiwaPluginDialog()
{

}

void NiwaPluginDialog::insertServices()
{
  mServicesComboBox->clear();
  insertServices( "WFS" );
  insertServices( "WMS" );
}

void NiwaPluginDialog::insertServices( const QString& service )
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

void NiwaPluginDialog::on_mAddPushButton_clicked()
{
  AddServiceDialog d( this );
  if ( d.exec() == QDialog::Accepted )
  {
    //add service, url, type to QSettings
    QString name = d.name();
    QString service = d.service();
    QString url = d.url();

    if ( name.isEmpty() || url.isEmpty() || service.isEmpty() )
    {
      return;
    }

    QSettings s;
    s.setValue( "/Qgis/connections-" + service.toLower() + "/" + name + "/url", url );
    insertServices();
  }
}

void NiwaPluginDialog::on_mRemovePushButton_clicked()
{
  //todo...
}

void NiwaPluginDialog::on_mEditPushButton_clicked()
{
  //todo...
}

void NiwaPluginDialog::on_mConnectPushButton_clicked()
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

  }
}

void NiwaPluginDialog::on_mAddLayerToListButton_clicked()
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

  QTreeWidgetItem* newItem = new QTreeWidgetItem();
  newItem->setText( 0, name );
  newItem->setText( 1, type );
  newItem->setText( 2, tr( "online" ) );
  newItem->setCheckState( 3, Qt::Unchecked );
  newItem->setText( 4, abstract );
  newItem->setText( 5, crs );
  newItem->setText( 6, style );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  //add url as user data to name column
  QString serviceURL = serviceURLFromComboBox();
  newItem->setData( 0, Qt::UserRole, serviceURL );

  mLayerTreeWidget->addTopLevelItem( newItem );
}

void NiwaPluginDialog::on_mAddToMapButton_clicked()
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
  QString srs = item->text( 4 );

  //online / offline
  //assume online for now
  if ( serviceType == "WFS" )
  {
    QString requestURL = wfsUrlFromLayerItem( item );
    QgsVectorLayer* vl = mIface->addVectorLayer( requestURL, layerName, "WFS" );
    if ( !vl )
    {
      return;
    }
  }
  else if ( serviceType == "WMS" )
  {

  }

  item->setCheckState( 3, Qt::Checked );
}

void NiwaPluginDialog::on_mChangeOfflineButton_clicked()
{
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

  bool offlineOk = false;

  if ( serviceType == "WFS" )
  {
    //create table <layername//url>
    QString wfsUrl = wfsUrlFromLayerItem( item );
    QgsVectorLayer wfsLayer( wfsUrl, layername, "WFS" );
    const QgsCoordinateReferenceSystem& layerCRS = wfsLayer.crs();
    offlineOk = ( QgsVectorFileWriter::writeAsVectorFormat( &wfsLayer, saveFilePath + layerId + ".gml",
                  "UTF-8", &layerCRS, "GML" ) == QgsVectorFileWriter::NoError );
  }

  if ( offlineOk )
  {
    item->setText( 2, "offline" );
  }

  //if in map, exchange layer with the offline one
}

void NiwaPluginDialog::wfsCapabilitiesRequestFinished()
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
    item->setText( 1, title );
    item->setText( 2, "WFS" );
    item->setText( 3, abstract );
    item->setText( 4, srs );
    mDatasourceLayersTreeWidget->addTopLevelItem( item );
  }
}

void NiwaPluginDialog::wmsCapabilitiesRequestFinished()
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

  //get name, title, abstract of all layers (style / CRS? )
  QDomNodeList layerList = capabilitiesDocument.elementsByTagNameNS( WMS_NAMESPACE, "Layer" );
  for ( unsigned int i = 0; i < layerList.length(); ++i )
  {
    QString name, title, abstract, crs, style;
    QDomElement layerElem = layerList.at( i ).toElement();
    QDomNodeList nameList = layerElem.elementsByTagNameNS( WMS_NAMESPACE, "Name" );
    if ( nameList.size() > 0 )
    {
      name = nameList.at( 0 ).toElement().text();
    }
    QDomNodeList titleList = layerElem.elementsByTagNameNS( WMS_NAMESPACE, "Title" );
    if ( titleList.size() > 0 )
    {
      title = titleList.at( 0 ).toElement().text();
    }
    QDomNodeList abstractList = layerElem.elementsByTagNameNS( WMS_NAMESPACE, "Abstract" );
    if ( abstractList.size() > 0 )
    {
      abstract = abstractList.at( 0 ).toElement().text();
    }

    //CRS in WMS 1.3
    QDomNodeList crsList = layerElem.elementsByTagNameNS( WMS_NAMESPACE, "CRS" );
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
    QDomNodeList srsList = layerElem.elementsByTagNameNS( WMS_NAMESPACE, "SRS" );
    for ( int i = 0; i < srsList.size(); ++i )
    {
      QString srsName = srsList.at( i ).toElement().text();
      if ( i > 0 )
      {
        srsName.prepend( "," );
      }
      crs.append( srsName );
    }
    QDomNodeList styleList = layerElem.elementsByTagNameNS( WMS_NAMESPACE, "Style" );
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
    item->setText( 1, title );
    item->setText( 2, "WMS" );
    item->setText( 3, abstract );
    item->setText( 4, crs );
    item->setText( 5, style );
    //todo: add style / CRS?
    mDatasourceLayersTreeWidget->addTopLevelItem( item );
  }
}

QString NiwaPluginDialog::serviceURLFromComboBox()
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
    url.append( "?" );
  }
  return url;
}

QString NiwaPluginDialog::wfsUrlFromLayerItem( QTreeWidgetItem* item )
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
  QString srs = item->text( 4 );

  wfsUrl = url + "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" + layerName;
  if ( !srs.isEmpty() )
  {
    wfsUrl.append( "&SRSNAME=" + srs );
  }
  return wfsUrl;
}
