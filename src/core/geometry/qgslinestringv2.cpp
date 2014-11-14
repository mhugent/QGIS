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
#include "qgscoordinatetransform.h"
#include "qgsmaptopixel.h"
#include "qgswkbptr.h"
#include <QPainter>

QgsLineStringV2::QgsLineStringV2(): QgsCurveV2()
{
}

QgsLineStringV2::~QgsLineStringV2()
{}

QgsAbstractGeometryV2* QgsLineStringV2::clone() const
{
  return new QgsLineStringV2( *this );
}

void QgsLineStringV2::fromWkb( const unsigned char* wkb )
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
  importVerticesFromWkb( wkbPtr );
}

int QgsLineStringV2::wkbSize() const
{
  int binarySize = 1 + 2 * sizeof( int ) + numPoints() * 2 * sizeof( double );
  if ( is3D() )
  {
    binarySize += ( mZ.size() * sizeof( double ) );
  }
  if ( isMeasure() )
  {
    binarySize += ( mM.size() * sizeof( double ) );
  }
  return binarySize;
}

void QgsLineStringV2::fromWkt( const QString& wkt )
{
  //todo...
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
  binarySize = wkbSize();
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
  return QString(); //todo...
}

double QgsLineStringV2::length() const
{
  return 0; //todo...
}

QgsPointV2 QgsLineStringV2::startPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( 0 );
}

QgsPointV2 QgsLineStringV2::endPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( numPoints() - 1 );
}

bool QgsLineStringV2::isClosed() const
{
  return ( numPoints() > 0 && pointN( 0 ) == pointN( numPoints() - 1 ) );
}

bool QgsLineStringV2::isRing() const
{
  return false;
}

QgsLineStringV2* QgsLineStringV2::curveToLine() const
{
  return dynamic_cast<QgsLineStringV2*>( clone() );
}

int QgsLineStringV2::numPoints() const
{
  return mCoords.size();
}

QgsPointV2 QgsLineStringV2::pointN( int i ) const
{
  if ( mCoords.size() <= i )
  {
    return QgsPointV2();
  }

  const QPointF& pt = mCoords.at( i );
  bool hasZ = is3D();
  bool hasM = isMeasure();
  if ( !hasZ && !hasM )
  {
    return QgsPointV2( pt.x(), pt.y() );
  }
  else if ( hasZ && hasM )
  {
    return QgsPointV2( pt.x(), pt.x(), mZ.at( i ), mM.at( i ) );
  }
  else
  {
    return QgsPointV2( pt.x(), pt.y(), hasM ? mM.at( i ) : mZ.at( i ), hasM );
  }
}

void QgsLineStringV2::points( QList<QgsPointV2>& pts ) const
{
  pts.clear();
  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    pts.push_back( pointN( i ) );
  }
}

void QgsLineStringV2::setPoints( const QList<QgsPointV2>& points )
{
  if ( points.size() < 1 )
  {
    mWkbType = QGis::WKBUnknown;
    mCoords.clear();
    mZ.clear();
    mM.clear();
    return;
  }

  //get wkb type from first point
  const QgsPointV2& firstPt = points.at( 0 );
  bool hasZ = firstPt.is3D();
  bool hasM = firstPt.isMeasure();

  if ( hasZ && hasM )
  {
    mWkbType = QGis::WKBLineStringZM;
  }
  else if ( hasZ )
  {
    mWkbType = QGis::WKBLineStringZ;
  }
  else if ( hasM )
  {
    mWkbType = QGis::WKBLineStringM;
  }
  else
  {
    mWkbType = QGis::WKBLineString;
  }

  mCoords.resize( points.size() );
  if ( hasZ )
  {
    mZ.resize( points.size() );
  }
  else
  {
    mZ.clear();
  }
  if ( hasM )
  {
    mM.resize( points.size() );
  }
  else
  {
    mM.clear();
  }

  for ( int i = 0; i < points.size(); ++i )
  {
    mCoords[i].rx() = points[i].x();
    mCoords[i].ry() = points[i].y();
    if ( hasZ )
    {
      mZ[i] = points[i].z();
    }
    if ( hasM )
    {
      mM[i] = points[i].m();
    }
  }
}

void QgsLineStringV2::append( const QgsLineStringV2* line )
{
  if ( !line )
  {
    return;
  }

  mCoords += line->mCoords;
  mZ += line->mZ;
  mM += line->mM;
}

QgsRectangle QgsLineStringV2::calculateBoundingBox() const
{
  QRectF rect = mCoords.boundingRect();
  return QgsRectangle( rect.left(), rect.top(), rect.right(), rect.bottom() );
}

void QgsLineStringV2::fromWkbPoints( QGis::WkbType type, const QgsConstWkbPtr& wkb )
{
  mWkbType = type;
  importVerticesFromWkb( wkb );
}

void QgsLineStringV2::importVerticesFromWkb( const QgsConstWkbPtr& wkb )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();

  int nVertices = 0;
  wkb >> nVertices;
  mCoords.resize( nVertices );
  hasZ ? mZ.resize( nVertices ) : mZ.clear();
  hasM ? mM.resize( nVertices ) : mM.clear();

  for ( int i = 0; i < nVertices; ++i )
  {
    wkb >> mCoords[i].rx();
    wkb >> mCoords[i].ry();
    if ( hasZ )
    {
      wkb >> mZ[i];
    }
    if ( hasM )
    {
      wkb >> mM[i];
    }
  }
}

void QgsLineStringV2::draw( QPainter& p ) const
{
  p.drawPolyline( mCoords );
}

void QgsLineStringV2::addToPainterPath( QPainterPath& path ) const
{
  path.addPolygon( mCoords );
}

void QgsLineStringV2::drawAsPolygon( QPainter& p ) const
{
  p.drawPolygon( mCoords );
}

void QgsLineStringV2::transform( const QgsCoordinateTransform& ct )
{
  ct.transformPolygon( mCoords );
}

void QgsLineStringV2::mapToPixel( const QgsMapToPixel& mtp )
{
  int nVertices = mCoords.size();
  for ( int i = 0; i < nVertices; ++i )
  {
    mtp.transformInPlace( mCoords[i].rx(), mCoords[i].ry() );
  }
}
