#ifndef QGSPNGIMAGEWRITER_H
#define QGSPNGIMAGEWRITER_H

class QImage;
class QIODevice;
class QString;

/**A copy of QPNGImageWriter with filter type 'none' for faster png generation*/
class QgsPNGImageWriter
{
  public:
    QgsPNGImageWriter( QIODevice* );
    ~QgsPNGImageWriter();

    enum DisposalMethod { Unspecified, NoDisposal, RestoreBackground, RestoreImage };
    void setDisposalMethod( DisposalMethod );
    void setLooping( int loops = 0 ); // 0 == infinity
    void setFrameDelay( int msecs );
    void setGamma( float );

    bool writeImage( const QImage& img, int x, int y );
    bool writeImage( const QImage& img, int quality, const QString &description, int x, int y );
    bool writeImage( const QImage& img )
    { return writeImage( img, 0, 0 ); }
    bool writeImage( const QImage& img, int quality, const QString &description )
    { return writeImage( img, quality, description, 0, 0 ); }

    QIODevice* device() { return dev; }

  private:
    QIODevice* dev;
    int frames_written;
    DisposalMethod disposal;
    int looping;
    int ms_delay;
    float gamma;
};

#endif // QGSPNGIMAGEWRITER_H
