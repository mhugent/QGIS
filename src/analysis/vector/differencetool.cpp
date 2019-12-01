/***************************************************************************
 *  differencetool.cpp                                                 *
 *  -------------------                                                    *
 *  begin                : Jun 10, 2014                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include "differencetool.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsvectorlayer.h"

namespace Geoprocessing
{

  DifferenceTool::DifferenceTool( QgsVectorLayer *layerA,
                                  QgsVectorLayer *layerB,
                                  bool selectedA, bool selectedB,
                                  const QString &output,
                                  const QString &outputDriverName, OutputFields, OutputCrs outputCrs,
                                  double precision )
    : AbstractTool( precision )
  {
    mLayerA = layerA;
    mLayerB = layerB;
    mSelectedA = selectedA;
    mSelectedB = selectedB;
    createOutputFileWriter( output, layerA, layerB, FieldsA, outputCrs, outputDriverName );
  }

  bool DifferenceTool::validateInputs( QgsVectorLayer *layerA, QgsVectorLayer *layerB, QString &errorMsgs )
  {
    if ( layerA->geometryType() != QgsWkbTypes::PolygonGeometry || layerB->geometryType() != QgsWkbTypes::PolygonGeometry )
    {
      errorMsgs = QApplication::translate( "differencetool", "Input and operator layers must both be polygon layers." );
      return false;
    }
    return true;
  }

  void DifferenceTool::prepare()
  {
    appendToJobQueue( mLayerA, mSelectedA );
    buildSpatialIndex( mSpatialIndex, mLayerB, mSelectedB );
  }

  void DifferenceTool::processFeature( const Job *job )
  {

    // Get currently processed feature
    QgsFeature f;
    if ( !getFeatureAtId( f, job->featureid, mLayerA ) ) return;
    QgsGeometry geom = f.geometry();

    // Get features which intersect current feature
    QVector<QgsFeature *> featureList = getIntersects( geom.boundingBox(), mSpatialIndex, mLayerB );

    // Compute difference (or add feature as is if no intersection)
    QgsGeos geos( geom.constGet() );
    geos.prepareGeometry();

    QgsGeometry newGeom( geom );
    for ( QgsFeature *testFeature : featureList )
    {
      QgsGeometry testGeom = testFeature->geometry();
      if ( geos.intersects( testGeom.constGet() ) )
      {
        QgsGeometry newGeomTmp = newGeom.difference( testGeom );
        if ( newGeomTmp.isNull() )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayerA, job->featureid ) << ErrorFeature( mLayerB, testFeature->id() ), newGeomTmp.lastError() );
        }
        else
        {
          newGeom = newGeomTmp;
        }
      }
    }

    if ( !newGeom.isEmpty() )
    {
      QgsFeature outFeature;
      outFeature.setGeometry( newGeom );
      outFeature.setAttributes( f.attributes() );
      writeFeatures( QVector<QgsFeature *>() << &outFeature );
    }

    qDeleteAll( featureList );
  }

} // Geoprocessing
