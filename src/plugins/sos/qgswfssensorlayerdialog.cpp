#include "qgswfssensorlayerdialog.h"
#include "qgsaddwfssensorlayerdialog.h"
#include "qgsmaplayerregistry.h"
#include <QSettings>

QgsWFSSensorLayerDialog::QgsWFSSensorLayerDialog( QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  QObject::connect( mAddRowButton, SIGNAL( clicked() ), this, SLOT( addEntry() ) );
  QObject::connect( mRemoveRowButton, SIGNAL( clicked() ), this, SLOT( removeEntry() ) );
  QObject::connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( writeSettings() ) );

  //fill current settings into table widget
  QSettings s;
  QStringList urlList = s.value( "SOS/WFSSensorUrls" ).toStringList();
  QStringList sosList = s.value( "SOS/WFSSOSSensorUrls" ).toStringList();
  QStringList idList = s.value( "SOS/WFSSensorFeatureIds" ).toStringList();
  QStringList obsList = s.value( "SOS/WFSSensorObservableAttributes" ).toStringList();
  QStringList beginList = s.value( "SOS/WFSSensorBeginAttributes" ).toStringList();
  QStringList endList = s.value( "SOS/WFSSensorEndAttributes" ).toStringList();

  int nRows = urlList.size();
  for ( int i = 0; i < nRows; ++i )
  {
    mTableWidget->insertRow( mTableWidget->rowCount() );
    mTableWidget->setItem( i, 0, new QTableWidgetItem( urlList.at( i ) ) );
    if ( i < sosList.size() )
    {
      mTableWidget->setItem( i, 1, new QTableWidgetItem( sosList.at( i ) ) );
    }
    if ( i < idList.size() )
    {
      mTableWidget->setItem( i, 2, new QTableWidgetItem( idList.at( i ) ) );
    }
    if ( i < obsList.size() )
    {
      mTableWidget->setItem( i, 3, new QTableWidgetItem( obsList.at( i ) ) );
    }
    if ( i < beginList.size() )
    {
      mTableWidget->setItem( i, 4, new QTableWidgetItem( beginList.at( i ) ) );
    }
    if ( i < endList.size() )
    {
      mTableWidget->setItem( i, 5, new QTableWidgetItem( endList.at( i ) ) );
    }
  }
}

QgsWFSSensorLayerDialog::~QgsWFSSensorLayerDialog()
{
}

void QgsWFSSensorLayerDialog::writeSettings()
{
  QStringList urlList, sosList, idList, obsList, beginList, endList;
  for ( int i = 0; i < mTableWidget->rowCount(); ++i )
  {
    urlList.append( mTableWidget->item( i, 0 )->text() );
    sosList.append( mTableWidget->item( i, 1 )->text() );
    idList.append( mTableWidget->item( i, 2 )->text() );
    obsList.append( mTableWidget->item( i, 3 )->text() );
    beginList.append( mTableWidget->item( i, 4 )->text() );
    endList.append( mTableWidget->item( i, 5 )->text() );
  }

  QSettings s;
  s.setValue( "SOS/WFSSensorUrls", urlList );
  s.setValue( "SOS/WFSSOSSensorUrls", sosList );
  s.setValue( "SOS/WFSSensorFeatureIds", idList );
  s.setValue( "SOS/WFSSensorObservableAttributes", obsList );
  s.setValue( "SOS/WFSSensorBeginAttributes", beginList );
  s.setValue( "SOS/WFSSensorEndAttributes", endList );
}

void QgsWFSSensorLayerDialog::addEntry()
{
  QgsAddWFSSensorLayerDialog d( QgsMapLayerRegistry::instance()->mapLayers() );
  if ( d.exec() == QDialog::Accepted )
  {
    mTableWidget->insertRow( mTableWidget->rowCount() );
    mTableWidget->setItem( mTableWidget->rowCount() - 1, 0, new QTableWidgetItem( d.wfsUrl() ) );
    mTableWidget->setItem( mTableWidget->rowCount() - 1, 1, new QTableWidgetItem( d.sosUrl() ) );
    mTableWidget->setItem( mTableWidget->rowCount() - 1, 2, new QTableWidgetItem( d.idAttribute() ) );
    mTableWidget->setItem( mTableWidget->rowCount() - 1, 3, new QTableWidgetItem( d.observableAttribute() ) );
    mTableWidget->setItem( mTableWidget->rowCount() - 1, 4, new QTableWidgetItem( d.fromAttribute() ) );
    mTableWidget->setItem( mTableWidget->rowCount() - 1, 5, new QTableWidgetItem( d.toAttribute() ) );
  }
}

void QgsWFSSensorLayerDialog::removeEntry()
{
  int currentRow = mTableWidget->currentRow();
  if ( currentRow >= 0 )
  {
    mTableWidget->removeRow( currentRow );
  }
}
