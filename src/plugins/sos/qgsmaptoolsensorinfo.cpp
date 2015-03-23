/***************************************************************************
                          qgsmaptoolsensorinfo.cpp  -  description
                          --------------------------------------
    begin                : June 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolsensorinfo.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentify.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssensorinfodialog.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <QDateTime>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QNetworkRequest>

QgsMapToolSensorInfo::QgsMapToolSensorInfo( QgsMapCanvas* canvas ): QgsMapTool( canvas ),
    mDataAvailabilityRequestFinished( true ), mSensorInfoDialog( 0 )
{

}

QgsMapToolSensorInfo::~QgsMapToolSensorInfo()
{
  delete mSensorInfoDialog;
}

void QgsMapToolSensorInfo::canvasReleaseEvent( QMouseEvent * e )
{

  QList< QgsMapLayer* > sensorLayerList = sensorLayers();
  if ( sensorLayerList.isEmpty() )
  {
    return;
  }

  showSensorInfoDialog();
  mSensorInfoDialog->clearObservables();


  QgsMapToolIdentify idTool( mCanvas );
  QList<QgsMapToolIdentify::IdentifyResult> idList = idTool.identify( e->x(), e->y(), sensorLayerList, QgsMapToolIdentify::TopDownAll );
  QList<QgsMapToolIdentify::IdentifyResult>::const_iterator idListIt = idList.constBegin();
  for ( ; idListIt != idList.constEnd(); ++idListIt )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( idListIt->mLayer );
    if ( !vl )
    {
      continue;
    }
    QgsDataProvider* dp = vl->dataProvider();

    QgsMapToolIdentify::IdentifyResult debug = *idListIt;
    QString name = idListIt->mFeature.attribute( "name" ).toString();
    QString id = idListIt->mFeature.attribute( "identifier" ).toString();

    //data structure with id / name / list< observable, start time, end time>
    QStringList observedProperties;
    QList< QDateTime > beginList;
    QList< QDateTime > endList;
    getDataAvailability( dp->dataSourceUri(), id, observedProperties, beginList, endList );
    mSensorInfoDialog->addObservables( dp->dataSourceUri(), id, observedProperties, beginList, endList );
  }
}

QList< QgsMapLayer* > QgsMapToolSensorInfo::sensorLayers()
{
  QList< QgsMapLayer* > sensorLayerList;

  if ( !mCanvas )
  {
    return sensorLayerList;
  }

  QList<QgsMapLayer*> layerList = mCanvas->layers();
  QList<QgsMapLayer*>::iterator layerIt = layerList.begin();
  for ( ; layerIt != layerList.end(); ++layerIt )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( *layerIt );
    if ( vl && vl->dataProvider() )
    {
      if ( vl->dataProvider()->name() == "SOS" )
      {
        sensorLayerList.append( vl );
      }
    }
  }
  return sensorLayerList;
}

int QgsMapToolSensorInfo::getDataAvailability( const QString& serviceUrl, const QString& station_id,
    QStringList& observedPropertyList,
    QList< QDateTime >& beginList, QList< QDateTime >& endList )
{
  if ( !mDataAvailabilityRequestFinished ) //another request is still in progress
  {
    return 1;
  }

  QUrl url( serviceUrl );
  url.removeQueryItem( "observedProperty" );
  url.removeQueryItem( "request" );
  url.addQueryItem( "request", "GetDataAvailability" );
  url.addQueryItem( "featureofinterest", station_id );

  QString debug = url.toString();
  qWarning( debug.toLocal8Bit().data() );

  mDataAvailabilityRequestFinished = false;
  QUrl u = QUrl::fromEncoded( url.toString().toLocal8Bit() );
  QNetworkRequest request( u );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, SIGNAL( finished() ), this, SLOT( dataAvailabilityRequestFinished() ) );
  while ( !mDataAvailabilityRequestFinished )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 200 );
  }

  //read results from xml
  QByteArray response = reply->readAll();
  reply->deleteLater();

  QDomDocument dataAvailabilityDocument;
  if ( !dataAvailabilityDocument.setContent( response, true ) )
  {
    return 2;
  }

  QSet<QString> observedPropertySet; //keep track of duplicates

  QDomNodeList dataAvailabilityList = dataAvailabilityDocument.elementsByTagNameNS( "http://www.opengis.net/sosgda/1.0", "dataAvailabilityMember" );
  for ( int i = 0; i < dataAvailabilityList.size(); ++i )
  {
    QDomElement observedPropertyElem = dataAvailabilityList.at( i ).firstChildElement( "observedProperty" );
    if ( observedPropertyElem.isNull() )
    {
      continue;
    }
    QString property = observedPropertyElem.attribute( "href" );
    QDateTime beginTime;
    QDateTime endTime;

    QDomElement phenomenonTimeElem = dataAvailabilityList.at( i ).firstChildElement( "phenomenonTime" );
    if ( !phenomenonTimeElem.isNull() )
    {
      QDomElement timePeriodElem = phenomenonTimeElem.firstChildElement( "TimePeriod" );
      if ( !timePeriodElem.isNull() )
      {
        QDomElement beginElem = timePeriodElem.firstChildElement( "beginPosition" );
        if ( !beginElem.isNull() )
        {
          beginTime = QDateTime::fromString( beginElem.text(), "yyyy'-'MM'-'dd'T'HH':'mm':'ss'.000+00:00'" );
        }
        QDomElement endElem = timePeriodElem.firstChildElement( "endPosition" );
        if ( !endElem.isNull() )
        {
          endTime = QDateTime::fromString( endElem.text(), "yyyy'-'MM'-'dd'T'HH':'mm':'ss'.000+00:00'" );
        }
      }
    }

    if ( observedPropertySet.contains( property ) )
    {
      continue;
    }

    observedPropertySet.insert( property );
    observedPropertyList.push_back( property );
    beginList.push_back( beginTime );
    endList.push_back( endTime );
  }

  qWarning( response.data() );
  return 0;
}

void QgsMapToolSensorInfo::dataAvailabilityRequestFinished()
{
  mDataAvailabilityRequestFinished = true;
}

void QgsMapToolSensorInfo::showSensorInfoDialog()
{
  if ( !mSensorInfoDialog )
  {
    mSensorInfoDialog = new QgsSensorInfoDialog();
  }
  mSensorInfoDialog->show();
  mSensorInfoDialog->raise();
}
