/***************************************************************************
                        qgsgeos.h
  -------------------------------------------------------------------
Date                 : 22 Sept 2014
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

#ifndef QGSGEOS_H
#define QGSGEOS_H

#include "qgsgeometryengine.h"
#include "qgspointv2.h"
#include <geos_c.h>

class QgsLineStringV2;

/**Does vector analysis using the geos library and handles import, export, exception handling*/
class QgsGeos: public QgsGeometryEngine
{
  public:
    QgsGeos( QgsAbstractGeometryV2* geometry );
    ~QgsGeos();

    /**Removes caches*/
    void geometryChanged();
    void prepareGeometry();

    QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const;
    QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const;
    QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const ;
    QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const;
    double distance( const QgsAbstractGeometryV2& geom ) const;
    bool intersects( const QgsAbstractGeometryV2& geom ) const;
    bool touches( const QgsAbstractGeometryV2& geom ) const;
    bool crosses( const QgsAbstractGeometryV2& geom ) const;
    bool within( const QgsAbstractGeometryV2& geom ) const;
    bool overlaps( const QgsAbstractGeometryV2& geom ) const;
    bool contains( const QgsAbstractGeometryV2& geom ) const;

    static QgsAbstractGeometryV2* fromGeos( const GEOSGeometry* geos );
    static GEOSGeometry* asGeos( const QgsAbstractGeometryV2* geom );
    static QgsPointV2 coordSeqPoint( const GEOSCoordSequence* cs, int i, bool hasZ, bool hasM );

  private:
    mutable GEOSGeometry* mGeos;
    const GEOSPreparedGeometry* mGeosPrepared;

    enum Overlay
    {
      INTERSECTION,
      DIFFERENCE,
      UNION,
      SYMDIFFERENCE
    };

    enum Relation
    {
      INTERSECTS,
      TOUCHES,
      CROSSES,
      WITHIN,
      OVERLAPS,
      CONTAINS
    };

    void cacheGeos() const;
    QgsAbstractGeometryV2* overlay( const QgsAbstractGeometryV2& geom, Overlay op ) const;
    bool relation( const QgsAbstractGeometryV2& geom, Relation r ) const;
    static GEOSCoordSequence* createCoordinateSequence( const QgsCurveV2* curve );
    static QgsLineStringV2* sequenceToLinestring( const GEOSGeometry* geos, bool hasZ, bool hasM );
};

#endif // QGSGEOS_H
