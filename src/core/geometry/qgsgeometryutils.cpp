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

    if( qgsDoubleNear( length, 0.0 ) )
    {
        return startPoint;
    }

    double scaleFactor = distance / length;
    return QgsPointV2( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}
