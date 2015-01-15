/***************************************************************************
                        qgsgeometryutils.h
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

#ifndef QGSGEOMETRYUTILS_H
#define QGSGEOMETRYUTILS_H

#include "qgspointv2.h"
#include <limits>

class QgsGeometryUtils
{
  public:
    static double sqrDistance2D( const QgsPointV2& pt1, const QgsPointV2& pt2 );

    static double sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double& minDistX, double& minDistY, double epsilon );

    /**Returns < 0 if point(x/y) is left of the line x1,y1 -> x1,y2*/
    static double leftOfLine( double x, double y, double x1, double y1, double x2, double y2 );

    static QgsPointV2 pointOnLineWithDistance( const QgsPointV2& startPoint, const QgsPointV2& directionPoint, double distance );

    enum componentType
    {
      VERTEX,
      RING,
      PART
    };

    template<class T> static double closestSegmentFromComponents( T& container, componentType ctype, const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon )
    {
      double minDist = std::numeric_limits<double>::max();
      double minDistSegmentX, minDistSegmentY;
      QgsVertexId minDistVertexAfter;
      bool minDistLeftOf;
      double sqrDist = 0.0;
      int vertexOffset = 0;
      int ringOffset = 0;
      int partOffset = 0;

      for ( int i = 0; i < container.size(); ++i )
      {
        sqrDist = container.at( i )->closestSegment( pt, segmentPt, vertexAfter, leftOf, epsilon );
        if ( sqrDist < minDist )
        {
          minDist = sqrDist;
          minDistSegmentX = segmentPt.x();
          minDistSegmentY = segmentPt.y();
          minDistVertexAfter = vertexAfter;
          minDistVertexAfter.vertex = vertexAfter.vertex + vertexOffset;
          minDistVertexAfter.feature = vertexAfter.feature + partOffset;
          minDistVertexAfter.ring = vertexAfter.ring + ringOffset;
          if ( leftOf )
          {
            minDistLeftOf = *leftOf;
          }
        }

        if ( ctype == VERTEX )
        {
          vertexOffset += container.at( i )->nCoordinates();
        }
        else if ( ctype == RING )
        {
          ringOffset += 1;
        }
        else if ( ctype == PART )
        {
          partOffset += 1;
        }
      }

      segmentPt.setX( minDistSegmentX );
      segmentPt.setY( minDistSegmentY );
      vertexAfter = minDistVertexAfter;
      if ( leftOf )
      {
        *leftOf = minDistLeftOf;
      }
      return minDist;
    }
};

#endif // QGSGEOMETRYUTILS_H
