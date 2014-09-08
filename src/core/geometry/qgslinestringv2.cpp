/***************************************************************************
                         qgslinestringv2.cpp
                         -------------------
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

#include "qgslinestringv2.h"
#include "qgsapplication.h"
#include "qgswkbptr.h"

QgsLineStringV2::QgsLineStringV2(): QgsCurveV2()
{
}

QgsLineStringV2::~QgsLineStringV2()
{}

QgsAbstractGeometryV2* QgsLineStringV2::clone() const
{
  return 0; //todo...
}

void QgsLineStringV2::fromWkb( const unsigned char* wkb, size_t length )
{
  //reset
  if ( length < ( 1 + sizeof( int ) ) )
  {
    return;
  }

  QgsConstWkbPtr wkbPtr( wkb + 1 );

  //type
  wkbPtr >> mWkbType;
  bool hasZ = is3D();
  bool hasM = isMeasure();

  int nVertices = 0;
  wkbPtr >> nVertices;
  mCoords.resize( nVertices );
  if ( hasZ )
  {
    mZ.resize( nVertices );
  }
  else
  {
    mZ.clear();
  }

  if ( hasM )
  {
    mM.resize( nVertices );
  }
  else
  {
    mM.clear();
  }

  for ( int i = 0; i < nVertices; ++i )
  {
    wkbPtr >> mCoords[i].rx();
    wkbPtr >> mCoords[i].ry();
    if ( hasZ )
    {
      wkbPtr >> mZ[i];
    }
    if ( hasM )
    {
      wkbPtr >> mM[i];
    }
  }
}

void QgsLineStringV2::fromGeos( GEOSGeometry* geos )
{

}

void QgsLineStringV2::fromWkt( const QString& wkt )
{

}

QString QgsLineStringV2::asText( int precision ) const
{
  return QString(); //soon...
}

unsigned char* QgsLineStringV2::asBinary( int& binarySize ) const
{
  bool hasZ = is3D();
  bool hasM = isMeasure();

  int nVertices = mCoords.size();
  binarySize = 1 + 2 * sizeof( int ) + nVertices * 2 * sizeof( double );
  if ( hasZ )
  {
    binarySize += ( mZ.size() * 2 * sizeof( double ) );
  }
  if ( hasM )
  {
    binarySize += ( mM.size() * 2 * sizeof( double ) );
  }
  unsigned char* geomPtr = new unsigned char[binarySize];
  char byteOrder = QgsApplication::endian();
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();
  wkb << nVertices;
  for ( int i = 0; i < nVertices; ++i )
  {
    wkb << mCoords.at( i ).x();
    wkb << mCoords.at( i ).y();
    if ( hasZ )
    {
      wkb << mZ.at( i );
    }
    if ( hasM )
    {
      wkb << mM.at( i );
    }
  }

  return geomPtr;
}

QString QgsLineStringV2::asGML() const
{
  return QString();
}

double QgsLineStringV2::length() const
{
  return 0; //todo...
}

QgsPointV2 QgsLineStringV2::startPoint() const
{
  return QgsPointV2();
}

QgsPointV2 QgsLineStringV2::endPoint() const
{
  return QgsPointV2();
}

bool QgsLineStringV2::isClosed() const
{
  return false; //soon...
}

bool QgsLineStringV2::isRing() const
{
  return false; //soon...
}

QgsCurveV2* QgsLineStringV2::curveToLine() const
{
  return dynamic_cast<QgsCurveV2*>( clone() );
}
