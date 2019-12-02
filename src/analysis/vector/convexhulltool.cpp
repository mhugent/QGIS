/***************************************************************************
 *  convexhulltool.cpp                                                     *
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

#include "convexhulltool.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollection.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"

namespace Vectoranalysis
{

  ConvexHullTool::ConvexHullTool( QgsVectorLayer *layer,
                                  bool selected,
                                  const QString &output,
                                  const QString &outputDriverName,
                                  Utils::GroupMode groupMode,
                                  int groupField,
                                  QgsExpression *groupExpression,
                                  Utils::SummarizeMode numericSummarizeMode,
                                  Utils::SummarizeMode nonNumericSummarizeMode,
                                  double precision )
    : AbstractTool( precision )
  {
    mOutWkbType = QgsWkbTypes::Polygon;
    mOutputWriter = new QgsVectorFileWriter( output, layer->dataProvider()->encoding(), layer->fields(), mOutWkbType, layer->crs(), outputDriverName );
    mLayer = layer;
    mSelected = selected;
    mGroupMode = groupMode;
    mGroupField = groupField;
    mGroupExpression = groupExpression;
    mNumericSummarizer = Utils::getSummarizer( numericSummarizeMode );
    mNonNumericSummarizer = Utils::getSummarizer( nonNumericSummarizeMode );
  }

  ConvexHullTool::~ConvexHullTool()
  {
    delete mGroupExpression;
  }

  void ConvexHullTool::prepare()
  {
    QMap< QString, QgsFeatureIds > clusters = Utils::groupFeatures( mLayer, mSelected, mGroupMode, mGroupField, mGroupExpression );
    for ( const QgsFeatureIds &cluster : clusters )
    {
      mJobQueue.append( new ConvexHullJob( cluster ) );
    }
  }

  void ConvexHullTool::processFeature( const Job *job )
  {
    const QgsFeatureIds &cluster = static_cast<const ConvexHullJob *>( job )->cluster;

    // Compute convex hull of features in current cluster
    QVector<QgsFeature *> features;
    QVector<QgsAttributes> attributes;

    QgsGeometryCollection *geomCollection = new QgsGeometryCollection();
    for ( const QgsFeatureId &id : cluster )
    {
      QgsFeature *feature = new QgsFeature();
      if ( !getFeatureAtId( *feature, id, mLayer ) )
      {
        delete feature;
        continue;
      }
      features.append( feature );
      geomCollection->addGeometry( feature->geometry().get() );
      attributes.append( feature->attributes() );
    }

    QgsGeometry collection( geomCollection );
    QgsGeometry outGeom = collection.convexHull();

    if ( outGeom.isEmpty() )
    {
      // TODO report error
      qDeleteAll( features );
      return;
    }

    QgsFeature outFeature;
    outFeature.setGeometry( outGeom );

    QgsAttributes outMap = Utils::summarizeAttributes( mLayer->fields(), attributes, mNumericSummarizer, mNonNumericSummarizer, mLayer->dataProvider()->pkAttributeIndexes() << mGroupField );
    //summarizeAttributes( const QgsFields& fields, const QVector<const QgsAttributes *> &maps, summarizer_t numericSummarizer, summarizer_t nonNumericSummarizer , const QgsAttributeList &exclude
    outFeature.setAttributes( outMap );

    // Write features
    writeFeatures( QVector<QgsFeature *>() << &outFeature );
    qDeleteAll( features );
  }

} // Geoprocessing
