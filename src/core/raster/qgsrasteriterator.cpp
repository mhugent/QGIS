#include "qgsrasteriterator.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterprojector.h"
#include <QObject>

QgsRasterIterator::QgsRasterIterator( int band, QgsRasterDataProvider* provider ): mRasterBand( band ), mDataProvider( provider ), mRasterProjector( 0 ),
    mMaximumTileWidth( 500 ), mMaximumTileHeight( 500 )
{

}

QgsRasterIterator::QgsRasterIterator()
{

}

QgsRasterIterator::~QgsRasterIterator()
{
  delete mRasterProjector;
}

void QgsRasterIterator::select( QgsRectangle& extent, double mapUnitsPerPixel )
{
  //get number of tiles in x/y direction
  mNPixelsX = extent.width() / mapUnitsPerPixel + 1;
  mNPixelsY = extent.height() / mapUnitsPerPixel + 1;
  mNTilesX = mNPixelsX / mMaximumTileWidth + 1;
  mNTilesY = mNPixelsY / mMaximumTileHeight + 1;
  mCurrentTileX = 0;
  mCurrentTileY = 0;
  mSelectionRect = extent;
  mMapUnitsPerPixel = mapUnitsPerPixel;
}

bool QgsRasterIterator::nextPart( void* data, QgsRectangle& mapRect, int& left, int& top, int& nCols, int& nRows )
{
  //get tile index to fetch
  if ( mCurrentTileX == mNTilesX )
  {
    ++mCurrentTileY;
    mCurrentTileX = 0;
  }
  if ( mCurrentTileY == mNTilesY )
  {
    return false; //at end of iteration
  }

  //get number of pixels to fetch
  nCols = mNPixelsX - mCurrentTileX * mMaximumTileWidth;
  if ( nCols > mMaximumTileWidth )
  {
    nCols = mMaximumTileWidth;
  }
  nRows = mNPixelsX - mCurrentTileY * mMaximumTileHeight;
  if ( nRows > mMaximumTileHeight )
  {
    nRows = mMaximumTileHeight;
  }
  if ( nCols <= 0 || nRows <= 0 )
  {
    ++mCurrentTileX;
    return true;
  }

  //calculate rectangle to fetch
  left = mCurrentTileX * mMaximumTileWidth;
  double mapLeft = mSelectionRect.xMinimum() + left * mMapUnitsPerPixel;
  top = mCurrentTileY * mMaximumTileHeight;
  double mapTop = mSelectionRect.yMaximum() - top * mMapUnitsPerPixel;
  double mapRight = mapLeft + nCols * mMapUnitsPerPixel;
  double mapBottom = mapTop - nRows * mMapUnitsPerPixel;
  mapRect = QgsRectangle( mapLeft, mapBottom, mapRight, mapTop );

  mDataProvider->readBlock( mRasterBand, mapRect, nCols, nRows, data );

  ++mCurrentTileX;
  return true;
}
