/***************************************************************************
                        qgsgeometryeditor.h
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

#ifndef QGSGEOMETRYEDITOR_H
#define QGSGEOMETRYEDITOR_H

class QgsAbstractGeometryV2;
class QgsGeometryEngine;

#include "qgspointv2.h"

/**Modifies geometry*/
class QgsGeometryEditor
{
    public:
        QgsGeometryEditor( QgsAbstractGeometryV2* geom );
        ~QgsGeometryEditor();

        QgsPointV2 closestVertex( const QgsPointV2& pt, QgsVertexId& id ) const;
        void adjacentVertices( const QgsVertexId& atVertex, QgsVertexId& beforeVertex, QgsVertexId& afterVertex ) const;

        /**Adds interior ring (taking ownership).
     @return 0 in case of success (ring added), 1 problem with geometry type, 2 ring not closed,
     3 ring is not valid geometry, 4 ring not disjoint with existing rings, 5 no polygon found which contained the ring*/
        int addRing( QgsCurveV2* ring );

    private:

        QgsAbstractGeometryV2* mGeometry;

        //default constructor forbidden
        QgsGeometryEditor();
        //Caller takes ownership
        static QgsGeometryEngine* createGeometryEngine( const QgsAbstractGeometryV2* geometry );
};

#endif // QGSGEOMETRYEDITOR_H
