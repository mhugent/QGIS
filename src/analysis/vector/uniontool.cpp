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
#include "qgsgeos.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  UnionTool::UnionTool( QgsVectorLayer *layerA,
                        QgsVectorLayer *layerB,
                        bool selectedA, bool selectedB,
                        const QString &output,
                        const QString &outputDriverName,
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

  bool UnionTool::validateInputs( QgsVectorLayer *layerA, QgsVectorLayer *layerB, QString &errorMsgs )
  {
    if ( layerA->geometryType() != QgsWkbTypes::PolygonGeometry || layerB->geometryType() != QgsWkbTypes::PolygonGeometry )
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
    QgsVectorLayer *layerCurr = 0, * layerInter = 0;
    QgsSpatialIndex *spatialIndex = 0;

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
    QgsGeometry geom = f.geometry();

    // Get features which intersect current feature
    QVector<QgsFeature *> featureList = getIntersects( geom.boundingBox(), *spatialIndex, layerInter );

    // Cut of all parts of current feature which intersect with features from intersecting layer
    // If processmode = ProcessLayerAFeature, add intersecting features as separate features
    QgsGeos geos( geom.constGet() );
    geos.prepareGeometry();

    QVector<QgsFeature *> outputFeatures; // Use pointers to prevent copies
    QgsGeometry newGeom( geom );
    QString errorMsg;
    for ( QgsFeature *testFeature : featureList )
    {
      QgsGeometry testGeom = testFeature->geometry();
      if ( geos.intersects( testGeom.constGet() ) )
      {
        QgsGeometry newGeomTmp = newGeom.difference( testGeom );
        if ( newGeomTmp.isNull() )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( layerCurr, f.id() ) << ErrorFeature( layerInter, testFeature->id() ), newGeomTmp.lastError() );
        }
        else
        {
          newGeom = newGeomTmp;
        }
        if ( job->taskFlag == ProcessLayerAFeature )
        {
          QgsGeometry outGeometry = geom.intersection( testGeom );
          if ( outGeometry.isNull() )
          {
            reportGeometryError( QList<ErrorFeature>() << ErrorFeature( layerCurr, f.id() ) << ErrorFeature( layerInter, testFeature->id() ), outGeometry.lastError() );
          }
          else if ( outGeometry.isEmpty() )
          {
            reportGeometryError( QList<ErrorFeature>() << ErrorFeature( layerCurr, f.id() ) << ErrorFeature( layerInter, testFeature->id() ), QApplication::translate( "UnionTool", "GEOSIntersection returned empty geometry even though the geometries intersect" ) );
          }
          else
          {
            QgsFeature *outFeature = new QgsFeature();
            outFeature->setGeometry( outGeometry );
            QgsAttributes fAtt = f.attributes();
            QgsAttributes testAtt = testFeature->attributes();
            outFeature->setAttributes( combineAttributes( &fAtt, &testAtt, mOutputFileds ) );
            outputFeatures.append( outFeature );
          }
        }
      }
    }
    if ( !newGeom.isEmpty() )
    {
      QgsFeature *outFeature = new QgsFeature();
      outFeature->setGeometry( newGeom );
      QgsAttributes fAtt = f.attributes();
      if ( job->taskFlag == ProcessLayerAFeature )
      {
        outFeature->setAttributes( combineAttributes( &fAtt, 0, mOutputFileds ) );
      }
      else
      {
        outFeature->setAttributes( combineAttributes( 0, &fAtt, mOutputFileds ) );
      }
      outputFeatures.append( outFeature );
    }

    writeFeatures( outputFeatures );
    qDeleteAll( outputFeatures );
    qDeleteAll( featureList );
  }

} // Geoprocessing
