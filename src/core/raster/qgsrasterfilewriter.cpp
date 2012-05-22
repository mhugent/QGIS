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

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( QgsRasterDataProvider* sourceRaster, int nCols, int nRows )
{
  if ( !sourceRaster || ! sourceRaster->isValid() ) //isValid() should be const
  {

  }

  QgsRasterLayer layer( mOutputUrl ); //only gdal for now...
  QgsRasterDataProvider* provider = layer.dataProvider();

  QgsRectangle sourceProviderRect = sourceRaster->extent();
  double geoTransform[6];
  geoTransform[0] = sourceProviderRect.xMinimum();
  geoTransform[1] = sourceProviderRect.width() / nCols;
  geoTransform[2] = 0.0;
  geoTransform[3] = sourceProviderRect.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -( sourceProviderRect.height() / nRows );

  //crs() should be const too
  if ( !provider->create( mOutputFormat, sourceRaster->bandCount(), ( QgsRasterDataProvider::DataType )sourceRaster->dataType( 1 ), nCols, nRows, geoTransform,
                          sourceRaster->crs() ) )
  {
    //error
  }



  //read/write data for each band
  for ( int i = 0; i < sourceRaster->bandCount(); ++i )
  {
    void* data = VSIMalloc( provider->dataTypeSize( i + 1 ) * nCols * nRows );
    sourceRaster->readBlock( i + 1, sourceProviderRect, nCols, nRows, data );
    if ( !provider->write( data, i + 1, nCols, nRows, 0, 0 ) )
    {
    }
    CPLFree( data );
  }


  return NoError;
}
