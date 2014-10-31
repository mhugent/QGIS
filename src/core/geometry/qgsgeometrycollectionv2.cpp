/***************************************************************************
                        qgsgeometrycollectionv2.cpp
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
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

#include "qgsgeometrycollectionv2.h"
#include "qgsapplication.h"
#include "qgsgeometryimport.h"
#include "qgswkbptr.h"

QgsGeometryCollectionV2::QgsGeometryCollectionV2(): QgsAbstractGeometryV2()
{

}

QgsGeometryCollectionV2::QgsGeometryCollectionV2( const QgsGeometryCollectionV2& c ): QgsAbstractGeometryV2( c )
{
  int nGeoms = c.mGeometries.size();
  mGeometries.resize( nGeoms );
  for ( int i = 0; i < nGeoms; ++i )
  {
    mGeometries[i] = c.mGeometries.at( i )->clone();
  }
}

QgsGeometryCollectionV2& QgsGeometryCollectionV2::operator=( const QgsGeometryCollectionV2 & c )
{
  if ( &c != this )
  {
    QgsAbstractGeometryV2::operator=( c );
    removeGeometries();
    int nGeoms = c.mGeometries.size();
    mGeometries.resize( nGeoms );
    for ( int i = 0; i < nGeoms; ++i )
    {
      mGeometries[i] = c.mGeometries.at( i )->clone();
    }
  }
  return *this;
}

QgsGeometryCollectionV2::~QgsGeometryCollectionV2()
{
  removeGeometries();
}

void QgsGeometryCollectionV2::removeGeometries()
{
  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    delete *it;
  }
  mGeometries.clear();
}

int QgsGeometryCollectionV2::numGeometries() const
{
  return mGeometries.size();
}

const QgsAbstractGeometryV2* QgsGeometryCollectionV2::geometryN( int n ) const
{
  if ( n >= mGeometries.size() )
  {
    return 0;
  }
  return mGeometries.at( n );
}

int QgsGeometryCollectionV2::dimension() const
{
  int maxDim = 0;
  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    int dim = ( *it )->dimension();
    if ( dim > maxDim )
    {
      maxDim = dim;
    }
  }
  return maxDim;
}

QgsRectangle QgsGeometryCollectionV2::calculateBoundingBox() const
{
  if ( mGeometries.size() < 1 )
  {
    return QgsRectangle();
  }

  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  QgsRectangle bbox = ( *it )->calculateBoundingBox();
  ++it;
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    QgsRectangle box = ( *it )->calculateBoundingBox();
    bbox.combineExtentWith( &box );
  }
  return bbox;
}

void QgsGeometryCollectionV2::transform( const QgsCoordinateTransform& ct )
{
  QVector< QgsAbstractGeometryV2* >::iterator it = mGeometries.begin();
  for ( ; it != mGeometries.end(); ++it )
  {
    ( *it )->transform( ct );
  }
}

void QgsGeometryCollectionV2::mapToPixel( const QgsMapToPixel& mtp )
{
  QVector< QgsAbstractGeometryV2* >::iterator it = mGeometries.begin();
  for ( ; it != mGeometries.end(); ++it )
  {
    ( *it )->mapToPixel( mtp );
  }
}

void QgsGeometryCollectionV2::clip( const QgsRectangle& rect )
{
  QVector< QgsAbstractGeometryV2* >::iterator it = mGeometries.begin();
  for ( ; it != mGeometries.end(); ++it )
  {
    ( *it )->clip( rect );
  }
}

void QgsGeometryCollectionV2::draw( QPainter& p ) const
{
  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    ( *it )->draw( p );
  }
}

void QgsGeometryCollectionV2::fromWkb( const unsigned char * wkb )
{
  if ( !wkb )
  {
    return;
  }

  QgsConstWkbPtr wkbPtr( wkb + 1 );

  //type
  wkbPtr >> mWkbType;

  int nGeometries = 0;
  wkbPtr >> nGeometries;
  mGeometries.resize( nGeometries );

  for ( int i = 0; i < nGeometries; ++i )
  {
    QgsAbstractGeometryV2* geom = QgsGeometryImport::geomFromWkb( wkbPtr );
    mGeometries[i] = geom;
    wkbPtr += geom->wkbSize();
  }
}

unsigned char* QgsGeometryCollectionV2::asBinary( int& binarySize ) const
{
  QList< QPair< unsigned char*, int > > geometryWkb;
  int currentGeomWkbSize = 0;
  QVector< QgsAbstractGeometryV2* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    geometryWkb.push_back( qMakePair(( *geomIt )->asBinary( currentGeomWkbSize ), currentGeomWkbSize ) );
  }

  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  char byteOrder = QgsApplication::endian();
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();
  wkb << mGeometries.size();

  QList< QPair< unsigned char*, int > >::const_iterator wkbIt = geometryWkb.constBegin();
  for ( ; wkbIt != geometryWkb.constEnd(); ++wkbIt )
  {
    memcpy( wkb, wkbIt->first, wkbIt->second );
    wkb += wkbIt->second;
    delete wkbIt->first;
  }

  return geomPtr;
}

int QgsGeometryCollectionV2::wkbSize() const
{
  int size = 0;
  QVector< QgsAbstractGeometryV2* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    size += ( *geomIt )->wkbSize();
  }
  size += ( 1 + 2 * sizeof( int ) );
  return size;
}
