/***************************************************************************
                          qgssosfeatureiterator.cpp  -  description
                          -----------------------------------------
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

#include "qgssosfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsmessagelog.h"
#include "qgssosprovider.h"
#include "qgsspatialindex.h"

QgsSOSFeatureSource::QgsSOSFeatureSource( const QgsSOSProvider* p ): QgsAbstractFeatureSource()
{
  if ( !p )
  {
    mSpatialIndex = 0;
    return ;
  }

  mSpatialIndex = new QgsSpatialIndex( *( p->mSpatialIndex ) );
  mFeatures = p->mFeatures;
  mFields = p->fields();
}

QgsSOSFeatureSource::~QgsSOSFeatureSource()
{
  delete mSpatialIndex;
}

QgsFeatureIterator QgsSOSFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return new QgsSOSFeatureIterator( this, request );
}

QgsSOSFeatureIterator::QgsSOSFeatureIterator( QgsSOSFeatureSource* source, const QgsFeatureRequest& request ): QgsAbstractFeatureIteratorFromSource( source, false, request ),
    mSource( source )
{
  if ( !mSource )
  {
    return;
  }

  switch ( request.filterType() )
  {
    case QgsFeatureRequest::FilterRect:
      mSelectedFeatures = mSource->mSpatialIndex->intersects( request.filterRect() );
      break;
    case QgsFeatureRequest::FilterFid:
      mSelectedFeatures.push_back( request.filterFid() );
      break;
    case QgsFeatureRequest::FilterNone:
    default: //QgsFeatureRequest::FilterNone
      mSelectedFeatures = mSource->mFeatures.keys();
  }
  mFeatureIterator = mSelectedFeatures.constBegin();
}

QgsSOSFeatureIterator::~QgsSOSFeatureIterator()
{

}

bool QgsSOSFeatureIterator::fetchFeature( QgsFeature& f )
{
  if ( !mSource )
  {
    return false;
  }

  if ( mFeatureIterator == mSelectedFeatures.constEnd() )
  {
    return false;
  }

  QMap<QgsFeatureId, QgsFeature* >::iterator it = mSource->mFeatures.find( *mFeatureIterator );
  if ( it == mSource->mFeatures.end() )
  {
    return false;
  }
  QgsFeature* fet =  it.value();

  QgsAttributeList attributes;
  if ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes )
  {
    attributes = mRequest.subsetOfAttributes();
  }
  else
  {
    attributes = mSource->mFields.allAttributesList();
  }

  copyFeature( fet, f, !( mRequest.flags() & QgsFeatureRequest::NoGeometry ), attributes );
  ++mFeatureIterator;
  return true;
}

void QgsSOSFeatureIterator::copyFeature( QgsFeature* f, QgsFeature& feature, bool fetchGeometry, QgsAttributeList fetchAttributes )
{
  Q_UNUSED( fetchGeometry );

  if ( !f )
  {
    return;
  }

  //copy the geometry
  QgsGeometry* geometry = f->geometry();
  if ( geometry && fetchGeometry )
  {
    const unsigned char *geom = geometry->asWkb();
    int geomSize = geometry->wkbSize();
    unsigned char* copiedGeom = new unsigned char[geomSize];
    memcpy( copiedGeom, geom, geomSize );
    feature.setGeometryAndOwnership( copiedGeom, geomSize );
  }
  else
  {
    feature.setGeometry( 0 );
  }

  //and the attributes
  const QgsAttributes& attributes = f->attributes();
  feature.setAttributes( attributes );

  int i = 0;
  for ( QgsAttributeList::const_iterator it = fetchAttributes.begin(); it != fetchAttributes.end(); ++it )
  {
    feature.setAttribute( i, attributes[*it] );
    ++i;
  }

  //id and valid
  feature.setValid( true );
  feature.setFeatureId( f->id() );
  feature.setFields( &( mSource->mFields ) ); // allow name-based attribute lookups
}


bool QgsSOSFeatureIterator::rewind()
{
  if ( !mSource )
  {
    return false;
  }

  mFeatureIterator = mSelectedFeatures.constBegin();
  return true;
}

bool QgsSOSFeatureIterator::close()
{
  if ( mClosed )
  {
    return false;
  }

  iteratorClosed();

  mClosed = true;
  return true;
}

