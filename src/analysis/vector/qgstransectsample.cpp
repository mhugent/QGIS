#include "qgstransectsample.h"
#include "qgsgeometry.h"
#include "qgsspatialindex.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

QgsTransectSample::QgsTransectSample( QgsVectorLayer* strataLayer, int strataIdAttribute, int minDistanceAttribute,
                                      int nPointsAttribute, QgsVectorLayer* baselineLayer, bool shareBaseline,
                                      int baselineStrataId, const QString& outputPointLayer,
                                      const QString& outputLineLayer ): mStrataLayer( strataLayer ),
    mStrataIdAttribute( strataIdAttribute ), mMinDistanceAttribute( minDistanceAttribute ),
    mNPointsAttribute( nPointsAttribute ), mBaselineLayer( baselineLayer ), mShareBaseline( shareBaseline ),
    mBaselineStrataId( baselineStrataId ), mOutputPointLayer( outputPointLayer ), mOutputLineLayer( outputLineLayer )
{
}

QgsTransectSample::QgsTransectSample()
{
}

QgsTransectSample::~QgsTransectSample()
{
}

int QgsTransectSample::createSample( QProgressDialog* pd )
{
  Q_UNUSED( pd );

  if ( !mStrataLayer || !mStrataLayer->isValid() )
  {
    return 1;
  }

  if ( !mBaselineLayer || !mBaselineLayer->isValid() )
  {
    return 2;
  }

  //create vector file writers for output
  QgsFieldMap outputPointFields;
  outputPointFields.insert( 0, QgsField( "id", QVariant::Int ) );
  outputPointFields.insert( 1, QgsField( "station_id", QVariant::Int ) );
  outputPointFields.insert( 2, QgsField( "stratum_id", QVariant::Int ) );
  QgsVectorFileWriter outputPointWriter( mOutputPointLayer, "utf-8", outputPointFields, QGis::WKBPoint,
                                         &( mStrataLayer->crs() ) );
  if ( outputPointWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 3;
  }

  QgsVectorFileWriter outputLineWriter( mOutputLineLayer, "utf-8", outputPointFields, QGis::WKBLineString,
                                        &( mStrataLayer->crs() ) );
  if ( outputLineWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 4;
  }

  //init random number generator
  srand( QTime::currentTime().msec() );

  //iterate over strata layer
  QgsAttributeList attList;
  attList << mStrataIdAttribute;
  attList << mMinDistanceAttribute;
  attList << mNPointsAttribute;
  mStrataLayer->select( attList );
  QgsFeature fet;
  int nTotalTransects = 0;

  while ( mStrataLayer->nextFeature( fet ) )
  {
    QgsGeometry* strataGeom = fet.geometry();
    if ( !strataGeom )
    {
      continue;
    }

    //find baseline for strata
    bool strataIdOk = true;
    int strataId = fet.attributeMap()[mStrataIdAttribute].toInt( &strataIdOk );
    QgsGeometry* baselineGeom = findBaselineGeometry( strataIdOk ? strataId : -1 );
    if ( !baselineGeom )
    {
      continue;
    }

    double minDistance = fet.attributeMap()[mMinDistanceAttribute].toDouble();

    //clip baseline by strata
    QgsGeometry* clippedBaseline = strataGeom->intersection( baselineGeom );
    if ( !clippedBaseline )
    {
      delete baselineGeom;
      continue;
    }

    //create line buffer and clip by strata
    QgsGeometry* clipBaselineBuffer = clippedBaseline->buffer( minDistance, 8 );
    if ( !clipBaselineBuffer && !( clipBaselineBuffer->wkbType() == QGis::WKBPolygon ||
                                   clipBaselineBuffer->wkbType() == QGis::WKBPolygon25D ) )
    {
      delete baselineGeom; delete clippedBaseline; delete clipBaselineBuffer;
      continue;
    }

    QgsPolygon bufferPolygon = clipBaselineBuffer->asPolygon();
    if ( bufferPolygon.size() < 1 )
    {
      delete baselineGeom; delete clippedBaseline; delete clipBaselineBuffer;
      continue;
    }
    QgsGeometry* bufferLine = QgsGeometry::fromPolyline( bufferPolygon[0] );
    QgsGeometry* bufferLineClipped = bufferLine->intersection( strataGeom );
    if ( !bufferLineClipped )
    {
      delete baselineGeom; delete clippedBaseline; delete clipBaselineBuffer; delete bufferLine;
      continue;
    }

    //start loop to create random points along the baseline
    int nTransects = fet.attributeMap()[mNPointsAttribute].toInt();
    int nCreatedTransects = 0;
    int nIterations = 0;
    int nMaxIterations = nTransects * 50;

    QgsSpatialIndex sIndex; //to check minimum distance
    QMap< QgsFeatureId, QgsGeometry* > lineFeatureMap;

    while ( nCreatedTransects < nTransects && nIterations < nMaxIterations )
    {
      double randomPosition = (( double )rand() / RAND_MAX ) * clippedBaseline->length();
      QgsGeometry* samplePoint = clippedBaseline->interpolate( randomPosition );
      ++nIterations;
      if ( !samplePoint )
      {
        continue;
      }

      QgsFeature samplePointFeature;
      samplePointFeature.setGeometry( samplePoint );
      samplePointFeature.addAttribute( 0, nTotalTransects );
      samplePointFeature.addAttribute( 1, nCreatedTransects );
      samplePointFeature.addAttribute( 2, fet.id() );

      //find closest point on clipped buffer line
      QgsPoint minDistPoint;
      QgsPoint sampleQgsPoint = samplePoint->asPoint();
      int afterVertex;
      if ( bufferLineClipped->closestSegmentWithContext( sampleQgsPoint, minDistPoint, afterVertex ) < 0 )
      {
        continue;
      }

      QgsPolyline sampleLinePolyline;
      QgsPoint ptFarAway( sampleQgsPoint.x() + ( minDistPoint.x() - sampleQgsPoint.x() ) * 1000000,
                          sampleQgsPoint.y() + ( minDistPoint.y() - sampleQgsPoint.y() ) * 1000000 );
      QgsPolyline lineFarAway;
      lineFarAway << sampleQgsPoint << ptFarAway;
      QgsGeometry* lineFarAwayGeom = QgsGeometry::fromPolyline( lineFarAway );
      QgsGeometry* lineClipStratum = lineFarAwayGeom->intersection( strataGeom );
      if ( !lineClipStratum )
      {
        delete lineFarAwayGeom; delete lineClipStratum;
        continue;
      }

      //search closest existing profile. Cancel if dist < minDist
      if ( otherTransectWithinDistance( lineClipStratum, minDistance, sIndex, lineFeatureMap ) )
      {
        delete lineFarAwayGeom; delete lineClipStratum;
        continue;
      }

      QgsFeatureId fid( nCreatedTransects );
      QgsFeature sampleLineFeature( fid );
      sampleLineFeature.setGeometry( lineClipStratum );
      sampleLineFeature.addAttribute( 0, nTotalTransects );
      sampleLineFeature.addAttribute( 1, nCreatedTransects );
      sampleLineFeature.addAttribute( 2, fet.id() );
      outputLineWriter.addFeature( sampleLineFeature );

      //add point to file writer here.
      //It can only be written if the corresponding transect has been as well
      outputPointWriter.addFeature( samplePointFeature );

      sIndex.insertFeature( sampleLineFeature );
      lineFeatureMap.insert( fid, sampleLineFeature.geometryAndOwnership() );

      delete lineFarAwayGeom;
      ++nTotalTransects;
      ++nCreatedTransects;
    }
    delete baselineGeom; delete clippedBaseline; delete clipBaselineBuffer; delete bufferLine;
    delete bufferLineClipped;

    //delete all line geometries in spatial index
    QMap< QgsFeatureId, QgsGeometry* >::iterator featureMapIt = lineFeatureMap.begin();
    for ( ; featureMapIt != lineFeatureMap.end(); ++featureMapIt )
    {
      delete( featureMapIt.value() );
    }
    lineFeatureMap.clear();
  }

  return 0;
}

QgsGeometry* QgsTransectSample::findBaselineGeometry( int strataId )
{
  QgsAttributeList attList;
  attList << mBaselineStrataId;
  mBaselineLayer->select( attList );
  QgsFeature fet;
  while ( mBaselineLayer->nextFeature( fet ) ) //todo: cache this in case there are many baslines
  {
    if ( strataId == fet.attributeMap()[mBaselineStrataId].toInt() || mShareBaseline )
    {
      return fet.geometryAndOwnership();
    }
  }
  return 0;
}

bool QgsTransectSample::otherTransectWithinDistance( QgsGeometry* geom, double minDistance, QgsSpatialIndex& sIndex,
    const QMap< QgsFeatureId, QgsGeometry* >& lineFeatureMap )
{
  if ( !geom )
  {
    return false;
  }

  QgsGeometry* buffer = geom->buffer( minDistance, 8 );
  if ( !buffer )
  {
    return false;
  }
  QgsRectangle rect = buffer->boundingBox();
  QList<QgsFeatureId> lineIdList = sIndex.intersects( rect );

  QList<QgsFeatureId>::const_iterator lineIdIt = lineIdList.constBegin();
  for ( ; lineIdIt != lineIdList.constEnd(); ++lineIdIt )
  {
    const QMap< QgsFeatureId, QgsGeometry* >::const_iterator idMapIt = lineFeatureMap.find( *lineIdIt );
    if ( idMapIt != lineFeatureMap.constEnd() )
    {
      if ( geom->distance( *( idMapIt.value() ) ) < minDistance )
      {
        delete buffer;
        return true;
      }
    }
  }

  delete buffer;
  return false;
}
