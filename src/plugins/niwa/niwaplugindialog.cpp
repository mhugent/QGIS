#include "niwaplugindialog.h"
#include "addservicedialog.h"
#include "qgsnetworkaccessmanager.h"
#include <QDomDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";

NiwaPluginDialog::NiwaPluginDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ), mCapabilitiesReply( 0 )
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
  insertWFSServices();
  insertWMSServices();
}

void NiwaPluginDialog::insertWMSServices()
{
  //todo...
}

void NiwaPluginDialog::insertWFSServices()
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wfs" );
  QStringList keys = settings.childGroups();
  QStringList::const_iterator it = keys.constBegin();
  for ( ; it != keys.constEnd(); ++it )
  {
    mServicesComboBox->addItem( *it, "WFS" );
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
  int currentIndex = mServicesComboBox->currentIndex();
  if ( currentIndex == -1 )
  {
    return;
  }
  QString serviceType = mServicesComboBox->itemData( currentIndex ).toString();

  //make service url
  QSettings settings;
  QString url = settings.value( QString( "/Qgis/connections-wfs/" ) + mServicesComboBox->currentText() + QString( "/url" ) ).toString();
  if ( !url.endsWith( "?" ) && !url.endsWith( "&" ) )
  {
    url.append( "?" );
  }
  url.append( "REQUEST=GetCapabilities&SERVICE=" );
  url.append( serviceType );

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

  QTreeWidgetItem* newItem = new QTreeWidgetItem();
  newItem->setText( 0, name );
  newItem->setText( 1, type );
  newItem->setText( 2, tr( "online" ) );
  newItem->setText( 3, abstract );
  mLayerTreeWidget->addTopLevelItem( newItem );
}

void NiwaPluginDialog::wfsCapabilitiesRequestFinished()
{
  if ( mCapabilitiesReply->error() != QNetworkReply::NoError )
  {
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
  }

  QDomNodeList featureTypeList = capabilitiesDocument.elementsByTagNameNS( WFS_NAMESPACE, "FeatureType" );
  for ( unsigned int i = 0; i < featureTypeList.length(); ++i )
  {
    QString name, title, abstract;
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

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText( 0, name );
    item->setText( 1, title );
    item->setText( 2, "WFS" );
    item->setText( 3, abstract );
    mDatasourceLayersTreeWidget->addTopLevelItem( item );
  }
}

void NiwaPluginDialog::wmsCapabilitiesRequestFinished()
{

}
