#include "qgsrasterfilewriter.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"

QgsRasterFileWriter::QgsRasterFileWriter( const QString& outputUrl ): mOutputUrl( outputUrl ), mOutputProviderKey( "gdal" ), mOutputFormat( "GTiff" )
{

}

QgsRasterFileWriter::QgsRasterFileWriter()
{

}

QgsRasterFileWriter::~QgsRasterFileWriter()
{

}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( const QgsRasterDataProvider* sourceRaster, int nCols, int nRows )
{
  if ( !sourceRaster || ! const_cast<QgsRasterDataProvider*>( sourceRaster )->isValid() ) //isValid() should be const
  {

  }

  QgsRasterLayer layer( mOutputUrl ); //only gdal for now...
  QgsRasterDataProvider* provider = layer.dataProvider();

  QgsRectangle sourceProviderRect = const_cast<QgsRasterDataProvider*>( sourceRaster )->extent();
  double geoTransform[6];
  geoTransform[0] = sourceProviderRect.xMinimum();
  geoTransform[1] = sourceProviderRect.width() / nCols;
  geoTransform[2] = 0.0;
  geoTransform[3] = sourceProviderRect.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -( sourceProviderRect.height() / nRows );

  //crs() should be const too
  if ( !provider->create( mOutputFormat, sourceRaster->bandCount(), ( QgsRasterDataProvider::DataType )sourceRaster->dataType( 1 ), nCols, nRows, geoTransform, const_cast<QgsRasterDataProvider*>( sourceRaster )->crs() ) )
  {
    //error
  }

  //read data from sourceRaster

  //write into provider


  return NoError;
}
