/***************************************************************************
                        qgsgeometryengine.h
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

#ifndef QGSVECTORTOPOLOGY_H
#define QGSVECTORTOPOLOGY_H

#include "qgspointv2.h"
#include <QList>

class QgsAbstractGeometryV2;
class QgsLineStringV2;

class QgsGeometryEngine
{
  public:
    QgsGeometryEngine( const QgsAbstractGeometryV2* geometry ): mGeometry( geometry ) {}
    virtual ~QgsGeometryEngine() {}

    virtual void geometryChanged() = 0;
    virtual void prepareGeometry() = 0;

    virtual QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual double distance( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool intersects( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool touches( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool crosses( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool within( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool overlaps( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool contains( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool disjoint( const QgsAbstractGeometryV2& geom ) const = 0;

    virtual int splitGeometry( const QgsLineStringV2& splitLine,
                               QList<QgsAbstractGeometryV2*>& newGeometries,
                               bool topological,
                               QList<QgsPointV2> &topologyTestPoints ) const = 0;

  protected:
    const QgsAbstractGeometryV2* mGeometry;

    QgsGeometryEngine();
};

#endif // QGSVECTORTOPOLOGY_H
