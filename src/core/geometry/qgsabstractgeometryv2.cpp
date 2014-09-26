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
}

QgsAbstractGeometryV2& QgsAbstractGeometryV2::operator=( const QgsAbstractGeometryV2 & geom )
{
  mVectorTopology = new QgsGeos( this );
  return *this;
}

void QgsAbstractGeometryV2::ref()
{
  ++mRefs;
}

void QgsAbstractGeometryV2::deref()
{
  --mRefs;
  if ( mRefs <= 0 )
  {
    delete this;
  }
}

bool QgsAbstractGeometryV2::is3D() const
{
  int val = ( mWkbType << 6 );
  return ( val == 2 || val == 3 || val == 80 );
}

bool QgsAbstractGeometryV2::isMeasure() const
{
  int val = ( mWkbType << 6 );
  return ( val == 4 || val == 2 );
}

void QgsAbstractGeometryV2::geometryChanged()
{
  if ( mVectorTopology )
  {
    mVectorTopology->geometryChanged();
  }
}
