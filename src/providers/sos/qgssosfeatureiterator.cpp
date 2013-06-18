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
#include "qgsmessagelog.h"
#include "qgssosprovider.h"
#include "qgsspatialindex.h"

QgsSOSFeatureIterator::QgsSOSFeatureIterator( QgsSOSProvider* provider, const QgsFeatureRequest& request ): QgsAbstractFeatureIterator( request ), mProvider( provider )
{
    if ( !mProvider )
    {
      return;
    }

    //close other iterators on provider
    if ( mProvider->mActiveIterator )
    {
      QgsMessageLog::logMessage( QObject::tr( "Already active iterator on this provider was closed." ), QObject::tr( "WFS" ) );
      mProvider->mActiveIterator->close();
    }

    mProvider->mActiveIterator = this;
    switch ( request.filterType() )
    {
        case QgsFeatureRequest::FilterRect:
            mSelectedFeatures = mProvider->mSpatialIndex->intersects( request.filterRect() );
            break;
        case QgsFeatureRequest::FilterFid:
            mSelectedFeatures.push_back( request.filterFid() );
            break;
        case QgsFeatureRequest::FilterNone:
        default: //QgsFeatureRequest::FilterNone
            mSelectedFeatures = mProvider->mFeatures.keys();
    }
    mFeatureIterator = mSelectedFeatures.constBegin();
}

QgsSOSFeatureIterator::~QgsSOSFeatureIterator()
{

}

bool QgsSOSFeatureIterator::nextFeature( QgsFeature& f )
{
    if ( !mProvider )
    {
      return false;
    }

    if ( mFeatureIterator == mSelectedFeatures.constEnd() )
    {
      return false;
    }

    QMap<QgsFeatureId, QgsFeature* >::iterator it = mProvider->mFeatures.find( *mFeatureIterator );
    if ( it == mProvider->mFeatures.end() )
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
      attributes = mProvider->attributeIndexes();
    }
    mProvider->copyFeature( fet, f, !( mRequest.flags() & QgsFeatureRequest::NoGeometry ), attributes );
    ++mFeatureIterator;
    return true;
}


bool QgsSOSFeatureIterator::rewind()
{
    if ( !mProvider )
    {
      return false;
    }

    mFeatureIterator = mSelectedFeatures.constBegin();
    return true;
}

bool QgsSOSFeatureIterator::close()
{
    if ( !mProvider )
    {
      return false;
    }
    mProvider->mActiveIterator = 0;
    mProvider = 0;
    return true;
}

