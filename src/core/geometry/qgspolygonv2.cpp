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

QgsPolygonV2* QgsPolygonV2::clone() const
{
  return new QgsPolygonV2( *this );
}

bool QgsPolygonV2::fromWkb( const unsigned char* wkb )
{
  clear();

  QgsConstWkbPtr wkbPtr( wkb );
  bool endianSwap;
  if ( !readWkbHeader( wkbPtr, mWkbType, endianSwap, QgsWKBTypes::Polygon ) )
    return false;

  quint32 nRings;
  wkbPtr >> nRings;
  if ( endianSwap )
    QgsApplication::endian_swap( nRings );

  if ( nRings <= 0 )
  {
    clear();
    return false;
  }

  mInteriorRings.reserve( nRings );

  for ( quint32 i = 0; i < nRings; ++i )
  {
    mInteriorRings.append( new QgsLineStringV2() );
    static_cast<QgsLineStringV2*>( mInteriorRings.back() )->setPoints( pointsFromWKB( wkbPtr, is3D(), isMeasure(), endianSwap ) );
  }

  mExteriorRing = mInteriorRings.first();
  mInteriorRings.removeFirst();

  return true;
}

int QgsPolygonV2::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  if ( mExteriorRing )
  {
    // Endianness and WkbType is not stored for LinearRings
    size += mExteriorRing->wkbSize() - ( sizeof( char ) + sizeof( quint32 ) );
  }
  foreach ( const QgsCurveV2* curve, mInteriorRings )
  {
    // Endianness and WkbType is not stored for LinearRings
    size += curve->wkbSize() - ( sizeof( char ) + sizeof( quint32 ) );
  }
  return size;
}

unsigned char* QgsPolygonV2::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>(( mExteriorRing != 0 ) + mInteriorRings.size() );
  if ( mExteriorRing )
  {
    QList<QgsPointV2> pts;
    mExteriorRing->points( pts );
    pointsToWKB( wkb, pts, mExteriorRing->is3D(), mExteriorRing->isMeasure() );
  }
  foreach ( const QgsCurveV2* curve, mInteriorRings )
  {
    QList<QgsPointV2> pts;
    curve->points( pts );
    pointsToWKB( wkb, pts, curve->is3D(), curve->isMeasure() );
  }
  return geomPtr;
}

QgsPolygonV2* QgsPolygonV2::surfaceToPolygon() const
{
  return clone();
}
