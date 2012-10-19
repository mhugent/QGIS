class QgsRasterFileWriter
{
%TypeHeaderCode
#include <qgsrasterfilewriter.h>
%End
    public:
        enum WriterError
        {
            NoError = 0
        };

        QgsRasterFileWriter( const QString& outputUrl );
        ~QgsRasterFileWriter();
        WriterError writeRaster( QgsRasterDataProvider* sourceRaster, int nCols );

        void setTiledMode( bool t );
        bool tiledMode() const;

        void setMaxTileWidth( int w );
        int maxTileWidth() const;

        void setMaxTileHeight( int h );
        int maxTileHeight() const;
};
