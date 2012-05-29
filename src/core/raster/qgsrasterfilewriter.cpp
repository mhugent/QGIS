#include "qgsrasterfilewriter.h"
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"

QgsRasterFileWriter::QgsRasterFileWriter( const QString& outputUrl ): mOutputUrl( outputUrl ), mOutputProviderKey( "gdal" ), mOutputFormat( "GTiff" ), mTiledMode( false ),
    mMaxTileWidth( 500 ), mMaxTileHeight( 500 )
{

}

QgsRasterFileWriter::QgsRasterFileWriter()
{

}

QgsRasterFileWriter::~QgsRasterFileWriter()
{

}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRaster( QgsRasterDataProvider* sourceProvider, int nCols )
{
  if ( mTiledMode )
  {
    return writeRasterTiled( sourceProvider, nCols );
  }
  else
  {
    return writeRasterSingleTile( sourceProvider, nCols );
  }
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRasterSingleTile( QgsRasterDataProvider* sourceProvider, int nCols )
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
  double pixelSize = sourceProviderRect.width() / nCols;
  int nRows = ( double )nCols / sourceProviderRect.width() * sourceProviderRect.height() + 0.5;
  double geoTransform[6];
  geoTransform[0] = sourceProviderRect.xMinimum();
  geoTransform[1] = pixelSize;
  geoTransform[2] = 0.0;
  geoTransform[3] = sourceProviderRect.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -pixelSize;

  //debug
  bool hasARGBType = false;
  QgsRasterDataProvider::DataType outputDataType = ( QgsRasterDataProvider::DataType )sourceProvider->dataType( 1 );
  int nOutputBands = sourceProvider->bandCount();
  int nInputBands = sourceProvider->bandCount();
  if ( outputDataType == QgsRasterDataProvider::ARGBDataType )
  {
    hasARGBType = true; //needs to be converted to four band 8bit
    outputDataType = QgsRasterDataProvider::Byte;
    nOutputBands = 4;
  }

  if ( !destProvider->create( mOutputFormat, nOutputBands, outputDataType, nCols, nRows, geoTransform,
                              sourceProvider->crs() ) )
  {
    delete destProvider;
    return CreateDatasourceError;
  }

  if ( hasARGBType && nInputBands == 1 )
  {
    //For ARGB data, always use 1 input band and four int8 output bands
    int nPixels = nCols * nRows;
    int dataSize = destProvider->dataTypeSize( 1 ) * nPixels;
    void* data = VSIMalloc( dataSize );
    sourceProvider->readBlock( 1, sourceProviderRect, nCols, nRows, data );

    //data for output bands
    void* redData = VSIMalloc( nPixels );
    void* greenData = VSIMalloc( nPixels );
    void* blueData = VSIMalloc( nPixels );
    void* alphaData = VSIMalloc( nPixels );

    int red = 0;
    int green = 0;
    int blue = 0;
    int alpha = 255;
    uint* p = ( uint* ) data;
    for ( int i = 0; i < nPixels; ++i )
    {
      QRgb c( *p++ );
      red = qRed( c ); green = qGreen( c ); blue = qBlue( c ); alpha = qAlpha( c );
      memcpy( redData + i, &red, 1 );
      memcpy( greenData + i, &green, 1 );
      memcpy( blueData + i, &blue, 1 );
      memcpy( alphaData + i, &alpha, 1 );
    }
    destProvider->write( redData, 1, nCols, nRows, 0, 0 );
    destProvider->write( greenData, 2, nCols, nRows, 0, 0 );
    destProvider->write( blueData, 3, nCols, nRows, 0, 0 );
    destProvider->write( alphaData, 4, nCols, nRows, 0, 0 );
  }
  else
  {
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
  }

  delete destProvider;
  return NoError;
}

QgsRasterFileWriter::WriterError QgsRasterFileWriter::writeRasterTiled( QgsRasterDataProvider* sourceProvider, int nCols )
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

  //create directory for output files
  QDir destDir( mOutputUrl );
  destDir.mkdir( mOutputUrl );

  bool hasARGBType = false;
  QgsRasterDataProvider::DataType outputDataType = ( QgsRasterDataProvider::DataType )sourceProvider->dataType( 1 );
  int nOutputBands = sourceProvider->bandCount();
  int nInputBands = sourceProvider->bandCount();
  if ( outputDataType == QgsRasterDataProvider::ARGBDataType )
  {
    hasARGBType = true; //needs to be converted to four band 8bit
    outputDataType = QgsRasterDataProvider::Byte;
    nOutputBands = 4;
  }

  //Get output map units per pixel
  QgsRectangle outputExtent = sourceProvider->extent();
  double outputMapUnitsPerPixel = outputExtent.width() / nCols;

  if ( hasARGBType && nInputBands == 1 )
  {
    QgsRasterIterator iter( 1, sourceProvider );
    iter.setMaximumTileWidth( mMaxTileWidth );
    iter.setMaximumTileHeight( mMaxTileHeight );
    void* data = VSIMalloc( sourceProvider->dataTypeSize( 1 ) * mMaxTileWidth * mMaxTileHeight );
    void* redData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
    void* greenData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
    void* blueData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
    void* alphaData = VSIMalloc( mMaxTileWidth * mMaxTileHeight );
    QgsRectangle mapRect;
    int iterLeft, iterTop, iterCols, iterRows;
    int fileIndex = 0;

    iter.select( outputExtent, outputMapUnitsPerPixel );
    while ( iter.nextPart( data, mapRect, iterLeft, iterTop, iterCols, iterRows ) )
    {
      if ( iterCols <= 0 || iterRows <= 0 )
      {
        continue;
      }

      //fill into red/green/blue/alpha channels
      uint* p = ( uint* ) data;
      int nPixels = iterCols * iterRows;
      int red = 0;
      int green = 0;
      int blue = 0;
      int alpha = 255;
      for ( int i = 0; i < nPixels; ++i )
      {
        QRgb c( *p++ );
        red = qRed( c ); green = qGreen( c ); blue = qBlue( c ); alpha = qAlpha( c );
        memcpy( redData + i, &red, 1 );
        memcpy( greenData + i, &green, 1 );
        memcpy( blueData + i, &blue, 1 );
        memcpy( alphaData + i, &alpha, 1 );
      }

      //create output file
      QString outputFile = mOutputUrl + "/" + QString::number( fileIndex );
      QgsRasterDataProvider* destProvider = QgsRasterLayer::loadProvider( mOutputProviderKey, outputFile );
      if ( !destProvider )
      {
        return DestProviderError;
      }

      //geotransform
      double geoTransform[6];
      geoTransform[0] = mapRect.xMinimum();
      geoTransform[1] = outputMapUnitsPerPixel;
      geoTransform[2] = 0.0;
      geoTransform[3] = mapRect.yMaximum();
      geoTransform[4] = 0.0;
      geoTransform[5] = -outputMapUnitsPerPixel;
      if ( !destProvider->create( mOutputFormat, 4, QgsRasterDataProvider::Byte, iterCols, iterRows, geoTransform,
                                  sourceProvider->crs() ) )
      {
        delete destProvider;
        return CreateDatasourceError;
      }

      //write data to output file
      destProvider->write( redData, 1, iterCols, iterRows, 0, 0 );
      destProvider->write( greenData, 2, iterCols, iterRows, 0, 0 );
      destProvider->write( blueData, 3, iterCols, iterRows, 0, 0 );
      destProvider->write( alphaData, 4, iterCols, iterRows, 0, 0 );

      delete destProvider;
      ++fileIndex;
    }
    CPLFree( data ); CPLFree( redData ); CPLFree( greenData ); CPLFree( blueData ); CPLFree( alphaData );
  }
  else
  {

  }


  return NoError;
}
