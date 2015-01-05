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
#include "qgscoordinatetransform.h"
#include "qgslinestringv2.h"
#include "qgsmaptopixel.h"
#include "qgspointv2.h"
#include "qgswkbptr.h"
#include <QPainter>
#include <QPainterPath>

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
}

void QgsCircularStringV2::fromWkt( const QString& wkt )
{
}

QgsRectangle QgsCircularStringV2::calculateBoundingBox() const
{
  QgsRectangle bbox;
  int nPoints = numPoints();
  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    if ( i == 0 )
    {
      bbox = segmentBoundingBox( QgsPointV2( mX[i], mY[i] ), QgsPointV2( mX[i + 1], mY[i + 1] ), QgsPointV2( mX[i + 2], mY[i + 2] ) );
    }
    else
    {
      QgsRectangle segmentBox = segmentBoundingBox( QgsPointV2( mX[i], mY[i] ), QgsPointV2( mX[i + 1], mY[i + 1] ), QgsPointV2( mX[i + 2], mY[i + 2] ) );
      bbox.combineExtentWith( &segmentBox );
    }
  }
  return bbox;
}

QgsRectangle QgsCircularStringV2::segmentBoundingBox( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3 )
{
  double centerX, centerY, radius;
  circleCenterRadius( pt1, pt2, pt3, radius, centerX, centerY );

  double p1Angle = ccwAngle( pt1.y() - centerY, pt1.x() - centerX ) + 90;
  if ( p1Angle > 360 )
  {
    p1Angle -= 360;
  }
  double p2Angle = ccwAngle( pt2.y() - centerY, pt2.x() - centerX ) + 90;
  if ( p2Angle > 360 )
  {
    p2Angle -= 360;
  }
  double p3Angle = ccwAngle( pt3.y() - centerY, pt3.x() - centerX ) + 90;
  if ( p3Angle > 360 )
  {
    p3Angle -= 360;
  }

  //start point, end point and compass points in between can be on bounding box
  QgsRectangle bbox( pt1.x(), pt1.y(), pt1.x(), pt1.y() );
  bbox.combineExtentWith( pt3.x(), pt3.y() );

  QList<QgsPointV2> compassPoints = compassPointsOnSegment( p1Angle, p2Angle, p3Angle, centerX, centerY, radius );
  QList<QgsPointV2>::const_iterator cpIt = compassPoints.constBegin();
  for ( ; cpIt != compassPoints.constEnd(); ++cpIt )
  {
    bbox.combineExtentWith( cpIt->x(), cpIt->y() );
  }
  return bbox;
}

QList<QgsPointV2> QgsCircularStringV2::compassPointsOnSegment( double p1Angle, double p2Angle, double p3Angle, double centerX, double centerY, double radius )
{
  QList<QgsPointV2> pointList;

  QgsPointV2 nPoint( centerX, centerY + radius );
  QgsPointV2 ePoint( centerX + radius, centerY );
  QgsPointV2 sPoint( centerX, centerY - radius );
  QgsPointV2 wPoint( centerX - radius, centerY );

  if ( p3Angle >= p1Angle )
  {
    if ( p2Angle > p1Angle && p2Angle < p3Angle )
    {
      if ( p1Angle <= 90 && p3Angle >= 90 )
      {
        pointList.append( ePoint );
      }
      if ( p1Angle <= 180 && p3Angle >= 180 )
      {
        pointList.append( sPoint );
      }
      if ( p1Angle <= 270 && p3Angle >= 270 )
      {
        pointList.append( wPoint );
      }
    }
    else
    {
      pointList.append( nPoint );
      if ( p1Angle >= 90 || p3Angle <= 90 )
      {
        pointList.append( ePoint );
      }
      if ( p1Angle >= 180 || p3Angle <= 180 )
      {
        pointList.append( ePoint );
      }
      if ( p1Angle >= 270 || p3Angle <= 270 )
      {
        pointList.append( ePoint );
      }
    }
  }
  else
  {
    if ( p2Angle < p1Angle && p2Angle > p3Angle )
    {
      if ( p1Angle >= 270 && p3Angle <= 270 )
      {
        pointList.append( wPoint );
      }
      if ( p1Angle >= 180 && p3Angle <= 180 )
      {
        pointList.append( sPoint );
      }
      if ( p1Angle >= 90 && p3Angle <= 90 )
      {
        pointList.append( ePoint );
      }
    }
    else
    {
      pointList.append( nPoint );
      if ( p1Angle <= 270 || p3Angle >= 270 )
      {
        pointList.append( wPoint );
      }
      if ( p1Angle <= 180 || p3Angle >= 180 )
      {
        pointList.append( sPoint );
      }
      if ( p1Angle <= 90 || p3Angle >= 90 )
      {
        pointList.append( ePoint );
      }
    }
  }
  return pointList;
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
  QString wkt( "CIRCULARSTRING(" );

  bool hasZ = is3D();
  bool hasM = isMeasure();

  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    if ( i > 0 )
    {
      wkt.append( "," );
    }
    wkt.append( qgsDoubleToString( mX[i], precision ) );
    wkt.append( " " );
    wkt.append( qgsDoubleToString( mY[i], precision ) );
    wkt.append( " " );
    if ( hasZ )
    {
      wkt.append( qgsDoubleToString( mZ[i], precision ) );
      wkt.append( " " );
    }
    if ( hasM )
    {
      wkt.append( qgsDoubleToString( mM[i], precision ) );
    }
  }

  wkt.append( ")" );
  return wkt;
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

QgsLineStringV2* QgsCircularStringV2::curveToLine() const
{
  QgsLineStringV2* line = new QgsLineStringV2();
  QList<QgsPointV2> points;
  int nPoints = numPoints();

  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
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

void QgsCircularStringV2::points( QList<QgsPointV2>& pts ) const
{
  pts.clear();
  int nPts = numPoints();
  for ( int i = 0; i < nPts; ++i )
  {
    pts.push_back( pointN( i ) );
  }
}

void QgsCircularStringV2::setPoints( const QList<QgsPointV2>& points )
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

  setZMTypeFromSubGeometry( &firstPt, QGis::WKBCircularString );

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
    double& centerX, double& centerY )
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

void QgsCircularStringV2::draw( QPainter& p ) const
{
  QPainterPath path;
  addToPainterPath( path );
  p.drawPath( path );
}

void QgsCircularStringV2::transform( const QgsCoordinateTransform& ct )
{
  double* zArray = mZ.data();

  bool hasZ = is3D();
  int nPoints = numPoints();
  if ( !hasZ )
  {
    zArray = new double[nPoints];
    for ( int i = 0; i < nPoints; ++i )
    {
      zArray[i] = 0;
    }
  }
  ct.transformCoords( nPoints, mX.data(), mY.data(), zArray );
  if ( !hasZ )
  {
    delete[] zArray;
  }
}

void QgsCircularStringV2::mapToPixel( const QgsMapToPixel& mtp )
{
  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    mtp.transformInPlace( mX[i], mY[i] );
  }
}

void QgsCircularStringV2::translate( double dx, double dy, double dz, double dm )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();
  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    mX[i] += dx;
    mY[i] += dy;
    if ( hasZ )
    {
      mZ[i] += dz;
    }
    if ( hasM )
    {
      mM[i] += dm;
    }
  }
}

void QgsCircularStringV2::clip( const QgsRectangle& rect )
{

}

void QgsCircularStringV2::addToPainterPath( QPainterPath& path ) const
{
  int nPoints = numPoints();
  if ( nPoints < 1 )
  {
    return;
  }

  if ( path.isEmpty() || path.currentPosition() != QPointF( mX[0], mY[0] ) )
  {
    path.moveTo( QPointF( mX[0], mY[0] ) );
  }

  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    QList<QgsPointV2> pt;
    segmentize( QgsPointV2( mX[i], mY[i] ), QgsPointV2( mX[i + 1], mY[i + 1] ), QgsPointV2( mX[i + 2], mY[i + 2] ), pt );
    for ( int j = 1; j < pt.size(); ++j )
    {
      path.lineTo( pt.at( j ).x(), pt.at( j ).y() );
    }
    //arcTo( path, QPointF( mX[i], mY[i] ), QPointF( mX[i + 1], mY[i + 1] ), QPointF( mX[i + 2], mY[i + 2] ) );
  }
}

void QgsCircularStringV2::arcTo( QPainterPath& path, const QPointF& pt1, const QPointF& pt2, const QPointF& pt3 )
{
  double centerX, centerY, radius;
  circleCenterRadius( QgsPointV2( pt1.x(), pt1.y() ), QgsPointV2( pt2.x(), pt2.y() ), QgsPointV2( pt3.x(), pt3.y() ),
                      radius, centerX, centerY );

  double p1Angle = ccwAngle( pt1.y() - centerY, pt1.x() - centerX );
  double p2Angle = ccwAngle( pt2.y() - centerY, pt2.x() - centerX );
  double p3Angle = ccwAngle( pt3.y() - centerY, pt3.x() - centerX );

  //clockwise or counterclockwise?
  double sweepAngle = 0;
  if ( p3Angle >= p1Angle )
  {
    if ( p2Angle > p1Angle && p2Angle < p3Angle )
    {
      sweepAngle = p3Angle - p1Angle;
    }
    else
    {
      sweepAngle = - ( p1Angle + ( 360 - p3Angle ) );
    }
  }
  else
  {
    if ( p2Angle < p1Angle && p2Angle > p3Angle )
    {
      sweepAngle = -( p1Angle - p3Angle );
    }
    else
    {
      sweepAngle = p3Angle + ( 360 - p1Angle );
    }
  }

  double diameter = 2 * radius;
  path.arcTo( centerX - radius, centerY - radius, diameter, diameter, p1Angle, sweepAngle );
}

double QgsCircularStringV2::ccwAngle( double dy, double dx )
{
  double angle = atan2( dy, dx ) * 180 / M_PI;
  if ( angle < 0 )
  {
    return -angle;
  }
  return 360 - angle;
}

void QgsCircularStringV2::drawAsPolygon( QPainter& p ) const
{
  draw( p );
}

bool QgsCircularStringV2::insertVertex( const QgsVertexId& position, const QgsPointV2& vertex )
{
  if ( position.vertex > mX.size() )
  {
    return false;
  }

  mX.insert( position.vertex, vertex.x() );
  mY.insert( position.vertex, vertex.y() );
  if ( is3D() )
  {
    mZ.insert( position.vertex, vertex.z() );
  }
  if ( isMeasure() )
  {
    mM.insert( position.vertex, vertex.m() );
  }
  return true;
}

bool QgsCircularStringV2::moveVertex( const QgsVertexId& position, const QgsPointV2& newPos )
{
  if ( position.vertex > mX.size() )
  {
    return false;
  }

  mX[position.vertex] = newPos.x();
  mY[position.vertex] = newPos.y();
  if ( is3D() )
  {
    mZ[position.vertex] = newPos.z();
  }
  if ( isMeasure() )
  {
    mM[position.vertex] = newPos.m();
  }
  mBoundingBox = QgsRectangle(); //set bounding box invalid
  return true;
}

bool QgsCircularStringV2::deleteVertex( const QgsVertexId& position )
{
  if ( position.vertex > mX.size() )
  {
    return false;
  }

  mX.remove( position.vertex );
  mY.remove( position.vertex );
  if ( is3D() )
  {
    mZ.remove( position.vertex );
  }
  if ( isMeasure() )
  {
    mM.remove( position.vertex );
  }
  return true;
}
