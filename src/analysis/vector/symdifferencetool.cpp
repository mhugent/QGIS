/***************************************************************************
 *  symdifferencetool.cpp                                              *
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
#include "symdifferencetool.h"
#include "geos_c.h"
#include "qgsgeometry.h"

namespace Geoprocessing
{

  SymDifferenceTool::SymDifferenceTool(
    QgsVectorLayer *layerA,
    QgsVectorLayer *layerB,
    bool selectedA, bool selectedB,
    const QString &output,
    const QString& outputDriverName,
    OutputFields outputFields, OutputCrs outputCrs,
    double precision )
      : AbstractTool( precision )
  {
    mLayerA = layerA;
    mLayerB = layerB;
    mSelectedA = selectedA;
    mSelectedB = selectedB;
    createOutputFileWriter( output, layerA, layerB, outputFields, outputCrs, outputDriverName );
    mOutputFileds = outputFields;
  }

  bool SymDifferenceTool::validateInputs( QgsVectorLayer *layerA, QgsVectorLayer *layerB, QString& errorMsgs )
  {
    if ( layerA->geometryType() != QGis::Polygon || layerB->geometryType() != QGis::Polygon )
    {
      errorMsgs = QApplication::translate( "SymDifferenceTool", "Input and operator layers must both be polygon layers." );
      return false;
    }
    return true;
  }

  void SymDifferenceTool::prepare()
  {
    appendToJobQueue( mLayerA, mSelectedA, ProcessLayerAFeature );
    appendToJobQueue( mLayerB, mSelectedB, ProcessLayerBFeature );
    buildSpatialIndex( mSpatialIndexA, mLayerA, mSelectedA );
    buildSpatialIndex( mSpatialIndexB, mLayerB, mSelectedB );
  }

  void SymDifferenceTool::processFeature( const Job *job )
  {
    QgsVectorLayer* layerCurr = 0, * layerInter = 0;
    QgsSpatialIndex* spatialIndex = 0;

    if ( job->taskFlag == ProcessLayerAFeature )
    {
      layerCurr = mLayerA;
      layerInter = mLayerB;
      spatialIndex = &mSpatialIndexB;
    }
    else // if(job->taskFlag == ProcessLayerBFeature)
    {
      layerCurr = mLayerB;
      layerInter = mLayerA;;
      spatialIndex = &mSpatialIndexA;
    }

    // Get currently processed feature
    QgsFeature f;
    if ( !getFeatureAtId( f, job->featureid, layerCurr ) ) return;
    QgsGeometry* geom = f.geometry();

    // Get features which intersect current feature
    QVector<QgsFeature*> featureList = getIntersects( geom->boundingBox(), *spatialIndex, layerInter );

    // Cut off all parts of current feature which intersect with features from intersecting layer
    const GEOSPreparedGeometry* featureGeomPrepared = GEOSPrepare_r( QgsGeometry::getGEOSHandler(), geom->asGeos() );
    QString errorMsg;
    QgsGeometry* newGeom = new QgsGeometry( *geom );
    foreach ( QgsFeature* testFeature, featureList )
    {
      QgsGeometry* testGeom = testFeature->geometry();
      if ( GEOSPreparedIntersects_r( QgsGeometry::getGEOSHandler(), featureGeomPrepared, testGeom->asGeos() ) )
      {
        QgsGeometry* newGeomTmp = newGeom->difference( testGeom, &errorMsg );
        if ( !newGeomTmp )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( layerCurr, f.id() ) << ErrorFeature( layerInter, testFeature->id() ), errorMsg );
          continue;
        }
        else
        {
          delete newGeom;
          newGeom = newGeomTmp;
        }
      }
    }
    GEOSPreparedGeom_destroy_r( QgsGeometry::getGEOSHandler(), featureGeomPrepared );

    if ( !newGeom->isGeosEmpty() )
    {
      QgsFeature outFeature;
      outFeature.setGeometry( newGeom );
      if ( job->taskFlag == ProcessLayerAFeature )
      {
        outFeature.setAttributes( combineAttributes( &f.attributes(), 0, mOutputFileds ) );
      }
      else
      {
        outFeature.setAttributes( combineAttributes( 0, &f.attributes(), mOutputFileds ) );
      }
      writeFeatures( QVector<QgsFeature*>() << &outFeature );
    }
    else
    {
      delete newGeom;
    }

    qDeleteAll( featureList );
  }

} // Geoprocessing
