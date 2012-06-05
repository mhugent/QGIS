#ifndef QGSRASTERFILEWRITER_H
#define QGSRASTERFILEWRITER_H

#include "qgscoordinatereferencesystem.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

class QgsRasterDataProvider;

class QgsRasterFileWriter
{
  public:
    enum WriterError
    {
      NoError = 0,
      SourceProviderError = 1,
      DestProviderError = 2,
      CreateDatasourceError = 3,
      WriteError = 4
    };

    QgsRasterFileWriter( const QString& outputUrl );
    ~QgsRasterFileWriter();

    WriterError writeRaster( QgsRasterDataProvider* sourceProvider, int nCols );

    void setOutputFormat( const QString& format ) { mOutputFormat = format; }
    QString outputFormat() const { return mOutputFormat; }

    void setOutputProviderKey( const QString& key ) { mOutputProviderKey = key; }
    QString outputProviderKey() const { return mOutputProviderKey; }

    void setTiledMode( bool t ) { mTiledMode = t; }
    bool tiledMode() const { return mTiledMode; }

    void setMaxTileWidth( int w ) { mMaxTileWidth = w; }
    int maxTileWidth() const { return mMaxTileWidth; }

    void setMaxTileHeight( int h ) { mMaxTileHeight = h; }
    int maxTileHeight() const { return mMaxTileHeight; }

  private:
    QgsRasterFileWriter(); //forbidden
    WriterError writeRasterSingleTile( QgsRasterDataProvider* sourceProvider, int nCols );
    WriterError writeARGBRaster( QgsRasterDataProvider* sourceProvider, int nCols );

    //initialize vrt member variables
    void createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem& crs, double* geoTransform );
    //write vrt document to disk
    bool writeVRT( const QString& file );

    QString mOutputUrl;
    QString mOutputProviderKey;
    QString mOutputFormat;
    QgsCoordinateReferenceSystem mOutputCRS;

    /**False: Write one file, true: create a directory and add the files numbered*/
    bool mTiledMode;
    double mMaxTileWidth;
    double mMaxTileHeight;

    QDomDocument mVRTDocument;
    QDomElement mVRTRedBand;
    QDomElement mVRTGreenBand;
    QDomElement mVRTBlueBand;
    QDomElement mVRTAlphaBand;
};

#endif // QGSRASTERFILEWRITER_H
