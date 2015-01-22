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
#include "qgspolygonv2.h"
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
  if ( !mGeometry )
  {
    return QgsPointV2();
  }

  double minDist = std::numeric_limits<double>::max();
  double currentDist = 0;
  QgsPointV2 minDistPoint;

  QgsVertexId vertexId;
  QgsPointV2 vertex;
  while ( mGeometry->nextVertex( vertexId, vertex ) )
  {
    currentDist = QgsGeometryUtils::sqrDistance2D( pt, vertex );
    if ( currentDist < minDist )
    {
      minDist = currentDist;
      minDistPoint = vertex;
      id.feature = vertexId.feature;
      id.ring = vertexId.ring;
      id.vertex = vertexId.vertex;
    }
  }

  return minDistPoint;
}

void QgsGeometryEditor::adjacentVertices( const QgsVertexId& atVertex, QgsVertexId& beforeVertex, QgsVertexId& afterVertex ) const
{
  if ( !mGeometry )
  {
    return;
  }

  bool polygonType = ( mGeometry->dimension()  == 2 );
  QList< QList< QList< QgsPointV2 > > > coords;
  mGeometry->coordinateSequence( coords );

  //get feature
  if ( coords.size() <= atVertex.feature )
  {
    return; //error, no such feature
  }
  const QList< QList< QgsPointV2 > >& feature = coords.at( atVertex.feature );

  //get ring
  if ( feature.size() <= atVertex.ring )
  {
    return; //error, no such ring
  }
  const QList< QgsPointV2 >& ring = feature.at( atVertex.ring );
  if ( ring.size() <= atVertex.vertex )
  {
    return;
  }

  //vertex in the middle
  if ( atVertex.vertex > 0 && atVertex.vertex < ring.size() - 1 )
  {
    beforeVertex.feature = atVertex.feature; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = atVertex.vertex - 1;
    afterVertex.feature = atVertex.feature; afterVertex.ring = atVertex.ring; afterVertex.vertex = atVertex.vertex + 1;
  }
  else if ( atVertex.vertex == 0 )
  {
    afterVertex.feature = atVertex.feature; afterVertex.ring = atVertex.ring; afterVertex.vertex = atVertex.vertex + 1;
    if ( polygonType && ring.size() > 3 )
    {
      beforeVertex.feature = atVertex.feature; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = ring.size() - 2;
    }
    else
    {
      beforeVertex = QgsVertexId(); //before vertex invalid
    }
  }
  else if ( atVertex.vertex == ring.size() - 1 )
  {
    beforeVertex.feature = atVertex.feature; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = atVertex.vertex - 1;
    if ( polygonType )
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
  if ( !ring )
  {
    return 1;
  }

  QList< QgsCurvePolygonV2* > polygonList;
  QgsCurvePolygonV2* curvePoly = dynamic_cast< QgsCurvePolygonV2* >( mGeometry );
  QgsMultiSurfaceV2* multiSurface = dynamic_cast< QgsMultiSurfaceV2* >( mGeometry );
  if ( curvePoly )
  {
    polygonList.append( curvePoly );
  }
  else if ( multiSurface )
  {
    for ( int i = 0; i < multiSurface->numGeometries(); ++i )
    {
      polygonList.append( dynamic_cast< QgsCurvePolygonV2* >( multiSurface->geometryN( i ) ) );
    }
  }
  else
  {

    return 1; //not polygon / multipolygon;
  }

  //ring must be closed
  if ( !ring->isClosed() )
  {
    return 2;
  }
  else if ( !ring->isRing() )
  {
    return 3;
  }

  QScopedPointer<QgsGeometryEngine> ringGeom( createGeometryEngine( ring ) );
  ringGeom->prepareGeometry();

  //for each polygon, test if inside outer ring and no intersection with other interior ring
  QList< QgsCurvePolygonV2* >::iterator polyIter = polygonList.begin();
  for ( ; polyIter != polygonList.end(); ++polyIter )
  {
    if ( ringGeom->within( **polyIter ) )
    {
      //check if disjoint with other interior rings
      int nInnerRings = ( *polyIter )->numInteriorRings();
      for ( int i = 0; i < nInnerRings; ++i )
      {
        if ( !ringGeom->disjoint( *( *polyIter )->interiorRing( i ) ) )
        {
          return 4;
        }
      }
      ( *polyIter )->addInteriorRing( ring );
      return 0; //success
    }
  }
  return 5; //not contained in any outer ring
}

int QgsGeometryEditor::addPart( QgsAbstractGeometryV2* part )
{
  if ( !mGeometry )
  {
    return 1;
  }

  if ( !part )
  {
    return 2;
  }

  //multitype?
  QgsGeometryCollectionV2* geomCollection = dynamic_cast<QgsGeometryCollectionV2*>( mGeometry );
  if ( !geomCollection )
  {
    return 1;
  }

  bool added = false;
  if ( mGeometry->geometryType() == "MultiSurface" )
  {
    QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( part );
    if ( !curve || !curve->isClosed() )
    {
      delete part; return 2;
    }

    QgsCurvePolygonV2* poly = 0;
    if ( curve->geometryType() == "LineString" )
    {
      poly = new QgsPolygonV2();
    }
    else
    {
      poly = new QgsCurvePolygonV2();
    }
    poly->setExteriorRing( curve );
    added = geomCollection->addGeometry( poly );
  }
  else
  {
    added = geomCollection->addGeometry( part );
  }
  return added ? 0 : 2;
}

QgsGeometryEngine* QgsGeometryEditor::createGeometryEngine( const QgsAbstractGeometryV2* geometry )
{
  return new QgsGeos( geometry );
}


