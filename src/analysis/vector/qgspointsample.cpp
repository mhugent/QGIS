#include "qgspointsample.h"
#include "qgsgeometry.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"


QgsPointSample::QgsPointSample( QgsVectorLayer* inputLayer, const QString& outputLayer, int nPointsAttribute, int minDistAttribute ): mInputLayer( inputLayer ),
    mOutputLayer( outputLayer ), mNumberOfPointsAttribute( nPointsAttribute ), mMinDistanceAttribute( minDistAttribute )
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

  int nIterations = 0;
  int maxIterations = nPoints * 20;
  int points = 0;

  double randX = 0;
  double randY = 0;

  while ( nIterations < maxIterations && points < nPoints )
  {
    randX = (( double )rand() / RAND_MAX ) * geomRect.width() + geomRect.xMinimum();
    randY = (( double )rand() / RAND_MAX ) * geomRect.height() + geomRect.yMinimum();
    QgsGeometry* ptGeom = QgsGeometry::fromPoint( QgsPoint( randX, randY ) );
    if ( ptGeom->within( geom ) )
    {
      //add feature to writer
      QgsFeature f;
      f.addAttribute( 0, inputFeature.id() );
      f.setGeometry( ptGeom );
      writer.addFeature( f );
      ++points;
    }
    else
    {
      delete ptGeom;
    }
    ++nIterations;
  }
}


