/***************************************************************************
                        qgsgeometryeditor.cpp
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

#include "qgsgeometryeditor.h"
#include "qgscurvev2.h"
#include "qgscurvepolygonv2.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgsmultisurfacev2.h"
#include <limits>

QgsGeometryEditor::QgsGeometryEditor( QgsAbstractGeometryV2* geom ): mGeometry( geom )
{

}

QgsGeometryEditor::QgsGeometryEditor(): mGeometry( 0 )
{

}

QgsGeometryEditor::~QgsGeometryEditor()
{

}

QgsPointV2 QgsGeometryEditor::closestVertex( const QgsPointV2& pt, QgsVertexId& id ) const
{
    if( !mGeometry )
    {
        return QgsPointV2();
    }

    double minDist = std::numeric_limits<double>::max();
    double currentDist = 0;
    QgsPointV2 minDistPoint;

    QList< QList< QList< QgsPointV2 > > > coords;
    mGeometry->coordinateSequence( coords );
    for( int feature = 0; feature < coords.size(); ++feature )
    {
        const QList< QList< QgsPointV2 > >& featureCoords = coords.at( feature );
        for( int ring = 0; ring < featureCoords.size(); ++ring )
        {
            const QList< QgsPointV2 >& ringCoords = featureCoords.at( ring );
            for( int vertex = 0; vertex < ringCoords.size(); ++vertex )
            {
                currentDist = QgsGeometryUtils::sqrDistance2D( pt, ringCoords.at( vertex ) );
                if( currentDist < minDist )
                {
                    minDist = currentDist;
                    minDistPoint = ringCoords.at( vertex );
                    id.feature = feature;
                    id.ring = ring;
                    id.vertex = vertex;
                }
            }
        }
    }

    return minDistPoint;
}

void QgsGeometryEditor::adjacentVertices( const QgsVertexId& atVertex, QgsVertexId& beforeVertex, QgsVertexId& afterVertex ) const
{
    if( !mGeometry )
    {
        return;
    }

    bool polygonType = ( mGeometry->dimension()  == 2 );
    QList< QList< QList< QgsPointV2 > > > coords;
    mGeometry->coordinateSequence( coords );

    //get feature
    if( coords.size() <= atVertex.feature )
    {
        return; //error, no such feature
    }
    const QList< QList< QgsPointV2 > >& feature = coords.at( atVertex.feature );

    //get ring
    if( feature.size() <= atVertex.ring )
    {
        return; //error, no such ring
    }
    const QList< QgsPointV2 >& ring = feature.at( atVertex.ring );
    if( ring.size() <= atVertex.vertex )
    {
        return;
    }

    //vertex in the middle
    if( atVertex.vertex > 0 && atVertex.vertex < ring.size() - 1 )
    {
        beforeVertex.feature = atVertex.feature; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = atVertex.vertex - 1;
        afterVertex.feature = atVertex.feature; afterVertex.ring = atVertex.ring; afterVertex.vertex = atVertex.vertex + 1;
    }
    else if( atVertex.vertex == 0 )
    {
        afterVertex.feature = atVertex.feature; afterVertex.ring = atVertex.ring; afterVertex.vertex = atVertex.vertex + 1;
        if( polygonType && ring.size() > 3 )
        {
            beforeVertex.feature = atVertex.feature; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = ring.size() - 2;
        }
        else
        {
            beforeVertex = QgsVertexId(); //before vertex invalid
        }
    }
    else if( atVertex.vertex == ring.size() - 1 )
    {
       beforeVertex.feature = atVertex.feature; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = atVertex.vertex - 1;
       if( polygonType )
       {
           afterVertex.feature = atVertex.feature; afterVertex.ring = atVertex.ring; afterVertex.vertex = 0;
       }
       else
       {
           afterVertex = QgsVertexId(); //after vertex invalid
       }
    }
}

int QgsGeometryEditor::addRing( QgsCurveV2* ring )
{
    if( !ring )
    {
        return 1;
    }

    QList< QgsCurvePolygonV2* > polygonList;
    QgsCurvePolygonV2* curvePoly = dynamic_cast< QgsCurvePolygonV2* >( mGeometry );
    QgsMultiSurfaceV2* multiSurface = dynamic_cast< QgsMultiSurfaceV2* >( mGeometry );
    if( curvePoly )
    {
        polygonList.append( curvePoly );
    }
    else if( multiSurface )
    {
        for( int i = 0; i < multiSurface->numGeometries(); ++i )
        {
            polygonList.append( dynamic_cast< QgsCurvePolygonV2* >( multiSurface->geometryN( i ) ) );
        }
    }
    else
    {

        return 1; //not polygon / multipolygon;
    }

    //ring must be closed
    if( !ring->isClosed() )
    {
        return 2;
    }
    else if( !ring->isRing() )
    {
        return 3;
    }

    QScopedPointer<QgsGeometryEngine> ringGeom( createGeometryEngine( ring ) );
    ringGeom->prepareGeometry();

    //for each polygon, test if inside outer ring and no intersection with other interior ring
    QList< QgsCurvePolygonV2* >::iterator polyIter = polygonList.begin();
    for(; polyIter != polygonList.end(); ++polyIter )
    {
        if( ringGeom->within( **polyIter ) )
        {
            //check if disjoint with other interior rings
            int nInnerRings = ( *polyIter )->numInteriorRings();
            for( int i = 0; i < nInnerRings; ++i )
            {
                if( !ringGeom->disjoint( *(*polyIter)->interiorRing( i ) ) )
                {
                    return 4;
                }
            }
            (*polyIter)->addInteriorRing( ring );
            return 0; //success
        }
    }
    return 5; //not contained in any outer ring
}

QgsGeometryEngine* QgsGeometryEditor::createGeometryEngine( const QgsAbstractGeometryV2* geometry )
{
    return new QgsGeos( geometry );
}


