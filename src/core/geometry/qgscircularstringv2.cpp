/***************************************************************************
                         qgscircularstringv2.cpp
                         -----------------------
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

#include "qgscircularstringv2.h"
#include "qgsapplication.h"
#include "qgslinestringv2.h"
#include "qgspointv2.h"
#include "qgswkbptr.h"

QgsCircularStringV2::QgsCircularStringV2(): QgsCurveV2()
{

}

QgsCircularStringV2::~QgsCircularStringV2()
{

}

QgsAbstractGeometryV2* QgsCircularStringV2::clone() const
{
  return 0;
}

void QgsCircularStringV2::fromWkb( const unsigned char* wkb )
{
  if ( !wkb )
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
  mX.resize( nVertices );
  mY.resize( nVertices );
  hasZ ? mZ.resize( nVertices ) : mZ.clear();
  hasM ? mM.resize( nVertices ) : mM.clear();

  for ( int i = 0; i < nVertices; ++i )
  {
    wkbPtr >> mX[i];
    wkbPtr >> mY[i];
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

void QgsCircularStringV2::fromGeos( GEOSGeometry* geos )
{

}

void QgsCircularStringV2::fromWkt( const QString& wkt )
{

}

int QgsCircularStringV2::wkbSize() const
{
  int binarySize = 1 + 2 * sizeof( int ) + numPoints() * 2 * sizeof( double );
  if ( is3D() )
  {
    binarySize += ( mZ.size() * 2 * sizeof( double ) );
  }
  if ( isMeasure() )
  {
    binarySize += ( mM.size() * 2 * sizeof( double ) );
  }
  return binarySize;
}

QString QgsCircularStringV2::asText( int precision ) const
{
  return ""; //soon...
}

unsigned char* QgsCircularStringV2::asBinary( int& binarySize ) const
{
  bool hasZ = is3D();
  bool hasM = isMeasure();

  int nVertices = qMin( mX.size(), mY.size() );
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  char byteOrder = QgsApplication::endian();
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();
  wkb << nVertices;
  for ( int i = 0; i < nVertices; ++i )
  {
    wkb << mX.at( i );
    wkb << mY.at( i );
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

QString QgsCircularStringV2::asGML() const
{
  return "";
}

//curve interface
double QgsCircularStringV2::length() const
{
  return 0; //soon...
}

QgsPointV2 QgsCircularStringV2::startPoint() const
{
  return QgsPointV2(); //soon...
}

QgsPointV2 QgsCircularStringV2::endPoint() const
{
  return QgsPointV2(); //soon...
}

bool QgsCircularStringV2::isClosed() const
{
  return false; //soon...
}

bool QgsCircularStringV2::isRing() const
{
  return false; //soon...
}

QgsLineStringV2* QgsCircularStringV2::curveToLine() const
{
  QgsLineStringV2* line = new QgsLineStringV2();
  QList<QgsPointV2> points;
  for ( int i = 0; i < numPoints(); ++i )
  {
    points.push_back( pointN( i ) );
  }
  line->setPoints( points );
  return line;
}

int QgsCircularStringV2::numPoints() const
{
  return qMin( mX.size(), mY.size() );
}

QgsPointV2 QgsCircularStringV2::pointN( int i ) const
{
  if ( qMin( mX.size(), mY.size() ) <= i )
  {
    return QgsPointV2();
  }

  bool hasZ = is3D();
  bool hasM = isMeasure();

  if ( !hasZ && !hasM )
  {
    return QgsPointV2( mX.at( i ), mY.at( i ) );
  }
  else if ( hasZ && hasM )
  {
    return QgsPointV2( mX.at( i ), mY.at( i ), mZ.at( i ), mM.at( i ) );
  }
  else
  {
    return QgsPointV2( mX.at( i ), mY.at( i ), hasM ? mM.at( i ) : mZ.at( i ), hasM );
  }
}
