#include "qgsrasteriterator.h"
#include "qgsrasterprojector.h"
#include <QObject>

QgsRasterIterator::QgsRasterIterator( QgsRasterDataProvider* provider ): mDataProvider( provider ), mRasterProjector( 0 ),
    mMaximumTileWidth( 500 ), mMaximumTileHeight( 500 )
{

}

QgsRasterIterator::~QgsRasterIterator()
{
    delete mRasterProjector;
}

void QgsRasterIterator::select( QgsRectangle& extent )
{

}

bool QgsRasterIterator::nextPart( void* data, QgsRectangle& mapRect, int& left, int& top, int& nCols, int& nRows )
{
    Q_UNUSED( data );
    Q_UNUSED( mapRect );
    Q_UNUSED( left );
    Q_UNUSED( top );
    Q_UNUSED( nCols );
    Q_UNUSED( nRows );
    return false; //soon...
}
