#ifndef QGSRASTERITERATOR_H
#define QGSRASTERITERATOR_H

#include "qgsrectangle.h"

class QgsRasterDataProvider;
class QgsRasterProjector;

class QgsRasterIterator
{
  public:
    QgsRasterIterator( int band, QgsRasterDataProvider* provider );
    ~QgsRasterIterator();
    /**Start raster read
        @param mapUnitsPerPixel raster resolution*/
    void select( QgsRectangle& extent, double mapUnitsPerPixel );
    /**Returns the next tile data (or false if end)*/
    bool nextPart( void* data, QgsRectangle& mapRect, int& left, int& top, int& nCols, int& nRows );

    //maximum pixel width/height of a part
    int maximumTileWidth() const { return mMaximumTileWidth; }
    void setMaximumTileWidth( int w ) { mMaximumTileWidth = w; }
    int maximumTileHeight() const { return mMaximumTileHeight; }
    void setMaximumTileHeight( int h ) { mMaximumTileHeight = h; }

    /**Set raster projector (takes ownership). Without projector, the provider CRS is used*/
    void setRasterProjector( QgsRasterProjector* projector ) { mRasterProjector = projector; }

  private:

    QgsRasterIterator(); //default constructor forbidden

    int mRasterBand;
    QgsRasterDataProvider* mDataProvider;
    QgsRasterProjector* mRasterProjector;
    /**Maximum tile width in pixels*/
    int mMaximumTileWidth;
    /**Maximum tile height in pixels*/
    int mMaximumTileHeight;

    //store information about read between select / nextPart
    int mNPixelsX;
    int mNPixelsY;
    int mNTilesX;
    int mNTilesY;
    int mCurrentTileX;
    int mCurrentTileY;
    QgsRectangle mSelectionRect;
    double mMapUnitsPerPixel;
};

#endif // QGSRASTERITERATOR_H
