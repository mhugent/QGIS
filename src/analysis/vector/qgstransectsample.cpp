#include "qgstransectsample.h"
#include "qgsgeometry.h"
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
  outputPointFields.insert( 0, QgsField( "strata_id", QVariant::Int ) );
  QgsVectorFileWriter outputPointWriter( mOutputPointLayer, "utf-8", outputPointFields, QGis::WKBPoint,
                                         &( mStrataLayer->crs() ) );
  if ( outputPointWriter.hasError() != QgsVectorFileWriter::NoError )
  {
    return 3;
  }

  //iterate over strata layer
  QgsAttributeList attList;
  attList << mStrataIdAttribute;
  attList << mMinDistanceAttribute;
  attList << mNPointsAttribute;
  mStrataLayer->select( attList );
  QgsFeature fet;
  while ( mStrataLayer->nextFeature( fet ) )
  {
    QgsGeometry* strataGeom = fet.geometry();
    if ( !strataGeom )
    {
      continue;
    }

    //find baseline for strata
    QgsGeometry* baselineGeom = findBaselineGeometry( fet.attributeMap()[mStrataIdAttribute].toInt() );
    if ( !baselineGeom )
    {
      continue;
    }

    //clip baseline by strata
    QgsGeometry* clippedBaseline = strataGeom->intersection( baselineGeom );
    if ( !clippedBaseline )
    {
      continue;
    }

    //create line buffer and clip by strata
    double minDistance = fet.attributeMap()[mMinDistanceAttribute].toDouble();
    QgsGeometry* clipBaselineBuffer = clippedBaseline->buffer( minDistance, 8 );
    if ( !clipBaselineBuffer && !( clipBaselineBuffer->wkbType() == QGis::WKBPolygon ||
                                   clipBaselineBuffer->wkbType() == QGis::WKBPolygon25D ) )
    {
      continue;
    }

    QgsPolygon bufferPolygon = clipBaselineBuffer->asPolygon();
    QgsGeometry* bufferLine = QgsGeometry::fromPolyline( bufferPolygon[0] );
    QgsGeometry* bufferLineClipped = bufferLine->intersection( strataGeom );
    if ( !bufferLineClipped )
    {
      continue;
    }

    //start loop to create random points along the baseline
    int nTransects = fet.attributeMap()[mNPointsAttribute].toInt();
    int nCreatedTransects = 0;
    int nIterations = 0;
    int nMaxIterations = nTransects * 50;

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
      samplePointFeature.addAttribute( 0, fet.id() );
      outputPointWriter.addFeature( fet );
      ++nIterations;

      //find closest point on clipped buffer line

      //search closest existing profile. Cancel if dist < minDist
    }
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
    int id = fet.attributeMap()[mBaselineStrataId].toInt();
    if ( id == strataId )
    {
      return fet.geometryAndOwnership();
    }
  }
  return 0;
}
