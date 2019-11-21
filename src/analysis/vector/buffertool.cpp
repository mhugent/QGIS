/***************************************************************************
 *  buffertool.cpp                                                         *
 *  -------------------                                                    *
 *  begin                : Jul 10, 2014                                    *
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

#include "buffertool.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

namespace Geoprocessing
{

  BufferTool::BufferTool( QgsVectorLayer *layer,
                          bool selected,
                          const QString &output,
                          const QString& outputDriverName,
                          double distance,
                          int distanceField,
                          double mapToLayer,
                          BufferParams *params,
                          double precision )
      : AbstractTool( precision )
  {
    mOutWkbType = QgsWkbTypes::Polygon;
    mOutputWriter = new QgsVectorFileWriter( output, layer->dataProvider()->encoding(), layer->fields(), mOutWkbType, layer->crs(), outputDriverName );
    mLayer = layer;
    mSelected = selected;
    mDistance = distance;
    mDistanceField = distanceField;
    mMapToLayer = mapToLayer;
    mParams = params;
  }

  BufferTool::~BufferTool()
  {
    delete mParams;
  }

  void BufferTool::prepare()
  {
    appendToJobQueue( mLayer, mSelected );
  }

  void BufferTool::processFeature( const Job *job )
  {
    // Get currently processed feature
    QgsFeature f;
    if ( !getFeatureAtId( f, job->featureid, mLayer ) ) return;

    QString errorMsg;
    double distance = mDistance;
    if ( mDistanceField >= 0 )
    {
      distance = f.attribute( mDistanceField ).toDouble();
    }
    distance *= mMapToLayer;
    QgsGeometry outGeometry;
    if( mParams->singleSided )
    {
        outGeometry = f.geometry().singleSidedBuffer( distance, mParams->segments, mParams->bufferSide, mParams->joinStyle, mParams->mitreLimit );
    }
    else
    {
        outGeometry = f.geometry().buffer( distance, mParams->segments, mParams->endCapStyle, mParams->joinStyle, mParams->mitreLimit );
    }

    if ( outGeometry.isEmpty() )
    {
      reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayer, f.id() ), outGeometry.lastError() );
      return;
    }

    QgsFeature outputFeature;
    outputFeature.setAttributes( f.attributes() );
    outputFeature.setGeometry( outGeometry );
    writeFeatures( QVector<QgsFeature*>() << &outputFeature );
  }

} // Geoprocessing
