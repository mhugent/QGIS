/***************************************************************************
                         qgspolygonv2.cpp
                         ----------------
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

#include "qgspolygonv2.h"
#include "qgsapplication.h"
#include "qgslinestringv2.h"
#include "qgswkbptr.h"

QgsPolygonV2::QgsPolygonV2(): QgsCurvePolygonV2()
{

}

QgsPolygonV2::~QgsPolygonV2()
{

}

void QgsPolygonV2::fromWkb( const unsigned char* wkb )
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

  bool hasZ = is3D();
  bool hasM = isMeasure();
  double x, y, z, m;

  int nPoints = 0;
  for ( int i = 0; i < nRings; ++i )
  {
    QgsLineStringV2* line = new QgsLineStringV2();
    line->fromWkbPoints( mWkbType, wkbPtr );

    if ( i == 0 )
    {
      mExteriorRing = line;
    }
    else
    {
      mInteriorRings.append( line );
    }
  }
}

unsigned char* QgsPolygonV2::asBinary( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[wkbSize()];
  char byteOrder = QgsApplication::endian();
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();

  int nRings = 0;
  if ( mExteriorRing )
  {
    nRings = mInteriorRings.size() + 1;
  }
  else
  {
    delete[] geomPtr;
    return 0;
  }

  wkb << nRings;
  unsigned char* ringWkbPtr = wkb;
  ringWkb( &ringWkbPtr, mExteriorRing );
  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    ringWkb( &ringWkbPtr, ( *ringIt ) );
  }

  return geomPtr;
}

int QgsPolygonV2::wkbSize() const
{
  int size = 1 + 2 * sizeof( int );
  if ( mExteriorRing )
  {
    size += ringWkbSize( mExteriorRing );
  }

  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    size += ringWkbSize( *ringIt );
  }

  return size;
}

int QgsPolygonV2::ringWkbSize( const QgsCurveV2* ring )
{
  int size = 0;
  const QgsLineStringV2* line = dynamic_cast<const QgsLineStringV2*>( ring );
  if ( !ring )
  {
    return size;
  }

  size = sizeof( int );
  size += 2 * sizeof( double ) * line->numPoints();
  if ( line->is3D() )
  {
    size += sizeof( double ) * line->numPoints();
  }
  if ( line->isMeasure() )
  {
    size += sizeof( double ) * line->numPoints();
  }
  return size;
}

void QgsPolygonV2::ringWkb( unsigned char** wkb, QgsCurveV2* ring )
{
  const QgsLineStringV2* line = dynamic_cast<const QgsLineStringV2*>( ring );
  if ( !ring )
  {
    return;
  }

  bool hasZ = line->is3D();
  bool hasM = line->isMeasure();

  int nPoints = line->numPoints();

  QgsWkbPtr wkbPtr( *wkb );
  wkbPtr << nPoints;
  for ( int i = 0; i < nPoints; ++i )
  {
    QgsPointV2 pt = line->pointN( i );
    wkbPtr << pt.x();
    wkbPtr << pt.y();
    if ( hasZ )
    {
      wkbPtr << pt.z();
    }
    if ( hasM )
    {
      wkbPtr << pt.m();
    }
  }
  *wkb = wkbPtr;
}
