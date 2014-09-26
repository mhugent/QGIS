/***************************************************************************
                         qgscurvepolygonv2.cpp
                         ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscurvepolygonv2.h"
#include "qgsapplication.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgslinestringv2.h"
#include "qgswkbptr.h"

QgsCurvePolygonV2::QgsCurvePolygonV2(): QgsSurfaceV2(), mExteriorRing( 0 )
{

}

QgsCurvePolygonV2::~QgsCurvePolygonV2()
{
  removeRings();
}

QgsAbstractGeometryV2* QgsCurvePolygonV2::clone() const
{
  return 0;
}

void QgsCurvePolygonV2::fromWkb( const unsigned char* wkb )
{
  removeRings();
  if ( !wkb )
  {
    return;
  }

  QgsConstWkbPtr wkbPtr( wkb + 1 );

  //type
  wkbPtr >> mWkbType;
  int nRings;
  wkbPtr >> nRings;

  QgsCurveV2* currentCurve = 0;
  int currentCurveSize = 0;

  for ( int i = 0; i < nRings; ++i )
  {
    wkbPtr += 1; //skip endian
    QGis::WkbType curveType;
    wkbPtr >> curveType;
    wkbPtr -= ( 1  + sizeof( int ) );

    if ( curveType == QGis::WKBLineString || curveType == QGis::WKBLineStringZ || curveType == QGis::WKBLineStringM ||
         curveType == QGis::WKBLineStringZM || curveType == QGis::WKBLineString25D )
    {
      currentCurve = new QgsLineStringV2();
    }
    else if ( curveType == QGis::WKBCircularString || curveType == QGis::WKBCircularStringZ || curveType == QGis::WKBCircularStringZM ||
              curveType == QGis::WKBCircularStringM )
    {
      currentCurve = new QgsCircularStringV2();
    }
    else if ( curveType == QGis::WKBCompoundCurve || curveType == QGis::WKBCompoundCurveZ || curveType == QGis::WKBCompoundCurveZM )
    {
      currentCurve = new QgsCompoundCurveV2();
    }

    currentCurve->fromWkb( wkbPtr );
    currentCurveSize = currentCurve->wkbSize();
    if ( i == 0 )
    {
      mExteriorRing = currentCurve;
    }
    else
    {
      mInteriorRings.append( currentCurve );
    }
    wkbPtr += currentCurveSize;
  }
  geometryChanged();
}

void QgsCurvePolygonV2::fromWkt( const QString& wkt )
{

}

QString QgsCurvePolygonV2::asText( int precision ) const
{
  return QString();
}

unsigned char* QgsCurvePolygonV2::asBinary( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  char byteOrder = QgsApplication::endian();
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();
  unsigned char* currentWkbPtr = wkb;

  if ( mExteriorRing )
  {
    addRingWkb( &currentWkbPtr, mExteriorRing );
  }

  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    addRingWkb( &currentWkbPtr, ( *ringIt ) );
  }

  return geomPtr;
}

void QgsCurvePolygonV2::addRingWkb( unsigned char** wkb, const QgsCurveV2* ring ) const
{
  if ( !ring )
  {
    return;
  }

  int ringWkbSize = 0;
  unsigned char* ringWkb = ring->asBinary( ringWkbSize );
  memcpy( *wkb, ringWkb, ringWkbSize );
  *wkb += ringWkbSize;
  delete[] ringWkb;
}

int QgsCurvePolygonV2::wkbSize() const
{
  int size = 0;
  if ( mExteriorRing )
  {
    size += mExteriorRing->wkbSize();
  }
  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    size += ( *ringIt )->wkbSize();
  }

  size += ( 1 + 2 * sizeof( int ) );
  return size;
}

QString QgsCurvePolygonV2::asGML() const
{
  return QString();
}

double QgsCurvePolygonV2::area() const
{
  return 0.0;
}

double QgsCurvePolygonV2::perimeter() const
{
  return 0.0;
}

QgsPointV2 QgsCurvePolygonV2::centroid() const
{
  return QgsPointV2( 0, 0 );
}

QgsPointV2 QgsCurvePolygonV2::pointOnSurface() const
{
  return QgsPointV2( 0, 0 );
}

void QgsCurvePolygonV2::removeRings()
{
  delete mExteriorRing;
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();
}


