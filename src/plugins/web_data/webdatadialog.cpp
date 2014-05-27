#include "webdatadialog.h"
#include "addservicedialog.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsnetworkaccessmanager.h"
#include <QDomDocument>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>

WebDataDialog::WebDataDialog( QgisInterface* iface, QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ), mIface( iface ),
    mModel( iface ), mNIWAServicesRequestFinished( false )
{
  setupUi( this );
  insertServices();
  mFilterModel.setParent( this );
  mFilterModel.setSourceModel( &mModel );
  mFilterModel.setFilterKeyColumn( -1 );
  mFilterModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mLayersTreeView->setModel( &mFilterModel );
  connect( mLayersTreeView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( showContextMenu( const QPoint& ) ) );
  connect( &mModel, SIGNAL( serviceAdded() ), this, SLOT( resetStateAndCursor() ) );
  QSettings s;
  mOnlyFavouritesCheckBox->setCheckState( s.value( "/NIWA/showOnlyFavourites", "false" ).toBool() ? Qt::Checked : Qt::Unchecked );

  //expand items
  QStringList expandedServices = s.value( "/NIWA/expandedServices" ).toStringList();
  QSet<QString> expanded = expandedServices.toSet();
  int nChildren = mFilterModel.rowCount();
  for ( int i = 0; i < nChildren; ++i )
  {
    QModelIndex idx = mFilterModel.index( i, 0 );
    if ( expanded.contains( mModel.itemFromIndex( mFilterModel.mapToSource( idx ) )->text() ) )
    {
      mLayersTreeView->setExpanded( idx, true );
    }
  }

  mContextMenu = new QMenu();
  mContextMenu->addAction( QIcon( ":/niwa/icons/remove_from_list.png" ), tr( "Delete" ), this, SLOT( deleteEntry( ) ) );
  mContextMenu->addAction( QIcon( ":/niwa/icons/refresh.png" ), tr( "Update" ), this, SLOT( updateEntry() ) );
}

WebDataDialog::~WebDataDialog()
{
  QSettings s;
  s.setValue( "/NIWA/showOnlyFavourites", mOnlyFavouritesCheckBox->isChecked() );

  //store names of expanded items
  QStringList expandedServices;
  int nChildren = mFilterModel.rowCount();
  for ( int i = 0; i < nChildren; ++i )
  {
    if ( mLayersTreeView->isExpanded( mFilterModel.index( i, 0 ) ) )
    {
      expandedServices.append( mModel.itemFromIndex( mFilterModel.mapToSource( mFilterModel.index( i, 0 ) ) )->text() );
    }
  }
  s.setValue( "/NIWA/expandedServices", expandedServices );
  delete mContextMenu;
}

void WebDataDialog::on_mConnectPushButton_clicked()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  mStatusLabel->setText( tr( "Retrieving service capabilities..." ) );
  QString url = serviceURLFromComboBox();
  int currentIndex = mServicesComboBox->currentIndex();
  QString serviceType = mServicesComboBox->itemData( currentIndex ).toString();
  QString serviceTitle = mServicesComboBox->currentText();
  mModel.addService( serviceTitle, url, serviceType );
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
//  addServicesFromHtml( "https://www.niwa.co.nz/ei/feeds/report" );
  addServicesFromCSW( "http://dc.niwa.co.nz/niwa_dc_cogs/srv/eng/csw" );
}

void WebDataDialog::on_mAddLINZServicesButton_clicked()
{
  //ask user about the LINZ key
  QString key = QInputDialog::getText( 0, tr( "Enter your personal LINZ key" ), tr( "Key:" ), QLineEdit::Normal, QString(), 0,
                                       Qt::Dialog | Qt::WindowStaysOnTopHint );
  if ( key.isNull() )
  {
    return;
  }

  //add WFS
  setServiceSetting( "LINZ WFS", "WFS", "http://wfs.data.linz.govt.nz/" + key + "/wfs" );

  //add WMS
  setServiceSetting( "LINZ WMS", "WMS", "http://wms.data.linz.govt.nz/" + key + "/r/wms" );
}

void WebDataDialog::on_mAddLRISButton_clicked()
{
  //ask user about the LRIS key
  QString key = QInputDialog::getText( 0, tr( "Enter your personal LRIS key" ), tr( "Key:" ), QLineEdit::Normal, QString(), 0,
                                       Qt::Dialog | Qt::WindowStaysOnTopHint );
  if ( !key.isNull() )
  {
    //add WFS
    setServiceSetting( "LRIS WFS", "WFS", "http://wfs.lris.scinfo.org.nz/" + key + "/wfs" );
  }

  //add WMS
  setServiceSetting( "LRIS Basemaps", "WMS", "http://maps.scinfo.org.nz/basemaps/wms" );
  setServiceSetting( "LRIS Land ressource inventory", "WMS", "http://maps.scinfo.org.nz/lri/wms" );
  setServiceSetting( "LRIS Land cover", "WMS", "http://maps.scinfo.org.nz/lcdb/wms" );
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

void WebDataDialog::addServicesFromCSW( const QString &url )
{
  //get xml from csw

  mNIWAServicesRequestFinished = false;
  QString get = QString( "%1?SERVICE=CSW&REQUEST=GetRecords&VERSION=2.0.2&CONSTRAINTLANGUAGE=CQL_TEXT&RESULTTYPE=results&maxrecords=200&constraint=dc:type LIKE 'service'&constraint_language_version=1.1.0&ElementSetName=full" ).arg( url );
  QNetworkRequest request( get );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, SIGNAL( finished() ), this, SLOT( NIWAServicesRequestFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleDownloadProgress( qint64, qint64 ) ) );

  while ( !mNIWAServicesRequestFinished )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
  }

  QByteArray response = reply->readAll();
  reply->deleteLater();

  QDomDocument xml;
  QString errorMsg;
  int errorLine;
  int errorColumn;
  if ( !xml.setContent( response, false, &errorMsg, &errorLine, &errorColumn ) )
  {
    QMessageBox::critical( 0, tr( "Failed to parse XML file" ), tr( "Error parsing the xml from %1: %2 on line %3, column %4" ).arg( url ).arg( errorMsg ).arg( errorLine ).arg( errorColumn ) );
    return;
  }

  QRegExp wmsTest("OGC:WMS-[\\w\\.]+-http-get-capabilities");

  QDomNodeList records = xml.elementsByTagName( "csw:Record" );
  for ( int i = 0; i < records.size(); ++i )
  {
    QDomElement record = records.at( i ).toElement();
    QString title = record.firstChildElement( "dc:title" ).text();
    QDomNodeList uris = record.elementsByTagName( "dc:URI" );
    for ( int j = 0; j < uris.size(); ++j )
    {
      QDomElement uri = uris.at( j ).toElement();
      QString protocol = uri.attribute( "protocol" );
      if ( wmsTest.indexIn(protocol) != -1 )
      {
        setServiceSetting( title, "WMS", uri.text() );
      }
      else if ( protocol == "WWW:LINK-1.0-http--link" )
      {
        setServiceSetting( title, "WFS", uri.text() );
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

void WebDataDialog::on_mOnlyFavouritesCheckBox_stateChanged( int state )
{
  mFilterModel.setShowOnlyFavourites( state == Qt::Checked );
}

void WebDataDialog::on_mSearchTableEdit_textChanged( const QString&  text )
{
  mFilterModel._setFilterWildcard( text );
}

void WebDataDialog::on_mLayersTreeView_clicked( const QModelIndex& index )
{
  QModelIndex srcIndex = mFilterModel.mapToSource( index );
  QStandardItem* item = mModel.itemFromIndex( srcIndex );
  if ( !item || item->column() != 4 )
  {
    return;
  }

  if ( item->text().compare( "online", Qt::CaseInsensitive ) == 0 )
  {
    mStatusLabel->setText( tr( "Saving layer offline..." ) );
    mModel.changeEntryToOffline( srcIndex.sibling( srcIndex.row(), 0 ) );
  }
  else if ( item->text().compare( "offline", Qt::CaseInsensitive ) == 0 )
  {
    mStatusLabel->setText( tr( "Changing layer to online..." ) );
    mModel.changeEntryToOnline( srcIndex.sibling( srcIndex.row(), 0 ) );
  }
  resetStateAndCursor();
}

void WebDataDialog::resetStateAndCursor()
{
  mStatusLabel->setText( tr( "Ready" ) );
  QApplication::restoreOverrideCursor();
}

void WebDataDialog::keyPressEvent( QKeyEvent* event )
{
  if ( event->key() != Qt::Key_Delete && event->key() != Qt::Key_F5 )
  {
    return;
  }

  //find selected model index
  QModelIndex srcIndex;
  QItemSelectionModel * selectModel = mLayersTreeView->selectionModel();
  QModelIndexList selectList = selectModel->selectedRows( 0 );
  if ( selectList.size() > 0 )
  {
    srcIndex = mFilterModel.mapToSource( selectList.at( 0 ) );
  }

  if ( !srcIndex.isValid() )
  {
    return;
  }

  if ( event->key() == Qt::Key_Delete )
  {
    mModel.removeRow( srcIndex.row(), srcIndex.parent() );
  }
  else if ( event->key() == Qt::Key_F5 )
  {
    if ( mapCanvasDrawing() )
    {
      return;
    }
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    mStatusLabel->setText( tr( "Reloading..." ) );
    mModel.reload( srcIndex );
    resetStateAndCursor();
  }

}

bool WebDataDialog::mapCanvasDrawing() const
{
  if ( !mIface )
  {
    return false;
  }

  QgsMapCanvas* canvas = mIface->mapCanvas();
  if ( !canvas )
  {
    return false;
  }

  return canvas->isDrawing();
}

void WebDataDialog::deleteEntry()
{
  QModelIndex srcIndex = selectedModelIndex();
  if ( !srcIndex.isValid() )
  {
    return;
  }
  mModel.removeRow( srcIndex.row(), srcIndex.parent() );
}

void WebDataDialog::updateEntry()
{
  QModelIndex srcIndex = selectedModelIndex();
  if ( !srcIndex.isValid() )
  {
    return;
  }
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  mStatusLabel->setText( tr( "Reloading..." ) );
  mModel.reload( srcIndex );
  resetStateAndCursor();
}

void WebDataDialog::showContextMenu( const QPoint&  point )
{
  Q_UNUSED( point );
  if ( mContextMenu )
  {
    mContextMenu->exec( QCursor::pos() );
  }
}

QModelIndex WebDataDialog::selectedModelIndex() const
{
  //find selected model index
  QModelIndex srcIndex;
  QItemSelectionModel * selectModel = mLayersTreeView->selectionModel();
  QModelIndexList selectList = selectModel->selectedRows( 0 );
  if ( selectList.size() > 0 )
  {
    return mFilterModel.mapToSource( selectList.at( 0 ) );
  }
  return QModelIndex(); //return invalid index in case of error
}
