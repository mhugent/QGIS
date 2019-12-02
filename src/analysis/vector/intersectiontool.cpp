/***************************************************************************
 *  intersectiontool.cpp                                               *
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
#include "intersectiontool.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsvectorlayer.h"

namespace Vectoranalysis
{

  IntersectionTool::IntersectionTool(
    QgsVectorLayer *layerA,
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

  bool IntersectionTool::validateInputs( QgsVectorLayer *layerA, QgsVectorLayer *layerB, QString &errorMsgs )
  {
    if ( layerA->geometryType() > layerB->geometryType() )
    {
      errorMsgs = QApplication::translate( "IntersectionTool", "Dimensionality of the input layer must be less or equal the dimensionality of the operator layer." );
      return false;
    }
    return true;
  }

  void IntersectionTool::prepare()
  {
    appendToJobQueue( mLayerA, mSelectedA );
    buildSpatialIndex( mSpatialIndex, mLayerB, mSelectedB );
  }

  void IntersectionTool::processFeature( const Job *job )
  {
    // Get currently processed feature
    QgsFeature f;
    if ( !getFeatureAtId( f, job->featureid, mLayerA ) ) return;
    QgsGeometry geom = f.geometry();

    // Get features which intersect current feature
    QVector<QgsFeature *> featureList = getIntersects( geom.boundingBox(), mSpatialIndex, mLayerB );

    // Perform intersections
    QgsGeos geos( geom.constGet() );
    geos.prepareGeometry();

    QVector<QgsFeature *> outputFeatures; // Use pointers to prevent copies
    QString errorMsg;

    for ( QgsFeature *testFeature : featureList )
    {
      QgsGeometry testGeom = testFeature->geometry();
      if ( geos.intersects( testGeom.constGet() ) )
      {
        QgsGeometry outGeometry = geom.intersection( testGeom );
        if ( outGeometry.isNull() )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayerA, job->featureid ) << ErrorFeature( mLayerB, testFeature->id() ), outGeometry.lastError() );
        }
        else if ( outGeometry.isEmpty() )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayerA, job->featureid ) << ErrorFeature( mLayerB, testFeature->id() ), QApplication::translate( "IntersectionTool", "GEOSIntersection returned empty geometry even though the geometries intersect" ) );
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
    writeFeatures( outputFeatures );
    qDeleteAll( outputFeatures );
    qDeleteAll( featureList );
  }

} // Geoprocessing
