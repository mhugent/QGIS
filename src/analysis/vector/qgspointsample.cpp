#include "qgspointsample.h"
#include "qgsgeometry.h"
#include "qgsspatialindex.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include <QFile>


QgsPointSample::QgsPointSample( QgsVectorLayer* inputLayer, const QString& outputLayer, int nPointsAttribute, int minDistAttribute ): mInputLayer( inputLayer ),
    mOutputLayer( outputLayer ), mNumberOfPointsAttribute( nPointsAttribute ), mMinDistanceAttribute( minDistAttribute ), mNCreatedPoints( 0 )
{
}

QgsPointSample::QgsPointSample()
{
}

QgsPointSample::~QgsPointSample()
{
}

int QgsPointSample::createRandomPoints( QProgressDialog* pd )
{
  Q_UNUSED( pd );

  //create input layer from id (test if polygon, valid)
  if ( !mInputLayer )
  {
    return 1;
  }

  if ( mInputLayer->geometryType() != QGis::Polygon )
  {
    return 2;
  }

  //delete output file if it already exists
  if ( QFile::exists( mOutputLayer ) )
  {
    QgsVectorFileWriter::deleteShapeFile( mOutputLayer );
  }

  //create vector file writer
  QgsFieldMap outputFields;
  outputFields.insert( 0, QgsField( "feature_id", QVariant::Int ) );
  QgsVectorFileWriter writer( mOutputLayer, "UTF-8",
                              outputFields,
                              QGis::WKBPoint,
                              &( mInputLayer->crs() ) );

  //check if creation of output layer successfull
  if ( writer.hasError() != QgsVectorFileWriter::NoError )
  {
    return 3;
  }

  //init random number generator
  srand( QTime::currentTime().msec() );

  //iterate through input layer
  QgsAttributeList attList;
  attList << mNumberOfPointsAttribute;
  if ( mMinDistanceAttribute > 0 )
  {
    attList << mMinDistanceAttribute;
  }

  mInputLayer->select( attList );
  QgsFeature fet;
  int nPoints = 0;
  double minDistance = 0;
  mNCreatedPoints = 0;

  while ( mInputLayer->nextFeature( fet ) )
  {
    nPoints = fet.attributeMap()[mNumberOfPointsAttribute].toInt();
    if ( mMinDistanceAttribute > 0 )
    {
      minDistance = fet.attributeMap()[mMinDistanceAttribute].toDouble();
    }
    addSamplePoints( fet, writer, nPoints, minDistance );
  }

  return 0;
}

void QgsPointSample::addSamplePoints( QgsFeature& inputFeature, QgsVectorFileWriter& writer, int nPoints, double minDistance )
{
  QgsGeometry* geom = inputFeature.geometry();
  if ( !geom )
  {
    return;
  }

  QgsRectangle geomRect = geom->boundingBox();
  if ( geomRect.isEmpty() )
  {
    return;
  }

  QgsSpatialIndex sIndex; //to check minimum distance
  QMap< QgsFeatureId, QgsPoint > pointMapForFeature;

  int nIterations = 0;
  int maxIterations = nPoints * 20;
  int points = 0;

  double randX = 0;
  double randY = 0;

  while ( nIterations < maxIterations && points < nPoints )
  {
    randX = (( double )rand() / RAND_MAX ) * geomRect.width() + geomRect.xMinimum();
    randY = (( double )rand() / RAND_MAX ) * geomRect.height() + geomRect.yMinimum();
    QgsPoint randPoint( randX, randY );
    QgsGeometry* ptGeom = QgsGeometry::fromPoint( randPoint );
    if ( ptGeom->within( geom ) && checkMinDistance( randPoint, sIndex, minDistance, pointMapForFeature ) )
    {
      //add feature to writer
      QgsFeature f( mNCreatedPoints );
      f.addAttribute( 0, inputFeature.id() );
      f.setGeometry( ptGeom );
      writer.addFeature( f );
      sIndex.insertFeature( f );
      pointMapForFeature.insert( mNCreatedPoints, randPoint );
      ++points;
      ++mNCreatedPoints;
    }
    else
    {
      delete ptGeom;
    }
    ++nIterations;
  }
}

bool QgsPointSample::checkMinDistance( QgsPoint& pt, QgsSpatialIndex& index, double minDistance, QMap< QgsFeatureId, QgsPoint >& pointMap )
{
  if ( minDistance <= 0 )
  {
    return true;
  }

  QList<QgsFeatureId> neighborList = index.nearestNeighbor( pt, 1 );
  if ( neighborList.isEmpty() )
  {
    return true;
  }

  QMap< QgsFeatureId, QgsPoint >::const_iterator it = pointMap.find( neighborList[0] );
  if ( it == pointMap.constEnd() ) //should not happen
  {
    return true;
  }

  QgsPoint neighborPt = it.value();
  if ( neighborPt.sqrDist( pt ) < ( minDistance * minDistance ) )
  {
    return false;
  }
  return true;
}




