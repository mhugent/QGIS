#include "qgsrasterfilewriter.h"
#include "qgsproviderregistry.h"
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

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( QgsRasterDataProvider* sourceProvider, int nCols, int nRows )
{
  if ( !sourceProvider || ! sourceProvider->isValid() )
  {
    return SourceProviderError;
  }

  QgsRasterDataProvider* destProvider = QgsRasterLayer::loadProvider( mOutputProviderKey, mOutputUrl );
  if ( !destProvider )
  {
    return DestProviderError;
  }

  QgsRectangle sourceProviderRect = sourceProvider->extent();
  double geoTransform[6];
  geoTransform[0] = sourceProviderRect.xMinimum();
  geoTransform[1] = sourceProviderRect.width() / nCols;
  geoTransform[2] = 0.0;
  geoTransform[3] = sourceProviderRect.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -( sourceProviderRect.height() / nRows );

  if ( !destProvider->create( mOutputFormat, sourceProvider->bandCount(), ( QgsRasterDataProvider::DataType )sourceProvider->dataType( 1 ), nCols, nRows, geoTransform,
                              sourceProvider->crs() ) )
  {
    delete destProvider;
    return CreateDatasourceError;
  }



  //read/write data for each band
  for ( int i = 0; i < sourceProvider->bandCount(); ++i )
  {
    void* data = VSIMalloc( destProvider->dataTypeSize( i + 1 ) * nCols * nRows );
    sourceProvider->readBlock( i + 1, sourceProviderRect, nCols, nRows, data );
    bool writeSuccess = destProvider->write( data, i + 1, nCols, nRows, 0, 0 );
    CPLFree( data );
    if ( !writeSuccess )
    {
      delete destProvider;
      return WriteError;
    }
  }

  delete destProvider;
  return NoError;
}
