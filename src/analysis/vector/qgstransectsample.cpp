#include "qgstransectsample.h"
#include "qgsdistancearea.h"
#include "qgsgeometry.h"
#include "qgsspatialindex.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

QgsTransectSample::QgsTransectSample( QgsVectorLayer* strataLayer, int strataIdAttribute, int minDistanceAttribute, DistanceUnits minDistUnits,
                                      int nPointsAttribute, QgsVectorLayer* baselineLayer, bool shareBaseline,
                                      int baselineStrataId, const QString& outputPointLayer,
                                      const QString& outputLineLayer ): mStrataLayer( strataLayer ),
    mStrataIdAttribute( strataIdAttribute ), mMinDistanceAttribute( minDistanceAttribute ),
    mNPointsAttribute( nPointsAttribute ), mBaselineLayer( baselineLayer ), mShareBaseline( shareBaseline ),
    mBaselineStrataId( baselineStrataId ), mOutputPointLayer( outputPointLayer ), mOutputLineLayer( outputLineLayer ), mMinDistanceUnits( minDistUnits )
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

  //debug code. Cleanup later
  QgsPolyline debugPoly1; debugPoly1 << QgsPoint( 1.0, 1.0 ) << QgsPoint( 2.0, 2.0 );
  QgsGeometry* debugGeom1 = QgsGeometry::fromPolyline( debugPoly1 );
  QgsPolyline debugPoly2; debugPoly2 << QgsPoint( 4.0, 4.0 ) << QgsPoint( 3.0, 3.0 );
  QgsGeometry* debugGeom2 = QgsGeometry::fromPolyline( debugPoly2 );

  QgsPoint debugPt1, debugPt2;
  double debugDist;

  closestSegmentPoints( *debugGeom1, *debugGeom2, debugDist, debugPt1, debugPt2 );

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

  //configure distanceArea depending on minDistance units and output CRS
  QgsDistanceArea distanceArea;
  distanceArea.setSourceCrs( mStrataLayer->crs().srsid() );
  if ( mMinDistanceUnits == Meters )
  {
    distanceArea.setProjectionsEnabled( true );
  }
  else
  {
    distanceArea.setProjectionsEnabled( false );
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
      if ( otherTransectWithinDistance( lineClipStratum, minDistance, sIndex, lineFeatureMap, distanceArea ) )
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
    const QMap< QgsFeatureId, QgsGeometry* >& lineFeatureMap, QgsDistanceArea& da )
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
      //debug code
#if 0
      QgsPoint debugPoint1, debugPoint2;
      double dist1 = 0.0;
      closestSegmentPoints( *geom, *( idMapIt.value() ), dist1, debugPoint1, debugPoint2 );
      double dist2 = geom->distance( *( idMapIt.value() ) );
      if ( !doubleNear( dist1, dist2 ) )
      {
        closestSegmentPoints( *geom, *( idMapIt.value() ), dist1, debugPoint1, debugPoint2 );
      }
#endif //0
      double dist = 0;
      QgsPoint pt1, pt2;
      closestSegmentPoints( *geom, *( idMapIt.value() ), dist, pt1, pt2 );
      dist = da.measureLine( pt1, pt2 ); //convert degrees to meters if necessary

      if ( dist < minDistance )
      {
        delete buffer;
        return true;
      }
    }
  }

  delete buffer;
  return false;
}

bool QgsTransectSample::closestSegmentPoints( QgsGeometry& g1, QgsGeometry& g2, double& dist, QgsPoint& pt1, QgsPoint& pt2 )
{
  QGis::WkbType t1 = g1.wkbType();
  if ( t1 != QGis::WKBLineString && t1 != QGis::WKBLineString25D )
  {
    return false;
  }

  QGis::WkbType t2 = g2.wkbType();
  if ( t2 != QGis::WKBLineString && t2 != QGis::WKBLineString25D )
  {
    return false;
  }

  QgsPolyline pl1 = g1.asPolyline();
  QgsPolyline pl2 = g2.asPolyline();

  if ( pl1.size() < 2 || pl2.size() < 2 )
  {
    return false;
  }

  QgsPoint p11 = pl1.at( 0 );
  QgsPoint p12 = pl1.at( 1 );
  QgsPoint p21 = pl2.at( 0 );
  QgsPoint p22 = pl2.at( 1 );

  double p1x = p11.x();
  double p1y = p11.y();
  double v1x = p12.x() - p11.x();
  double v1y = p12.y() - p11.y();
  double p2x = p21.x();
  double p2y = p21.y();
  double v2x = p22.x() - p21.x();
  double v2y = p22.y() - p21.y();

  double denominatorU = v2x * v1y - v2y * v1x;
  double denominatorT = v1x * v2y - v1y * v2x;

  if ( doubleNear( denominatorU, 0 ) || doubleNear( denominatorT, 0 ) )
  {
    //lines are parallel
    //project all points on the other segment and take the one with the smallest distance
    QgsPoint minDistPoint1;
    double d1 = p11.sqrDistToSegment( p21.x(), p21.y(), p22.x(), p22.y(), minDistPoint1 );
    QgsPoint minDistPoint2;
    double d2 = p12.sqrDistToSegment( p21.x(), p21.y(), p22.x(), p22.y(), minDistPoint2 );
    QgsPoint minDistPoint3;
    double d3 = p21.sqrDistToSegment( p11.x(), p11.y(), p12.x(), p12.y(), minDistPoint3 );
    QgsPoint minDistPoint4;
    double d4 = p22.sqrDistToSegment( p11.x(), p11.y(), p12.x(), p12.y(), minDistPoint4 );

    if ( d1 <= d2 && d1 <= d3 && d1 <= d4 )
    {
      dist = sqrt( d1 ); pt1 = p11; pt2 = minDistPoint1;
      return true;
    }
    else if ( d2 <= d1 && d2 <= d3 && d2 <= d4 )
    {
      dist = sqrt( d2 );  pt1 = p12; pt2 = minDistPoint2;
      return true;
    }
    else if ( d3 <= d1 && d3 <= d2 && d3 <= d4 )
    {
      dist = sqrt( d3 ); pt1 = p21; pt2 = minDistPoint3;
      return true;
    }
    else
    {
      dist = sqrt( d4 ); pt1 = p21; pt2 = minDistPoint4;
      return true;
    }
  }

  double u = ( p1x * v1y - p1y * v1x - p2x * v1y + p2y * v1x ) / denominatorU;
  double t = ( p2x * v2y - p2y * v2x - p1x * v2y + p1y * v2x ) / denominatorT;

  if ( u >= 0 && u <= 1.0 && t >= 0 && t <= 1.0 )
  {
    dist = 0;
    pt1.setX( p2x + u * v2x );
    pt1.setY( p2y + u * v2y );
    pt2 = pt1;
    dist = 0;
    return true;
  }

  if ( t > 1.0 )
  {
    pt1.setX( p12.x() );
    pt1.setY( p12.y() );
  }
  else if ( t < 0.0 )
  {
    pt1.setX( p11.x() );
    pt1.setY( p11.y() );
  }
  if ( u > 1.0 )
  {
    pt2.setX( p22.x() );
    pt2.setY( p22.y() );
  }
  if ( u < 0.0 )
  {
    pt2.setX( p21.x() );
    pt2.setY( p21.y() );
  }
  if ( t >= 0.0 && t <= 1.0 )
  {
    //project pt2 onto g1
    pt2.sqrDistToSegment( p11.x(), p11.y(), p12.x(), p12.y(), pt1 );
  }
  if ( u >= 0.0 && u <= 1.0 )
  {
    //project pt1 onto g2
    pt1.sqrDistToSegment( p21.x(), p21.y(), p22.x(), p22.y(), pt2 );
  }

  dist = sqrt( pt1.sqrDist( pt2 ) );
  return true;
}
