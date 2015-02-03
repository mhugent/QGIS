/***************************************************************************
                        qgsgeometryutils.cpp
  -------------------------------------------------------------------
Date                 : 21 Nov 2014
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

#include "qgsgeometryutils.h"

QgsPointV2 QgsGeometryUtils::closestVertex( const QgsAbstractGeometryV2& geom, const QgsPointV2& pt, QgsVertexId& id )
{
  double minDist = std::numeric_limits<double>::max();
  double currentDist = 0;
  QgsPointV2 minDistPoint;

  QgsVertexId vertexId;
  QgsPointV2 vertex;
  while ( geom.nextVertex( vertexId, vertex ) )
  {
    currentDist = QgsGeometryUtils::sqrDistance2D( pt, vertex );
    if ( currentDist < minDist )
    {
      minDist = currentDist;
      minDistPoint = vertex;
      id.part = vertexId.part;
      id.ring = vertexId.ring;
      id.vertex = vertexId.vertex;
    }
  }

  return minDistPoint;
}

void QgsGeometryUtils::adjacentVertices( const QgsAbstractGeometryV2& geom, const QgsVertexId& atVertex, QgsVertexId& beforeVertex, QgsVertexId& afterVertex )
{
  bool polygonType = ( geom.dimension()  == 2 );
  QList< QList< QList< QgsPointV2 > > > coords;
  geom.coordinateSequence( coords );

  //get feature
  if ( coords.size() <= atVertex.part )
  {
    return; //error, no such feature
  }
  const QList< QList< QgsPointV2 > >& part = coords.at( atVertex.part );

  //get ring
  if ( part.size() <= atVertex.ring )
  {
    return; //error, no such ring
  }
  const QList< QgsPointV2 >& ring = part.at( atVertex.ring );
  if ( ring.size() <= atVertex.vertex )
  {
    return;
  }

  //vertex in the middle
  if ( atVertex.vertex > 0 && atVertex.vertex < ring.size() - 1 )
  {
    beforeVertex.part = atVertex.part; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = atVertex.vertex - 1;
    afterVertex.part = atVertex.part; afterVertex.ring = atVertex.ring; afterVertex.vertex = atVertex.vertex + 1;
  }
  else if ( atVertex.vertex == 0 )
  {
    afterVertex.part = atVertex.part; afterVertex.ring = atVertex.ring; afterVertex.vertex = atVertex.vertex + 1;
    if ( polygonType && ring.size() > 3 )
    {
      beforeVertex.part = atVertex.part; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = ring.size() - 2;
    }
    else
    {
      beforeVertex = QgsVertexId(); //before vertex invalid
    }
  }
  else if ( atVertex.vertex == ring.size() - 1 )
  {
    beforeVertex.part = atVertex.part; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = atVertex.vertex - 1;
    if ( polygonType )
    {
      afterVertex.part = atVertex.part; afterVertex.ring = atVertex.ring; afterVertex.vertex = 0;
    }
    else
    {
      afterVertex = QgsVertexId(); //after vertex invalid
    }
  }
}

double QgsGeometryUtils::sqrDistance2D( const QgsPointV2& pt1, const QgsPointV2& pt2 )
{
  return ( pt1.x() - pt2.x() ) * ( pt1.x() - pt2.x() ) + ( pt1.y() - pt2.y() ) * ( pt1.y() - pt2.y() );
}

double QgsGeometryUtils::sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double& minDistX, double& minDistY, double epsilon )
{
  //normal vector
  double nx = y2 - y1;
  double ny = -( x2 - x1 );

  double t;
  t = ( ptX * ny - ptY * nx - x1 * ny + y1 * nx ) / (( x2 - x1 ) * ny - ( y2 - y1 ) * nx );

  if ( t < 0.0 )
  {
    minDistX = x1;
    minDistY = y1;
  }
  else if ( t > 1.0 )
  {
    minDistX = x2;
    minDistY = y2;
  }
  else
  {
    minDistX = x1 + t * ( x2 - x1 );
    minDistY = y1 + t * ( y2 - y1 );
  }

  double dist = ( minDistX - ptX ) * ( minDistX - ptX ) + ( minDistY - ptY ) * ( minDistY - ptY );

  //prevent rounding errors if the point is directly on the segment
  if ( qgsDoubleNear( dist, 0.0, epsilon ) )
  {
    minDistX = ptX;
    minDistY = ptY;
    return 0.0;
  }

  return dist;
}

double QgsGeometryUtils::leftOfLine( double x, double y, double x1, double y1, double x2, double y2 )
{
  double f1 = x - x1;
  double f2 = y2 - y1;
  double f3 = y - y1;
  double f4 = x2 - x1;
  return f1*f2 - f3*f4;
}

QgsPointV2 QgsGeometryUtils::pointOnLineWithDistance( const QgsPointV2& startPoint, const QgsPointV2& directionPoint, double distance )
{
  double dx = directionPoint.x() - startPoint.x();
  double dy = directionPoint.y() - startPoint.y();
  double length = sqrt( dx * dx + dy * dy );

  if ( qgsDoubleNear( length, 0.0 ) )
  {
    return startPoint;
  }

  double scaleFactor = distance / length;
  return QgsPointV2( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}

double QgsGeometryUtils::ccwAngle( double dy, double dx )
{
  double angle = atan2( dy, dx ) * 180 / M_PI;
  if ( angle < 0 )
  {
    return 360 + angle;
  }
  else if ( angle > 360 )
  {
    return 360 - angle;
  }
  return angle;
}

void QgsGeometryUtils::circleCenterRadius( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3, double& radius, double& centerX, double& centerY )
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

bool QgsGeometryUtils::circleClockwise( double angle1, double angle2, double angle3 )
{
  if ( angle3 >= angle1 )
  {
    if ( angle2 > angle1 && angle2 < angle3 )
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    if ( angle2 > angle1 || angle2 < angle3 )
    {
      return false;
    }
    else
    {
      return true;
    }
  }
}

bool QgsGeometryUtils::circleAngleBetween( double angle, double angle1, double angle2, bool clockwise )
{
  if ( clockwise )
  {
    if ( angle2 < angle1 )
    {
      return ( angle <= angle1 && angle >= angle2 );
    }
    else
    {
      return ( angle <= angle1 || angle >= angle2 );
    }
  }
  else
  {
    if ( angle2 > angle1 )
    {
      return ( angle >= angle1 && angle <= angle2 );
    }
    else
    {
      return ( angle >= angle1 || angle <= angle2 );
    }
  }
}

bool QgsGeometryUtils::angleOnCircle( double angle, double angle1, double angle2, double angle3 )
{
  bool clockwise = circleClockwise( angle1, angle2, angle3 );
  return circleAngleBetween( angle, angle1, angle3, clockwise );
}

double QgsGeometryUtils::circleLength( double x1, double y1, double x2, double y2, double x3, double y3 )
{
  double centerX, centerY, radius;
  circleCenterRadius( QgsPointV2( x1, y1 ), QgsPointV2( x2, y2 ), QgsPointV2( x3, y3 ), radius, centerX, centerY );
  return M_PI / 180.0 * radius * sweepAngle( centerX, centerY, x1, y1, x2, y2, x3, y3 );
}

double QgsGeometryUtils::sweepAngle( double centerX, double centerY, double x1, double y1, double x2, double y2, double x3, double y3 )
{
  double p1Angle = QgsGeometryUtils::ccwAngle( y1 - centerY, x1 - centerX );
  double p2Angle = QgsGeometryUtils::ccwAngle( y2 - centerY, x2 - centerX );
  double p3Angle = QgsGeometryUtils::ccwAngle( y3 - centerY, x3 - centerX );

  if ( p3Angle >= p1Angle )
  {
    if ( p2Angle > p1Angle && p2Angle < p3Angle )
    {
      return( p3Angle - p1Angle );
    }
    else
    {
      return ( - ( p1Angle + ( 360 - p3Angle ) ) );
    }
  }
  else
  {
    if ( p2Angle < p1Angle && p2Angle > p3Angle )
    {
      return( -( p1Angle - p3Angle ) );
    }
    else
    {
      return( p3Angle + ( 360 - p1Angle ) );
    }
  }
}
