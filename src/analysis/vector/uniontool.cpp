/***************************************************************************
 *  unionaytool.cpp                                                    *
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
#include "uniontool.h"
#include "geos_c.h"
#include "qgsgeometry.h"

namespace Geoprocessing
{

  UnionTool::UnionTool( QgsVectorLayer *layerA,
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

  bool UnionTool::validateInputs( QgsVectorLayer *layerA, QgsVectorLayer *layerB, QString& errorMsgs )
  {
    if ( layerA->geometryType() != QGis::Polygon || layerB->geometryType() != QGis::Polygon )
    {
      errorMsgs = QApplication::translate( "UnionTool", "Input and operator layers must both be polygon layers." );
      return false;
    }
    return true;
  }

  void UnionTool::prepare()
  {
    appendToJobQueue( mLayerA, mSelectedA, ProcessLayerAFeature );
    appendToJobQueue( mLayerB, mSelectedB, ProcessLayerBFeature );
    buildSpatialIndex( mSpatialIndexA, mLayerA, mSelectedA );
    buildSpatialIndex( mSpatialIndexB, mLayerB, mSelectedB );
  }

  void UnionTool::processFeature( const Job *job )
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
      layerInter = mLayerA;
      spatialIndex = &mSpatialIndexA;
    }

    // Get currently processed feature
    QgsFeature f;
    if ( !getFeatureAtId( f, job->featureid, layerCurr ) ) return;
    QgsGeometry* geom = f.geometry();

    // Get features which intersect current feature
    QVector<QgsFeature*> featureList = getIntersects( geom->boundingBox(), *spatialIndex, layerInter );

    // Cut of all parts of current feature which intersect with features from intersecting layer
    // If processmode = ProcessLayerAFeature, add intersecting features as separate features
    const GEOSPreparedGeometry* featureGeomPrepared = GEOSPrepare_r( QgsGeometry::getGEOSHandler(), geom->asGeos() );
    QVector<QgsFeature*> outputFeatures; // Use pointers to prevent copies
    QgsGeometry* newGeom = new QgsGeometry( *geom );
    QString errorMsg;
    foreach ( QgsFeature* testFeature, featureList )
    {
      QgsGeometry* testGeom = testFeature->geometry();
      if ( GEOSPreparedIntersects_r( QgsGeometry::getGEOSHandler(), featureGeomPrepared, testGeom->asGeos() ) )
      {
        QgsGeometry* newGeomTmp = newGeom->difference( testGeom, &errorMsg );
        if ( !newGeomTmp )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( layerCurr, f.id() ) << ErrorFeature( layerInter, testFeature->id() ), errorMsg );
        }
        else
        {
          delete newGeom;
          newGeom = newGeomTmp;
        }
        if ( job->taskFlag == ProcessLayerAFeature )
        {
          QgsGeometry* outGeometry = geom->intersection( testGeom, &errorMsg );
          if ( !outGeometry )
          {
            reportGeometryError( QList<ErrorFeature>() << ErrorFeature( layerCurr, f.id() ) << ErrorFeature( layerInter, testFeature->id() ), errorMsg );
          }
          else if ( outGeometry->isGeosEmpty() )
          {
            reportGeometryError( QList<ErrorFeature>() << ErrorFeature( layerCurr, f.id() ) << ErrorFeature( layerInter, testFeature->id() ), QApplication::translate( "UnionTool", "GEOSIntersection returned empty geometry even though the geometries intersect" ) );
          }
          else
          {
            QgsFeature* outFeature = new QgsFeature();
            outFeature->setGeometry( outGeometry );
            outFeature->setAttributes( combineAttributes( &f.attributes(), &testFeature->attributes(), mOutputFileds ) );
            outputFeatures.append( outFeature );
          }
        }
      }
    }
    if ( !newGeom->isGeosEmpty() )
    {
      QgsFeature* outFeature = new QgsFeature();
      outFeature->setGeometry( newGeom );
      if ( job->taskFlag == ProcessLayerAFeature )
      {
        outFeature->setAttributes( combineAttributes( &f.attributes(), 0, mOutputFileds ) );
      }
      else
      {
        outFeature->setAttributes( combineAttributes( 0, &f.attributes(), mOutputFileds ) );
      }
      outputFeatures.append( outFeature );
    }

    GEOSPreparedGeom_destroy_r( QgsGeometry::getGEOSHandler(), featureGeomPrepared );

    writeFeatures( outputFeatures );
    qDeleteAll( outputFeatures );
    qDeleteAll( featureList );
  }

} // Geoprocessing
