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
  return new QgsCircularStringV2( *this );
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
  geometryChanged();
}

void QgsCircularStringV2::fromWkt( const QString& wkt )
{
  geometryChanged();
}

int QgsCircularStringV2::wkbSize() const
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
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( 0 );
}

QgsPointV2 QgsCircularStringV2::endPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( numPoints() - 1 );
}

bool QgsCircularStringV2::isClosed() const
{
  return ( numPoints() > 0 && pointN( 0 ) == pointN( numPoints() - 1 ) );
}

bool QgsCircularStringV2::isRing() const
{
  return false; //soon...
}

QgsLineStringV2* QgsCircularStringV2::curveToLine() const
{
  QgsLineStringV2* line = new QgsLineStringV2();
  QList<QgsPointV2> points;

  for ( int i = 0; i < ( numPoints() - 2 ) ; i += 2 )
  {
    segmentize( pointN( i ), pointN( i + 1 ), pointN( i + 2 ), points );
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

QList<QgsPointV2> QgsCircularStringV2::points() const
{
  QList<QgsPointV2> pts;
  int nPts = numPoints();
  for ( int i = 0; i < nPts; ++i )
  {
    pts.push_back( pointN( i ) );
  }
  return pts;
}

void QgsCircularStringV2::setPoints( const QList<QgsPointV2> points )
{
  if ( points.size() < 1 )
  {
    mWkbType = QGis::WKBUnknown; mX.clear(); mY.clear(); mZ.clear(); mM.clear();
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

  mX.resize( points.size() );
  mY.resize( points.size() );
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
    mX[i] = points[i].x();
    mY[i] = points[i].y();
    if ( hasZ )
    {
      mZ[i] = points[i].z();
    }
    if ( hasM )
    {
      mM[i] = points[i].m();
    }
  }
  geometryChanged();
}

void QgsCircularStringV2::segmentize( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& p3, QList<QgsPointV2>& points ) const
{
  //adapted code from postgis
  double radius = 0;
  double centerX = 0;
  double centerY = 0;
  circleCenterRadius( p1, p2, p3, radius, centerX, centerY );
  int segSide = segmentSide( p1, p3, p2 );

  if ( radius < 0 || qgsDoubleNear( segSide, 0.0 ) ) //points are colinear
  {
    points.append( p1 );
    points.append( p2 );
    points.append( p3 );
  }

  bool clockwise = false;
  if ( segSide == -1 )
  {
    clockwise = true;
  }

  double increment = fabs( M_PI_2 / 90 ); //one segment per degree

  //angles of pt1, pt2, pt3
  double a1 = atan2( p1.y() - centerY, p1.x() - centerX );
  double a2 = atan2( p2.y() - centerY, p2.x() - centerX );
  double a3 = atan2( p3.y() - centerY, p3.x() - centerX );

  if ( clockwise )
  {
    increment *= -1;
    /* Adjust a3 down so we can decrement from a1 to a3 cleanly */
    if ( a3 > a1 )
      a3 -= 2.0 * M_PI;
    if ( a2 > a1 )
      a2 -= 2.0 * M_PI;
  }
  else
  {
    /* Adjust a3 up so we can increment from a1 to a3 cleanly */
    if ( a3 < a1 )
      a3 += 2.0 * M_PI;
    if ( a2 < a1 )
      a2 += 2.0 * M_PI;
  }

  bool hasZ = is3D();
  bool hasM = isMeasure();

  double x, y, z, m;
  points.append( p1 );
  for ( double angle = a1 + increment; clockwise ? angle > a3 : angle < a3; angle += increment )
  {
    x = centerX + radius * cos( angle );
    y = centerY + radius * sin( angle );

    if ( !hasZ && !hasM )
    {
      points.append( QgsPointV2( x, y ) );
      continue;
    }

    if ( hasZ )
    {
      z = interpolateArc( angle, a1, a2, a3, p1.z(), p2.z(), p3.z() );
    }
    if ( hasM )
    {
      m = interpolateArc( angle, a1, a2, a3, p1.m(), p2.m(), p3.m() );
    }

    if ( hasZ && hasM )
    {
      points.append( QgsPointV2( x, y, z, m ) );
    }
    else if ( hasZ )
    {
      points.append( QgsPointV2( x, y, z, false ) );
    }
    else
    {
      points.append( QgsPointV2( x, y, m, true ) );
    }
  }
  points.append( p3 );
}

void QgsCircularStringV2::circleCenterRadius( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3, double& radius,
    double& centerX, double& centerY ) const
{
  double temp, bc, cd, det;

  //closed circle
  if ( qgsDoubleNear( pt1.x(), pt3.x() ) && qgsDoubleNear( pt1.y(), pt3.y() ) )
  {
    centerX = pt2.x();
    centerY = pt2.y();
    radius = sqrt( pow( pt2.x() - pt1.x(), 2.0 ) + pow( pt2.y() - pt1.y(), 2.0 ) );
    return;
  }

  temp = pt2.x() * pt2.x() + pt2.y() * pt2.y();
  bc = ( pt1.x() * pt1.x() + pt1.y() * pt1.y() - temp ) / 2.0;
  cd = ( temp - pt3.x() * pt3.x() - pt3.y() * pt3.y() ) / 2.0;
  det = ( pt1.x() - pt2.x() ) * ( pt2.y() - pt3.y() ) - ( pt2.x() - pt3.x() ) * ( pt1.y() - pt2.y() );

  /* Check colinearity */
  if ( qgsDoubleNear( fabs( det ), 0.0 ) )
  {
    radius = -1.0;
    return;
  }

  det = 1.0 / det;
  centerX = ( bc * ( pt2.y() - pt3.y() ) - cd * ( pt1.y() - pt2.y() ) ) * det;
  centerY = (( pt1.x() - pt2.x() ) * cd - ( pt2.x() - pt3.x() ) * bc ) * det;
  radius = sqrt(( centerX - pt1.x() ) * ( centerX - pt1.x() ) + ( centerY - pt1.y() ) * ( centerY - pt1.y() ) );
}

int QgsCircularStringV2::segmentSide( const QgsPointV2& pt1, const QgsPointV2& pt3, const QgsPointV2& pt2 ) const
{
  double side = (( pt2.x() - pt1.x() ) * ( pt3.y() - pt1.y() ) - ( pt3.x() - pt1.x() ) * ( pt2.y() - pt1.y() ) );
  if ( side == 0.0 )
  {
    return 0;
  }
  else
  {
    if ( side < 0 )
    {
      return -1;
    }
    if ( side > 0 )
    {
      return 1;
    }
    return 0;
  }
}

double QgsCircularStringV2::interpolateArc( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 ) const
{
  /* Counter-clockwise sweep */
  if ( a1 < a2 )
  {
    if ( angle <= a2 )
      return zm1 + ( zm2 - zm1 ) * ( angle - a1 ) / ( a2 - a1 );
    else
      return zm2 + ( zm3 - zm2 ) * ( angle - a2 ) / ( a3 - a2 );
  }
  /* Clockwise sweep */
  else
  {
    if ( angle >= a2 )
      return zm1 + ( zm2 - zm1 ) * ( a1 - angle ) / ( a1 - a2 );
    else
      return zm2 + ( zm3 - zm2 ) * ( a2 - angle ) / ( a2 - a3 );
  }
}
