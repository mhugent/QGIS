/***************************************************************************
                         qgspointv2.cpp
                         --------------
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


#include "qgspointv2.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsmaptopixel.h"
#include "qgswkbptr.h"

QgsPointV2::QgsPointV2( double x, double y ): QgsAbstractGeometryV2(), mX( x ), mY( y ), mZ( 0.0 ), mM( 0.0 )
{
  mWkbType = QGis::WKBPoint;
}

QgsPointV2::QgsPointV2( double x, double y, double mz, bool hasM ): QgsAbstractGeometryV2(),
    mX( x ), mY( y ), mZ( hasM ? 0.0 : mz ), mM( hasM ? mz : 0.0 )
{
  mWkbType = hasM ? QGis::WKBPointM : QGis::WKBPointZ;
}

QgsPointV2::QgsPointV2( double x, double y, double z, double m ): QgsAbstractGeometryV2(), mX( x ), mY( y ), mZ( z ), mM( m )
{
  mWkbType = QGis::WKBPointZM;
}

QgsPointV2::~QgsPointV2()
{}

bool QgsPointV2::operator==( const QgsPointV2& pt ) const
{
  return ( pt.wkbType() == wkbType() && qgsDoubleNear( pt.x(), mX ) && qgsDoubleNear( pt.y(), mY ) &&
           qgsDoubleNear( pt.z(), mZ ) && qgsDoubleNear( pt.m(), mM ) );
}

QgsAbstractGeometryV2* QgsPointV2::clone() const
{
  return new QgsPointV2( *this );
}

int QgsPointV2::wkbSize() const
{
  int binarySize = 1 + sizeof( int ) + sizeof( double );
  if ( is3D() > 2 )
  {
    binarySize += sizeof( double );
  }
  if ( isMeasure() )
  {
    binarySize += sizeof( double );
  }
  return binarySize;
}

void QgsPointV2::fromWkb( const unsigned char* wkb )
{
  reset();
  if ( !wkb )
  {
    return;
  }

  QgsConstWkbPtr wkbPtr( wkb + 1 );
  wkbPtr >> mWkbType;


  wkbPtr >> mX;
  wkbPtr >> mY;
  if ( is3D() )
  {
    wkbPtr >> mZ;
  }
  if ( isMeasure() )
  {
    wkbPtr >> mM;
  }
}

void QgsPointV2::fromWkt( const QString& wkt )
{
  reset();

  //todo: better with regexp
  QString text = wkt;
  text.remove( "POINT(" );
  text.chop( 1 );
  QStringList coords = text.split( " ", QString::SkipEmptyParts );
  if ( coords.size() < 2 )
  {
    return;
  }
  mX = coords.at( 0 ).toDouble();
  mY = coords.at( 1 ).toDouble();

  if ( coords.size() > 2 )
  {
    mWkbType = QGis::WKBPointZ;
    mZ = coords.at( 2 ).toDouble();
  }
  if ( coords.size() > 3 )
  {
    mWkbType = QGis::WKBPointZM;
    mM = coords.at( 3 ).toDouble();
  }
  else
  {
    mWkbType = QGis::WKBPoint;
  }
}

QString QgsPointV2::asText( int precision ) const
{
  QString wkt = "POINT(" + qgsDoubleToString( mX, precision ) + " " + qgsDoubleToString( mY, precision );
  if ( is3D() )
  {
    wkt.append( "" );
    wkt.append( qgsDoubleToString( mZ, precision ) );
  }
  if ( isMeasure() )
  {
    wkt.append( "" );
    wkt.append( qgsDoubleToString( mM, precision ) );
  }
  wkt.append( ")" );
  return wkt;
}

unsigned char* QgsPointV2::asBinary( int& binarySize ) const
{
  binarySize = wkbSize();
  char byteOrder = QgsApplication::endian();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();

  wkb << mX << mY;
  if ( is3D() > 2 )
  {
    wkb << mZ;
  }
  if ( isMeasure() )
  {
    wkb << mM;
  }

  return geomPtr;
}

QString QgsPointV2::asGML() const
{
  return QString();
}

void QgsPointV2::reset()
{
  mWkbType = QGis::WKBUnknown;
  mX = 0; mY = 0; mZ = 0; mM = 0;
}

void QgsPointV2::transform( const QgsCoordinateTransform& ct )
{
  ct.transformInPlace( mX, mY, mZ );
}

void QgsPointV2::mapToPixel( const QgsMapToPixel& mtp )
{
  mtp.transformInPlace( mX, mY );
}

void QgsPointV2::translate( double dx, double dy, double dz, double dm )
{
  mX += dx;
  mY += dy;
  if ( is3D() )
  {
    mZ += dz;
  }
  if ( isMeasure() )
  {
    mM += dm;
  }
}

void QgsPointV2::coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const
{
  coord.clear();
  QList< QList< QgsPointV2 > > featureCoord;
  featureCoord.append( QList< QgsPointV2 >() << QgsPointV2( *this ) );
  coord.append( featureCoord );
}
