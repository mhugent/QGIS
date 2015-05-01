#include "qgsaddwfssensorlayerdialog.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayer.h"
#include <QUrl>

QgsAddWFSSensorLayerDialog::QgsAddWFSSensorLayerDialog( const QMap<QString, QgsMapLayer*>& layerMap, QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );

  QMap<QString, QgsMapLayer*>::const_iterator layerIt = layerMap.constBegin();
  for ( ; layerIt != layerMap.constEnd(); ++layerIt )
  {
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layerIt.value() );
    if ( !vl )
    {
      continue;
    }

    //provider key WFS?
    if ( vl->dataProvider()->name() != "WFS" )
    {
      continue;
    }

    mLayerComboBox->addItem( vl->name(), vl->dataProvider()->dataSourceUri() );
  }
}

QgsAddWFSSensorLayerDialog::~QgsAddWFSSensorLayerDialog()
{
}

void QgsAddWFSSensorLayerDialog::on_mLayerComboBox_currentIndexChanged( const QString& text )
{
  QgsVectorLayer* vl = 0;

  //take first wfs layer
  QList<QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayersByName( text );
  QList<QgsMapLayer*>::const_iterator layerIt = layers.constBegin();
  for ( ; layerIt != layers.constEnd(); ++layerIt )
  {
    QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>( *layerIt );
    if ( vLayer && vLayer->dataProvider()->name() == "WFS" )
    {
      vl = vLayer;
      break;
    }
  }

  mIdAttributeComboBox->clear(); mObservableAttributeComboBox->clear(); mBeginAttributeComboBox->clear(); mEndAttributeComboBox->clear();
  if ( !vl )
  {
    return;
  }

  addAttributesToComboBox( mIdAttributeComboBox, vl );
  addAttributesToComboBox( mObservableAttributeComboBox, vl );
  addAttributesToComboBox( mBeginAttributeComboBox, vl );
  addAttributesToComboBox( mEndAttributeComboBox, vl );
}

void QgsAddWFSSensorLayerDialog::addAttributesToComboBox( QComboBox* cb, const QgsVectorLayer* vl )
{
  const QgsFields& fields = vl->pendingFields();
  for ( int i = 0; i < fields.size(); ++i )
  {
    cb->addItem( fields.at( i ).name() );
  }
}

QString QgsAddWFSSensorLayerDialog::wfsUrl() const
{
  int idx = mLayerComboBox->currentIndex();
  if ( idx < 0 )
  {
    return QString();
  }
  return mLayerComboBox->itemData( idx ).toString();
}

QString QgsAddWFSSensorLayerDialog::sosUrl() const
{
  //default: remove parameters from WFS url
  int idx = mLayerComboBox->currentIndex();
  if ( idx < 0 )
  {
    return QString();
  }
  QUrl url( mLayerComboBox->itemData( idx ).toString() );
  QList<QPair<QString, QString> > queryItems = url.queryItems();
  QList<QPair<QString, QString> >::const_iterator queryIt = queryItems.constBegin();
  for ( ; queryIt != queryItems.constEnd(); ++queryIt )
  {
    url.removeQueryItem( queryIt->first );
  }
  return url.toString();
}

QString QgsAddWFSSensorLayerDialog::idAttribute() const
{
  return mIdAttributeComboBox->currentText();
}

QString QgsAddWFSSensorLayerDialog::observableAttribute() const
{
  return mObservableAttributeComboBox->currentText();
}

QString QgsAddWFSSensorLayerDialog::fromAttribute() const
{
  return mBeginAttributeComboBox->currentText();
}

QString QgsAddWFSSensorLayerDialog::toAttribute() const
{
  return mEndAttributeComboBox->currentText();
}


