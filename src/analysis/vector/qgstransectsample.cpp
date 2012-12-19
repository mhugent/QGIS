#include "qgstransectsample.h"
#include "qgsdistancearea.h"
#include "qgsgeometry.h"
#include "qgsspatialindex.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

QgsTransectSample::QgsTransectSample( QgsVectorLayer* strataLayer, int strataIdAttribute, int minDistanceAttribute, DistanceUnits minDistUnits,
                                      int nPointsAttribute, QgsVectorLayer* baselineLayer, bool shareBaseline,
                                      int baselineStrataId, const QString& outputPointLayer,
                                      const QString& outputLineLayer, const QString& usedBaselineLayer ): mStrataLayer( strataLayer ),
    mStrataIdAttribute( strataIdAttribute ), mMinDistanceAttribute( minDistanceAttribute ),
    mNPointsAttribute( nPointsAttribute ), mBaselineLayer( baselineLayer ), mShareBaseline( shareBaseline ),
    mBaselineStrataId( baselineStrataId ), mOutputPointLayer( outputPointLayer ), mOutputLineLayer( outputLineLayer ), mUsedBaselineLayer( usedBaselineLayer ),
    mMinDistanceUnits( minDistUnits )
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
  outputPointFields.insert( 3, QgsField( "station_code", QVariant::String ) );
  outputPointFields.insert( 4, QgsField( "start_lat", QVariant::Double ) );
  outputPointFields.insert( 5, QgsField( "start_long", QVariant::Double ) );

  QgsVectorFileWriter outputPointWriter( mOutputPointLayer, "utf-8", outputPointFields, QGis::WKBPoint,
                                         &( mStrataLayer->crs() ) );
  if ( outputPointWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 3;
  }

  outputPointFields.insert( 6, QgsField( "bearing", QVariant::Double ) ); //add bearing attribute for lines
  QgsVectorFileWriter outputLineWriter( mOutputLineLayer, "utf-8", outputPointFields, QGis::WKBLineString,
                                        &( mStrataLayer->crs() ) );
  if ( outputLineWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 4;
  }

  QgsFieldMap usedBaselineFields;
  usedBaselineFields.insert( 0, QgsField( "stratum_id", QVariant::Int ) );
  usedBaselineFields.insert( 1, QgsField( "ok", QVariant::String ) );
  QgsVectorFileWriter usedBaselineWriter( mUsedBaselineLayer, "utf-8", usedBaselineFields, QGis::WKBLineString,
                                          &( mStrataLayer->crs() ) );
  if ( usedBaselineWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 5;
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

  //possibility to transform output points to lat/long
  QgsCoordinateTransform toLatLongTransform( mStrataLayer->crs(), QgsCoordinateReferenceSystem( 4326, QgsCoordinateReferenceSystem::EpsgCrsId ) );

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

    //stratum could be a multipolygon, so we need another loop over the single polygon parts
    QList<QgsGeometry*> strataPolygonGeoms;
    if ( strataGeom->wkbType() == QGis::WKBPolygon || strataGeom->wkbType() == QGis::WKBPolygon25D )
    {
      strataPolygonGeoms.push_back( new QgsGeometry( *strataGeom ) );
    }
    else if ( strataGeom->wkbType() ==  QGis::WKBMultiPolygon || strataGeom->wkbType() ==  QGis::WKBMultiPolygon25D )
    {
      QgsMultiPolygon multiPoly = strataGeom->asMultiPolygon();
      for ( int i = 0; i < multiPoly.size(); ++i )
      {
        strataPolygonGeoms.push_back( QgsGeometry::fromPolygon( multiPoly[i] ) );
      }
    }

    for ( int i = 0; i < strataPolygonGeoms.size(); ++i )
    {
      QgsGeometry* stratumPolygonGeom = strataPolygonGeoms.at( i );

      //clip baseline by strata
      QgsGeometry* clippedBaseline = stratumPolygonGeom->intersection( baselineGeom );
      if ( !clippedBaseline || clippedBaseline->wkbType() == QGis::WKBUnknown )
      {
        continue;
      }

      //save clipped baseline to file
      QgsFeature blFeature;
      blFeature.setGeometry( *clippedBaseline );
      blFeature.addAttribute( 0, strataId );
      blFeature.addAttribute( 1, "f" );
      usedBaselineWriter.addFeature( blFeature );

      //create line buffer and clip by strata

      //if minDistance is in meters and the data in degrees, we need to apply a rough conversion for the buffer distance
      double bufferDist = minDistance;
      if ( mMinDistanceUnits == Meters && mStrataLayer->crs().mapUnits() == QGis::DecimalDegrees )
      {
        bufferDist = minDistance / 111319.9;
      }

      QgsGeometry* clipBaselineBuffer = clippedBaseline->buffer( bufferDist, 8 );
      if ( !clipBaselineBuffer && !( clipBaselineBuffer->wkbType() == QGis::WKBPolygon ||
                                     clipBaselineBuffer->wkbType() == QGis::WKBPolygon25D ) )
      {
        delete clippedBaseline; delete clipBaselineBuffer;
        continue;
      }

      QgsPolygon bufferPolygon = clipBaselineBuffer->asPolygon();
      if ( bufferPolygon.size() < 1 )
      {
        delete clippedBaseline; delete clipBaselineBuffer;
        continue;
      }
      QgsGeometry* bufferLine = QgsGeometry::fromPolyline( bufferPolygon[0] );
      QgsGeometry* bufferLineClipped = bufferLine->intersection( stratumPolygonGeom );
      if ( !bufferLineClipped )
      {
        delete clippedBaseline; delete clipBaselineBuffer; delete bufferLine;
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
        QgsPoint sampleQgsPoint = samplePoint->asPoint();
        QgsPoint latLongSamplePoint = toLatLongTransform.transform( sampleQgsPoint );

        QgsFeature samplePointFeature;
        samplePointFeature.setGeometry( samplePoint );
        samplePointFeature.addAttribute( 0, nTotalTransects );
        samplePointFeature.addAttribute( 1, nCreatedTransects );
        samplePointFeature.addAttribute( 2, strataId );
        samplePointFeature.addAttribute( 3, QString::number( strataId ) + "_" + QString::number( nCreatedTransects ) );
        samplePointFeature.addAttribute( 4, latLongSamplePoint.y() );
        samplePointFeature.addAttribute( 5, latLongSamplePoint.x() );

        //find closest point on clipped buffer line
        QgsPoint minDistPoint;

        int afterVertex;
        if ( bufferLineClipped->closestSegmentWithContext( sampleQgsPoint, minDistPoint, afterVertex ) < 0 )
        {
          continue;
        }

        //bearing between sample point and min dist point (transect direction)
        double bearing = distanceArea.bearing( sampleQgsPoint, minDistPoint ) / M_PI * 180.0;

        QgsPolyline sampleLinePolyline;
        QgsPoint ptFarAway( sampleQgsPoint.x() + ( minDistPoint.x() - sampleQgsPoint.x() ) * 1000000,
                            sampleQgsPoint.y() + ( minDistPoint.y() - sampleQgsPoint.y() ) * 1000000 );
        QgsPolyline lineFarAway;
        lineFarAway << sampleQgsPoint << ptFarAway;
        QgsGeometry* lineFarAwayGeom = QgsGeometry::fromPolyline( lineFarAway );
        QgsGeometry* lineClipStratum = lineFarAwayGeom->intersection( stratumPolygonGeom );
        if ( !lineClipStratum )
        {
          delete lineFarAwayGeom; delete lineClipStratum;
          continue;
        }

        //if lineClipStratum is a multiline, take the part line closest to sampleQgsPoint
        if ( lineClipStratum->wkbType() == QGis::WKBMultiLineString
             || lineClipStratum->wkbType() == QGis::WKBMultiLineString25D )
        {
          QgsGeometry* singleLine = closestMultilineElement( sampleQgsPoint, lineClipStratum );
          if ( singleLine )
          {
            delete lineClipStratum;
            lineClipStratum = singleLine;
          }
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
        sampleLineFeature.addAttribute( 2, strataId );
        sampleLineFeature.addAttribute( 3, QString::number( strataId ) + "_" + QString::number( nCreatedTransects ) );
        sampleLineFeature.addAttribute( 4, latLongSamplePoint.y() );
        sampleLineFeature.addAttribute( 5, latLongSamplePoint.x() );
        sampleLineFeature.addAttribute( 6, bearing );
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
      delete clippedBaseline; delete clipBaselineBuffer; delete bufferLine;
      delete bufferLineClipped;

      //delete all line geometries in spatial index
      QMap< QgsFeatureId, QgsGeometry* >::iterator featureMapIt = lineFeatureMap.begin();
      for ( ; featureMapIt != lineFeatureMap.end(); ++featureMapIt )
      {
        delete( featureMapIt.value() );
      }
      lineFeatureMap.clear();
    }

    //delete stratum geometries
    QList<QgsGeometry*>::const_iterator delIt = strataPolygonGeoms.constBegin();
    for ( ; delIt != strataPolygonGeoms.constEnd(); ++delIt )
    {
      delete( *delIt );
    }
    delete baselineGeom;
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

QgsGeometry* QgsTransectSample::closestMultilineElement( const QgsPoint& pt, QgsGeometry* multiLine )
{
  if ( !multiLine || ( multiLine->wkbType() != QGis::WKBMultiLineString
                       && multiLine->wkbType() != QGis::WKBMultiLineString25D ) )
  {
    return 0;
  }

  double minDist = DBL_MAX;
  double currentDist = 0;
  QgsGeometry* currentLine = 0;
  QgsGeometry* closestLine = 0;
  QgsGeometry* pointGeom = QgsGeometry::fromPoint( pt );

  QgsMultiPolyline multiPolyline = multiLine->asMultiPolyline();
  QgsMultiPolyline::const_iterator it = multiPolyline.constBegin();
  for ( ; it != multiPolyline.constEnd(); ++it )
  {
    currentLine = QgsGeometry::fromPolyline( *it );
    currentDist = pointGeom->distance( *currentLine );
    if ( currentDist < minDist )
    {
      minDist = currentDist;
      closestLine = currentLine;
    }
    else
    {
      delete currentLine;
    }
  }

  delete pointGeom;
  return closestLine;
}
