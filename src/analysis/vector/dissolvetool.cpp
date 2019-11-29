/***************************************************************************
 *  dissolvetool.cpp                                                   *
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

#include "dissolvetool.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

namespace Geoprocessing
{

  DissolveTool::DissolveTool( QgsVectorLayer *layer,
                              bool selected,
                              const QString &output,
                              const QString& outputDriverName,
                              Utils::GroupMode groupMode,
                              int groupField,
                              QgsExpression *groupExpression,
                              Utils::SummarizeMode numericSummarizeMode,
                              Utils::SummarizeMode nonNumericSummarizeMode,
                              bool allowMultipart,
                              double precision,
                              double bufferDistance,
                              int bufferDistanceField,
                              double mapToLayer,
                              BufferTool::BufferParams* bufferParams )
      : AbstractTool( precision )
  {
    mLayer = layer;
    mSelected = selected;
    mGroupMode = groupMode;
    mGroupField = groupField;
    mGroupExpression = groupExpression;
    mAllowMultipart = allowMultipart;
    mOutWkbType = QgsWkbTypes::Polygon;
    mBufferDistance = bufferDistance;
    mBufferDistanceField = bufferDistanceField;
    mMapToLayer = mapToLayer;
    mBufferParams = bufferParams;
    if ( mAllowMultipart )
    {
      mOutWkbType = QgsWkbTypes::multiType( mOutWkbType );
    }
    mOutputWriter = new QgsVectorFileWriter( output, layer->dataProvider()->encoding(), layer->fields(), mOutWkbType, layer->crs(), outputDriverName );

    mNumericSummarizer = Utils::getSummarizer( numericSummarizeMode );
    mNonNumericSummarizer = Utils::getSummarizer( nonNumericSummarizeMode );
  }

  DissolveTool::~DissolveTool()
  {
    delete mBufferParams;
    delete mGroupExpression;
  }

  void DissolveTool::prepare()
  {
    QMap< QString, QgsFeatureIds > clusters = Utils::groupFeatures( mLayer, mSelected, mGroupMode, mGroupField, mGroupExpression );
    for( const QgsFeatureIds& cluster : clusters )
    {
      mJobQueue.append( new DissolveJob( cluster ) );
    }
  }

  void DissolveTool::processFeature( const Job *job )
  {
    const QgsFeatureIds& cluster = static_cast<const DissolveJob*>( job )->cluster;

    // Merge features in current cluster
    QVector<QgsFeature*> features;
    QVector<QgsAttributes> attributes;
    QVector<QgsFeature*> outFeatures;
    QString errorMsg;
    QVector<QgsGeometry> combineGeometries;
    QMap<QgsFeatureId, QPair< QgsGeometry, QgsAttributes > > geomAttribMap;

    for ( const QgsFeatureId& id : cluster )
    {
      QgsFeature* feature = new QgsFeature();
      if ( !getFeatureAtId( *feature, id, mLayer ) )
      {
        delete feature;
        continue;
      }
      features.append( feature );

      // Buffer if requested
      if ( mBufferParams != 0 )
      {
        double distance = mBufferDistance;
        if ( mBufferDistanceField >= 0 )
        {
          distance = feature->attribute( mBufferDistanceField ).toDouble();
        }
        distance *= mMapToLayer;
        QString errorMsg;
        QgsGeometry bufferGeom;
        if( mBufferParams->singleSided )
        {
            bufferGeom = feature->geometry().singleSidedBuffer( distance, mBufferParams->segments, mBufferParams->bufferSide, mBufferParams->joinStyle, mBufferParams->mitreLimit );
        }
        else
        {
            bufferGeom = feature->geometry().buffer( distance, mBufferParams->segments, mBufferParams->endCapStyle, mBufferParams->joinStyle, mBufferParams->mitreLimit );
        }

        if( bufferGeom.isEmpty() )
        {
            reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayer, feature->id() ), bufferGeom.lastError() );
            continue;
        }
        feature->setGeometry( bufferGeom );
      }
      combineGeometries.append( feature->geometry() );
      geomAttribMap.insert( feature->id(), qMakePair( feature->geometry(), feature->attributes() ) );
      attributes.append( feature->attributes() );
    }

    // Combine geometries
    QgsGeometry geom = QgsGeometry::unaryUnion( combineGeometries );
    if ( geom.isEmpty() )
    {
      reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayer, features.first()->id() ) << ErrorFeature( mLayer, features.first()->id() ), geom.lastError() );
      qDeleteAll( features );
      return;
    }
    if ( !mAllowMultipart )
    {
      for( const QgsGeometry& part : geom.asGeometryCollection() )
      {
        QgsFeature* outFeature = new QgsFeature();
        outFeature->setGeometry( part );
        outFeatures.append( outFeature );
        QgsRectangle rect = part.boundingBox();

        // Collect attributes to summarize
        QVector<QgsAttributes> partAttributes;
        QMap<QgsFeatureId, QPair< QgsGeometry, QgsAttributes > >::const_iterator it = geomAttribMap.constBegin();
        QList< QgsFeatureId > removeList;
        for(; it != geomAttribMap.constEnd(); ++it )
        {
            if ( rect.contains( it.value().first.boundingBox() ) )
            {
              if ( part.contains( it.value().first ) )
              {
                partAttributes.append( it.value().second );
                removeList.append( it.key() );
              }
            }
        }

        for( const QgsFeatureId& id : removeList )
        {
            geomAttribMap.remove( id );
        }

        // Summarize attributes
        QgsAttributes outAttribs;
        if( partAttributes.size() > 0 )
        {
            outAttribs = Utils::summarizeAttributes( mLayer->fields(), partAttributes, mNumericSummarizer, mNonNumericSummarizer, mLayer->dataProvider()->pkAttributeIndexes() << mGroupField );
        }
        outFeature->setAttributes( outAttribs );
      }
    }
    else
    {
      QgsFeature* outFeature = new QgsFeature();
      outFeature->setGeometry( geom );
      outFeatures.append( outFeature );
      // Summarize attributes
      QgsAttributes outAttribs = Utils::summarizeAttributes( mLayer->fields(), attributes, mNumericSummarizer, mNonNumericSummarizer, mLayer->dataProvider()->pkAttributeIndexes() << mGroupField );
      outFeature->setAttributes( outAttribs );
    }

    // Write features
    writeFeatures( outFeatures );
    qDeleteAll( features );
    qDeleteAll( outFeatures );
  }

} // Geoprocessing
