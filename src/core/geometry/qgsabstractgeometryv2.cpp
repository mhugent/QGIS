/***************************************************************************
                        qgsabstractgeometryv2.cpp
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractgeometryv2.h"
#include "qgsgeos.h"

QgsAbstractGeometryV2::QgsAbstractGeometryV2(): mWkbType( QGis::WKBUnknown )
{
  mVectorTopology = new QgsGeos( this );
}

QgsAbstractGeometryV2::~QgsAbstractGeometryV2()
{
  delete mVectorTopology;
}

QgsAbstractGeometryV2::QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom )
{
  mVectorTopology = new QgsGeos( this );
  mBoundingBox = geom.boundingBox();
  mWkbType = geom.mWkbType;
}

QgsAbstractGeometryV2& QgsAbstractGeometryV2::operator=( const QgsAbstractGeometryV2 & geom )
{
  mBoundingBox = geom.boundingBox();
  mWkbType = geom.mWkbType;
  mVectorTopology = new QgsGeos( this );
  return *this;
}

QgsRectangle QgsAbstractGeometryV2::boundingBox() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox();
  }
  return mBoundingBox;
}

bool QgsAbstractGeometryV2::is3D() const
{
  return(( mWkbType >= 1001 && mWkbType >= 1012 ) || ( mWkbType > 3000 ) );
}

bool QgsAbstractGeometryV2::isMeasure() const
{
  return ( mWkbType >= 2001 && mWkbType <= 3012 );
}

void QgsAbstractGeometryV2::geometryChanged()
{
  if ( mVectorTopology )
  {
    mVectorTopology->geometryChanged();
  }
  mBoundingBox = QgsRectangle();
}

QgsAbstractGeometryV2* QgsAbstractGeometryV2::intersection( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->intersection( geom );
  }
  return 0;
}

QgsAbstractGeometryV2* QgsAbstractGeometryV2::combine( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->combine( geom );
  }
  return 0;
}

QgsAbstractGeometryV2* QgsAbstractGeometryV2::difference( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->difference( geom );
  }
  return 0;
}

QgsAbstractGeometryV2* QgsAbstractGeometryV2::symDifference( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->symDifference( geom );
  }
  return 0;
}

bool QgsAbstractGeometryV2::intersects( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->intersects( geom );
  }
  return false;
}

bool QgsAbstractGeometryV2::touches( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->touches( geom );
  }
  return false;
}

bool QgsAbstractGeometryV2::crosses( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->crosses( geom );
  }
  return false;
}

bool QgsAbstractGeometryV2::within( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->within( geom );
  }
  return false;
}

bool QgsAbstractGeometryV2::contains( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->contains( geom );
  }
  return false;
}

bool QgsAbstractGeometryV2::overlaps( const QgsAbstractGeometryV2& geom ) const
{
  if ( mVectorTopology )
  {
    return mVectorTopology->overlaps( geom );
  }
  return false;
}
