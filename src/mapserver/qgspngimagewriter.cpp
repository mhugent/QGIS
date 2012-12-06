#include "qgspngimagewriter.h"

//Qt includes
#include <QImage>
#include <QMap>
#include <QStringList>

//libpng includes
#include <png.h>
#include <pngconf.h>

static void qt_png_warning( png_structp /*png_ptr*/, png_const_charp message )
{
  qWarning( "libpng warning: %s", message );
}

static void qpiw_write_fn( png_structp png_ptr, png_bytep data, png_size_t length )
{
  QgsPNGImageWriter* qpiw = ( QgsPNGImageWriter* )png_get_io_ptr( png_ptr );
  QIODevice* out = qpiw->device();

  uint nr = out->write(( char* )data, length );
  if ( nr != length )
  {
    png_error( png_ptr, "Write Error" );
    return;
  }
}

static void qpiw_flush_fn( png_structp /* png_ptr */ )
{
}

QgsPNGImageWriter::QgsPNGImageWriter( QIODevice* iod ) :
    dev( iod ),
    frames_written( 0 ),
    disposal( Unspecified ),
    looping( -1 ),
    ms_delay( -1 ),
    gamma( 0.0 )
{
}

QgsPNGImageWriter::~QgsPNGImageWriter()
{
}

void QgsPNGImageWriter::setDisposalMethod( DisposalMethod dm )
{
  disposal = dm;
}

void QgsPNGImageWriter::setLooping( int loops )
{
  looping = loops;
}

void QgsPNGImageWriter::setFrameDelay( int msecs )
{
  ms_delay = msecs;
}

void QgsPNGImageWriter::setGamma( float g )
{
  gamma = g;
}


#ifndef QT_NO_IMAGE_TEXT
static void set_text( const QImage &image, png_structp png_ptr, png_infop info_ptr,
                      const QString &description )
{
  QMap<QString, QString> text;
  foreach ( const QString &key, image.textKeys() )
  {
    if ( !key.isEmpty() )
      text.insert( key, image.text( key ) );
  }
  foreach ( const QString &pair, description.split( QLatin1String( "\n\n" ) ) )
  {
    int index = pair.indexOf( QLatin1Char( ':' ) );
    if ( index >= 0 && pair.indexOf( QLatin1Char( ' ' ) ) < index )
    {
      QString s = pair.simplified();
      if ( !s.isEmpty() )
        text.insert( QLatin1String( "Description" ), s );
    }
    else
    {
      QString key = pair.left( index );
      if ( !key.simplified().isEmpty() )
        text.insert( key, pair.mid( index + 2 ).simplified() );
    }
  }

  if ( text.isEmpty() )
    return;

  png_textp text_ptr = new png_text[text.size()];
  qMemSet( text_ptr, 0, text.size() * sizeof( png_text ) );

  QMap<QString, QString>::ConstIterator it = text.constBegin();
  int i = 0;
  while ( it != text.constEnd() )
  {
    text_ptr[i].key = qstrdup( it.key().left( 79 ).toLatin1().constData() );
    bool noCompress = ( it.value().length() < 40 );

#ifdef PNG_iTXt_SUPPORTED
    bool needsItxt = false;
    foreach ( const QChar c, it.value() )
    {
      uchar ch = c.cell();
      if ( c.row() || ( ch < 0x20 && ch != '\n' ) || ( ch > 0x7e && ch < 0xa0 ) )
      {
        needsItxt = true;
        break;
      }
    }

    if ( needsItxt )
    {
      text_ptr[i].compression = noCompress ? PNG_ITXT_COMPRESSION_NONE : PNG_ITXT_COMPRESSION_zTXt;
      QByteArray value = it.value().toUtf8();
      text_ptr[i].text = qstrdup( value.constData() );
      text_ptr[i].itxt_length = value.size();
      text_ptr[i].lang = const_cast<char*>( "UTF-8" );
      text_ptr[i].lang_key = qstrdup( it.key().toUtf8().constData() );
    }
    else
#endif
    {
      text_ptr[i].compression = noCompress ? PNG_TEXT_COMPRESSION_NONE : PNG_TEXT_COMPRESSION_zTXt;
      QByteArray value = it.value().toLatin1();
      text_ptr[i].text = qstrdup( value.constData() );
      text_ptr[i].text_length = value.size();
    }
    ++i;
    ++it;
  }

  png_set_text( png_ptr, info_ptr, text_ptr, i );
  for ( i = 0; i < text.size(); ++i )
  {
    delete [] text_ptr[i].key;
    delete [] text_ptr[i].text;
#ifdef PNG_iTXt_SUPPORTED
    delete [] text_ptr[i].lang_key;
#endif
  }
  delete [] text_ptr;
}
#endif

bool QgsPNGImageWriter::writeImage( const QImage& image, int off_x, int off_y )
{
  return writeImage( image, -1, QString(), off_x, off_y );
}

bool QgsPNGImageWriter::writeImage( const QImage& image, int quality_in, const QString &description,
                                    int off_x_in, int off_y_in )
{
#ifdef QT_NO_IMAGE_TEXT
  Q_UNUSED( description );
#endif

  QPoint offset = image.offset();
  int off_x = off_x_in + offset.x();
  int off_y = off_y_in + offset.y();

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
  if ( !png_ptr )
  {
    return false;
  }

  png_set_error_fn( png_ptr, 0, 0, qt_png_warning );

  info_ptr = png_create_info_struct( png_ptr );
  if ( !info_ptr )
  {
    png_destroy_write_struct( &png_ptr, 0 );
    return false;
  }

  if ( setjmp( png_jmpbuf( png_ptr ) ) )
  {
    png_destroy_write_struct( &png_ptr, &info_ptr );
    return false;
  }

  int quality = quality_in;
  if ( quality >= 0 )
  {
    if ( quality > 9 )
    {
      qWarning( "PNG: Quality %d out of range", quality );
      quality = 9;
    }
    png_set_compression_level( png_ptr, quality );
  }
  //png_set_filter (png_ptr,0, PNG_FILTER_NONE);
  png_set_write_fn( png_ptr, ( void* )this, qpiw_write_fn, qpiw_flush_fn );


  int color_type = 0;
  if ( image.colorCount() )
    color_type = PNG_COLOR_TYPE_PALETTE;
  else if ( image.hasAlphaChannel() )
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  else
    color_type = PNG_COLOR_TYPE_RGB;

  png_set_IHDR( png_ptr, info_ptr, image.width(), image.height(),
                image.depth() == 1 ? 1 : 8, // per channel
                color_type, 0, 0, 0 );      // sets #channels

  if ( gamma != 0.0 )
  {
    png_set_gAMA( png_ptr, info_ptr, 1.0 / gamma );
  }

  png_color_8 sig_bit;
  sig_bit.red = 8;
  sig_bit.green = 8;
  sig_bit.blue = 8;
  sig_bit.alpha = image.hasAlphaChannel() ? 8 : 0;
  png_set_sBIT( png_ptr, info_ptr, &sig_bit );

  if ( image.format() == QImage::Format_MonoLSB )
    png_set_packswap( png_ptr );

  if ( image.colorCount() )
  {
    // Paletted
    int num_palette = qMin( 256, image.colorCount() );
    png_color palette[256];
    png_byte trans[256];
    int num_trans = 0;
    for ( int i = 0; i < num_palette; i++ )
    {
      QRgb rgba = image.color( i );
      palette[i].red = qRed( rgba );
      palette[i].green = qGreen( rgba );
      palette[i].blue = qBlue( rgba );
      trans[i] = qAlpha( rgba );
      if ( trans[i] < 255 )
      {
        num_trans = i + 1;
      }
    }
    png_set_PLTE( png_ptr, info_ptr, palette, num_palette );

    if ( num_trans )
    {
      png_set_tRNS( png_ptr, info_ptr, trans, num_trans, 0 );
    }
  }

  // Swap ARGB to RGBA (normal PNG format) before saving on
  // BigEndian machines
  if ( QSysInfo::ByteOrder == QSysInfo::BigEndian )
  {
    png_set_swap_alpha( png_ptr );
  }

  // Qt==ARGB==Big(ARGB)==Little(BGRA). But RGB888 is RGB regardless
  if ( QSysInfo::ByteOrder == QSysInfo::LittleEndian
       && image.format() != QImage::Format_RGB888 )
  {
    png_set_bgr( png_ptr );
  }

  if ( off_x || off_y )
  {
    png_set_oFFs( png_ptr, info_ptr, off_x, off_y, PNG_OFFSET_PIXEL );
  }

  if ( frames_written > 0 )
    png_set_sig_bytes( png_ptr, 8 );

  if ( image.dotsPerMeterX() > 0 || image.dotsPerMeterY() > 0 )
  {
    png_set_pHYs( png_ptr, info_ptr,
                  image.dotsPerMeterX(), image.dotsPerMeterY(),
                  PNG_RESOLUTION_METER );
  }

#ifndef QT_NO_IMAGE_TEXT
  set_text( image, png_ptr, info_ptr, description );
#endif
  png_write_info( png_ptr, info_ptr );

  if ( image.depth() != 1 )
    png_set_packing( png_ptr );

  if ( color_type == PNG_COLOR_TYPE_RGB && image.format() != QImage::Format_RGB888 )
    png_set_filler( png_ptr, 0,
                    QSysInfo::ByteOrder == QSysInfo::BigEndian ?
                    PNG_FILLER_BEFORE : PNG_FILLER_AFTER );

  if ( looping >= 0 && frames_written == 0 )
  {
    uchar data[13] = "NETSCAPE2.0";
    //                0123456789aBC
    data[0xB] = looping % 0x100;
    data[0xC] = looping / 0x100;
    png_write_chunk( png_ptr, ( png_byte* )"gIFx", data, 13 );
  }
  if ( ms_delay >= 0 || disposal != Unspecified )
  {
    uchar data[4];
    data[0] = disposal;
    data[1] = 0;
    data[2] = ( ms_delay / 10 ) / 0x100; // hundredths
    data[3] = ( ms_delay / 10 ) % 0x100;
    png_write_chunk( png_ptr, ( png_byte* )"gIFg", data, 4 );
  }

  int height = image.height();
  int width = image.width();
  switch ( image.format() )
  {
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
    case QImage::Format_Indexed8:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_RGB888:
    {
      png_bytep* row_pointers = new png_bytep[height];
      for ( int y = 0; y < height; y++ )
        row_pointers[y] = ( png_bytep )image.constScanLine( y );
      png_write_image( png_ptr, row_pointers );
      delete [] row_pointers;
    }
    break;
    default:
    {
      QImage::Format fmt = image.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32;
      QImage row;
      png_bytep row_pointers[1];
      for ( int y = 0; y < height; y++ )
      {
        row = image.copy( 0, y, width, 1 ).convertToFormat( fmt );
        row_pointers[0] = png_bytep( row.constScanLine( 0 ) );
        png_write_rows( png_ptr, row_pointers, 1 );
      }
    }
    break;
  }

  png_write_end( png_ptr, info_ptr );
  frames_written++;

  png_destroy_write_struct( &png_ptr, &info_ptr );

  return true;
}
