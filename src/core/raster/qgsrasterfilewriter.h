#ifndef QGSRASTERFILEWRITER_H
#define QGSRASTERFILEWRITER_H

#include "qgscoordinatereferencesystem.h"
#include <QString>

class QgsRasterDataProvider;

class QgsRasterFileWriter
{
  public:
    enum WriterError
    {
      NoError = 0
    };

    QgsRasterFileWriter( const QString& outputUrl );
    ~QgsRasterFileWriter();
    WriterError writeRaster( QgsRasterDataProvider* sourceRaster, int nCols, int nRows );

  private:
    QgsRasterFileWriter(); //forbidden

    QString mOutputUrl;
    QString mOutputProviderKey;
    QString mOutputFormat;
    QgsCoordinateReferenceSystem mOutputCRS;
};

#endif // QGSRASTERFILEWRITER_H
