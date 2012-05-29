#ifndef QGSRASTERITERATOR_H
#define QGSRASTERITERATOR_H

class QgsRasterDataProvider;
class QgsRasterProjector;
class QgsRectangle;

class QgsRasterIterator
{
    public:
        QgsRasterIterator( QgsRasterDataProvider* provider );
        ~QgsRasterIterator();
        /**Start raster read*/
        void select( QgsRectangle& extent );
        /**Returns the next tile data (or false if end)*/
        bool nextPart( void* data, QgsRectangle& mapRect, int& left, int& top, int& nCols, int& nRows );

        //maximum pixel width/height of a part
        int maximumTileWidth() const { return mMaximumTileWidth; }
        void setMaximumTileWidth( int w ){ mMaximumTileWidth = w; }
        int maximumTileHeight() const { return mMaximumTileHeight; }
        void setMaximumTileHeight( int h ){ mMaximumTileHeight = h; }

        /**Set raster projector (takes ownership)*/
        void setRasterProjector( QgsRasterProjector* projector ){ mRasterProjector = projector; }

    private:
        QgsRasterDataProvider* mDataProvider;
        QgsRasterProjector* mRasterProjector;
        /**Maximum tile width in pixels*/
        int mMaximumTileWidth;
        /**Maximum tile height in pixels*/
        int mMaximumTileHeight;
};

#endif // QGSRASTERITERATOR_H
