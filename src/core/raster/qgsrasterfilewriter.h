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

  private:
    QgsRasterFileWriter(); //forbidden

    QString mOutputUrl;
    QString mOutputProviderKey;
    QString mOutputFormat;
    QgsCoordinateReferenceSystem mOutputCRS;
};

#endif // QGSRASTERFILEWRITER_H
