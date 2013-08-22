/***************************************************************************
                          qgssosprovider.cpp  -  description
                          --------------------------------
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

#include "qgssosprovider.h"
#include "qgssosdata.h"
#include "qgssosfeatureiterator.h"
#include "qgsspatialindex.h"

static const QString TEXT_PROVIDER_KEY = "SOS";
static const QString TEXT_PROVIDER_DESCRIPTION = "SOS data provider";

QgsSOSProvider::QgsSOSProvider( const QString& uri ): QgsVectorDataProvider( uri ), mValid( false ),
    mSpatialIndex( 0 ), mActiveIterator( 0 )
{
  loadData();

  //expect sampling spatial output
  QgsField idField( "identifier", QVariant::String );
  QgsField nameField( "name", QVariant::String );
  mFields.append( idField );
  mFields.append( nameField );
}

QgsSOSProvider::~QgsSOSProvider()
{
}

void QgsSOSProvider::loadData()
{
  QgsSOSData data( dataSourceUri() );
  if ( data.getFeatures( &mFeatures, &mCrs, &mExtent ) == 0 )
  {
    reloadSpatialIndex();
    mValid = true;
  }
  else
  {
    mValid = false;
  }
}

void QgsSOSProvider::reloadSpatialIndex()
{
  delete mSpatialIndex;
  mSpatialIndex = new QgsSpatialIndex();
  QMap<QgsFeatureId, QgsFeature* >::const_iterator fIt = mFeatures.constBegin();
  for ( ; fIt != mFeatures.constEnd(); ++fIt )
  {
    mSpatialIndex->insertFeature( *( fIt.value() ) );
  }
}

QgsFeatureIterator QgsSOSProvider::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsSOSFeatureIterator( this, request ) );
}

QGis::WkbType QgsSOSProvider::geometryType() const
{
  return QGis::WKBPoint;
}

long QgsSOSProvider::featureCount() const
{
  return mFeatures.size();
}

const QgsFields& QgsSOSProvider::fields() const
{
  return mFields;
}

QgsCoordinateReferenceSystem QgsSOSProvider::crs()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  return crs;
}

QgsRectangle QgsSOSProvider::extent()
{
  return mExtent;
}

bool QgsSOSProvider::isValid()
{
  return mValid;
}

QString QgsSOSProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsSOSProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QGISEXTERN QgsSOSProvider* classFactory( const QString *uri )
{
  return new QgsSOSProvider( *uri );
}

QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  return true;
}
