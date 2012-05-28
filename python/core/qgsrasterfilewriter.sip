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
        WriterError writeRaster( const QgsRasterDataProvider* sourceRaster, int nCols );
};
