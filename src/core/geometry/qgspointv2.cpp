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
#include "qgswkbptr.h"

QgsPointV2::QgsPointV2( double x, double y ): QgsAbstractGeometryV2( 2, false ), mX( x ), mY( y ), mZ( 0.0 ), mM( 0.0 )
{}

QgsPointV2::QgsPointV2( double x, double y, double mz, bool hasM ): QgsAbstractGeometryV2( hasM ? 2 : 3, false ),
    mX( x ), mY( y ), mZ( hasM ? 0.0 : mz ), mM( hasM ? mz : 0.0 )
{}

QgsPointV2::QgsPointV2( double x, double y, double z, double m ): QgsAbstractGeometryV2( 3, true ), mX( x ), mY( y ), mZ( z ), mM( m )
{}

QgsPointV2::~QgsPointV2()
{}

QGis::WkbType QgsPointV2::wkbType() const
{
  //type
  if ( mCoordDimension > 2 && mHasM )
  {
    return QGis::WKBPointZM;
  }
  else if ( mCoordDimension > 2 )
  {
    return QGis::WKBPointZ;
  }
  else if ( mHasM )
  {
    return QGis::WKBPointM;
  }
  else
  {
    return QGis::WKBPoint;
  }
}

QgsAbstractGeometryV2* QgsPointV2::clone() const
{
  switch ( mCoordDimension )
  {
    case 3:
      return mHasM ? new QgsPointV2( mX, mY, mZ, mM ) : new QgsPointV2( mX, mY, mZ, false );
      break;
    default:
      return mHasM ? new QgsPointV2( mX, mY, mM, true ) : new QgsPointV2( mX, mY );
  }
}

void QgsPointV2::fromWkb( const unsigned char * wkb, size_t length )
{
  if ( length < ( 1 + sizeof( int ) ) )
  {
    mCoordDimension = 2; mHasM = false; mX = 0; mY = 0; mZ = 0; mM = 0;
  }

  QgsConstWkbPtr wkbPtr( wkb + 1 );
  int wkbtype;
  wkbPtr >> wkbtype;

  if ( wkbtype == QGis::WKBPoint )
  {
    mCoordDimension = 2; mHasM = false;
  }
  else if ( wkbtype == QGis::WKBPointZ )
  {
    mCoordDimension = 3; mHasM = false;
  }
  else if ( wkbtype == QGis::WKBPointM )
  {
    mCoordDimension = 2; mHasM = true;
  }
  else if ( wkbtype == QGis::WKBPointZM )
  {
    mCoordDimension = 3; mHasM = true;
  }
  else
  {
    mCoordDimension = 2; mHasM = false; mX = 0; mY = 0; mZ = 0; mM = 0;
    return;
  }

  wkbPtr >> mX;
  wkbPtr >> mY;
  if ( mCoordDimension > 2 )
  {
    wkbPtr >> mZ;
  }
  else
  {
    mZ = 0;
  }
  if ( mHasM )
  {
    wkbPtr >> mM;
  }
  else
  {
    mM = 0;
  }
}

void QgsPointV2::fromGeos( GEOSGeometry* geos )
{
  //soon
}

void QgsPointV2::fromWkt( const QString& wkt )
{
  //better with regexp
  QString text = wkt;
  text.remove( "POINT(" );
  text.chop( 1 );
  QStringList coords = text.split( " ", QString::SkipEmptyParts );
  if ( coords.size() < 2 )
  {
    return; //error
  }
  mX = coords.at( 0 ).toDouble();
  mY = coords.at( 1 ).toDouble();
  mCoordDimension = 2;
  mHasM = false;

  if ( coords.size() > 2 )
  {
    mZ = coords.at( 2 ).toDouble();
    mCoordDimension = 3;
  }
  if ( coords.size() > 3 )
  {
    mM = coords.at( 3 ).toDouble();
    mHasM = true;
  }
}

QString QgsPointV2::asText( int precision ) const
{
  QString wkt = "POINT(" + qgsDoubleToString( mX, precision ) + " " + qgsDoubleToString( mY, precision );
  if ( mCoordDimension > 2 )
  {
    wkt.append( "" );
    wkt.append( qgsDoubleToString( mZ, precision ) );
  }
  if ( mHasM )
  {
    wkt.append( "" );
    wkt.append( qgsDoubleToString( mM, precision ) );
  }
  wkt.append( ")" );
  return QString(); //soon...
}

unsigned char* QgsPointV2::asBinary( int& binarySize ) const
{
  binarySize = 1 + sizeof( int ) + sizeof( double );
  if ( mCoordDimension > 2 )
  {
    binarySize += sizeof( double );
  }
  if ( mHasM )
  {
    binarySize += sizeof( double );
  }

  char byteOrder = QgsApplication::endian();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();



  wkb << mX << mY;
  if ( mCoordDimension > 2 )
  {
    wkb << mZ;
  }
  if ( mHasM )
  {
    wkb << mM;
  }

  return geomPtr;
}

QString QgsPointV2::asGML() const
{
  return QString();
}
