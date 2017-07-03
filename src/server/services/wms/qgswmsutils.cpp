/***************************************************************************
                              qgswmsutils.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts fron qgswmshandler)
                         (C) 2014 by Alessandro Pasotti ( parts from qgswmshandler)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                  *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodule.h"
#include "qgswmsutils.h"
#include "qgsmediancut.h"
#include "qgsconfigcache.h"
#include "qgsserverprojectutils.h"
//for geotiff export
#include "cpl_string.h"

namespace QgsWms
{
  QString ImplementationVersion()
  {
    return QStringLiteral( "1.3.0" );
  }

  // Return the wms config parser (Transitional)
  QgsWmsConfigParser *getConfigParser( QgsServerInterface *serverIface )
  {
    QString configFilePath = serverIface->configFilePath();

    QgsWmsConfigParser *parser  = QgsConfigCache::instance()->wmsConfiguration( configFilePath, serverIface->accessControls() );
    if ( !parser )
    {
      throw QgsServiceException(
        QStringLiteral( "WMS configuration error" ),
        QStringLiteral( "There was an error reading the project file or the SLD configuration" ) );
    }
    return parser;
  }

  QUrl serviceUrl( const QgsServerRequest &request, const QgsProject *project )
  {
    QUrl href;
    if ( project )
    {
      href.setUrl( QgsServerProjectUtils::wmsServiceUrl( *project ) );
    }

    // Build default url
    if ( href.isEmpty() )
    {
      href = request.url();
      QUrlQuery q( href );

      q.removeAllQueryItems( QStringLiteral( "REQUEST" ) );
      q.removeAllQueryItems( QStringLiteral( "VERSION" ) );
      q.removeAllQueryItems( QStringLiteral( "SERVICE" ) );
      q.removeAllQueryItems( QStringLiteral( "LAYERS" ) );
      q.removeAllQueryItems( QStringLiteral( "SLD_VERSION" ) );
      q.removeAllQueryItems( QStringLiteral( "_DC" ) );

      href.setQuery( q );
    }

    return  href;
  }


  ImageOutputFormat parseImageFormat( const QString &format )
  {
    if ( format.compare( QLatin1String( "png" ), Qt::CaseInsensitive ) == 0 ||
         format.compare( QLatin1String( "image/png" ), Qt::CaseInsensitive ) == 0 )
    {
      return PNG;
    }
    else if ( format.compare( QLatin1String( "jpg " ), Qt::CaseInsensitive ) == 0  ||
              format.compare( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) == 0 )
    {
      return JPEG;
    }
    else if ( format.compare( QLatin1String( "geotiff" ), Qt::CaseInsensitive ) == 0 )
    {
      return GEOTIFF;
    }
    else
    {
      // lookup for png with mode
      QRegularExpression modeExpr = QRegularExpression( QStringLiteral( "image/png\\s*;\\s*mode=([^;]+)" ),
                                    QRegularExpression::CaseInsensitiveOption );

      QRegularExpressionMatch match = modeExpr.match( format );
      QString mode = match.captured();
      if ( mode.compare( QLatin1String( "16bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG16;
      if ( mode.compare( QLatin1String( "8bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG8;
      if ( mode.compare( QLatin1String( "1bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG1;
    }

    return UNKN;
  }

  void readLayersAndStyles( const QgsServerRequest::Parameters &parameters, QStringList &layersList, QStringList &stylesList )
  {
    //get layer and style lists from the parameters trying LAYERS and LAYER as well as STYLE and STYLES for GetLegendGraphic compatibility
    layersList = parameters.value( QStringLiteral( "LAYER" ) ).split( ',', QString::SkipEmptyParts );
    layersList = layersList + parameters.value( QStringLiteral( "LAYERS" ) ).split( ',', QString::SkipEmptyParts );
    stylesList = parameters.value( QStringLiteral( "STYLE" ) ).split( ',', QString::SkipEmptyParts );
    stylesList = stylesList + parameters.value( QStringLiteral( "STYLES" ) ).split( ',', QString::SkipEmptyParts );
  }


  // Write image response
  void writeImage( QgsServerResponse &response, QImage &img, const QString &formatStr,
                   int imageQuality, const QgsRectangle *mapExtent, const QgsCoordinateReferenceSystem *mapCrs )
  {
    ImageOutputFormat outputFormat = parseImageFormat( formatStr );
    QImage  result;
    QString saveFormat;
    QString contentType;
    switch ( outputFormat )
    {
      case PNG:
        result = img;
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case PNG8:
      {
        QVector<QRgb> colorTable;
        medianCut( colorTable, 256, img );
        result = img.convertToFormat( QImage::Format_Indexed8, colorTable,
                                      Qt::ColorOnly | Qt::ThresholdDither |
                                      Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
      }
      contentType = "image/png";
      saveFormat = "PNG";
      break;
      case PNG16:
        result = img.convertToFormat( QImage::Format_ARGB4444_Premultiplied );
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case PNG1:
        result = img.convertToFormat( QImage::Format_Mono,
                                      Qt::MonoOnly | Qt::ThresholdDither |
                                      Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case JPEG:
        result = img;
        contentType = "image/jpeg";
        saveFormat = "JPEG";
        break;
      case GEOTIFF:
        //extent
        //crs
        //call writeGeoTiff -> writeGeorefInfo u. writeImageToGDALDataSource
        writeGeoTiff( img, response, mapExtent, mapCrs );
        break;
      default:
        QgsMessageLog::logMessage( QString( "Unsupported format string %1" ).arg( formatStr ) );
        saveFormat = UNKN;
        break;
    }

    if ( outputFormat == UNKN )
    {
      throw QgsServiceException( "InvalidFormat",
                                 QString( "Output format '%1' is not supported in the GetMap request" ).arg( formatStr ) );
    }
    else
    {
      response.setHeader( "Content-Type", contentType );
      result.save( response.io(), qPrintable( saveFormat ), imageQuality );
    }
  }

  QgsRectangle parseBbox( const QString &bboxStr )
  {
    QStringList lst = bboxStr.split( ',' );
    if ( lst.count() != 4 )
      return QgsRectangle();

    double d[4];
    bool ok;
    for ( int i = 0; i < 4; i++ )
    {
      lst[i].replace( ' ', '+' );
      d[i] = lst[i].toDouble( &ok );
      if ( !ok )
        return QgsRectangle();
    }
    return QgsRectangle( d[0], d[1], d[2], d[3] );
  }

  void writeGeoTiff( const QImage &img, QgsServerResponse &response, const QgsRectangle *mapExtent, const QgsCoordinateReferenceSystem *mapCrs )
  {
    if ( !mapExtent || !mapCrs )
    {
      return; //throw exception?
    }

    //save image into vsi file with GDAL
    GDALDriverH geoTiffDriver = GDALGetDriverByName( "GTiff" );
    GDALDatasetH geoTiffDS;
    char **options = NULL;
    options = CSLSetNameValue( options, "COMPRESS", "LZW" );
    geoTiffDS = GDALCreate( geoTiffDriver, "/vsimem/wms.tif", img.width(), img.height(), 4, GDT_Byte, options );

    writeGeorefInfo( geoTiffDS, *mapExtent, *mapCrs, img.width(), img.height() );
    writeImageToGDALDataSource( geoTiffDS, img );

    CSLDestroy( options );
    GDALClose( geoTiffDS );

    //get content of vsi file into QBuffer ba
    long long unsigned int dataLength;
    GByte *data = VSIGetMemFileBuffer( "/vsimem/wms.tif", &dataLength, TRUE );

    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "image/geotiff" ) );
    response.io()->write( reinterpret_cast< const char *>( data ), dataLength );
  }

  void writeGeorefInfo( GDALDatasetH ds, const QgsRectangle &extent, const QgsCoordinateReferenceSystem &crs, int pixelWidth, int pixelHeight )
  {
    if ( extent.isNull() )
    {
      return;
    }

    double geoTransform[6] = { extent.xMinimum(), extent.width() / ( double )pixelWidth, 0, extent.yMaximum(), 0, -( extent.height() / ( double )pixelHeight ) };
    GDALSetGeoTransform( ds, geoTransform );

    if ( crs.isValid() )
    {
      GDALSetProjection( ds, crs.toWkt().toLocal8Bit().data() );
    }
  }

  void writeImageToGDALDataSource( GDALDatasetH ds, const QImage &img )
  {
    GDALRasterBandH redBand = GDALGetRasterBand( ds, 1 );
    GDALRasterBandH greenBand = GDALGetRasterBand( ds, 2 );
    GDALRasterBandH blueBand = GDALGetRasterBand( ds, 3 );
    GDALRasterBandH alphaBand = GDALGetRasterBand( ds, 4 );

    //separate image into four channels
    int nPixels = img.width() * img.height();
    void *redData = malloc( nPixels );
    void *greenData = malloc( nPixels );
    void *blueData = malloc( nPixels );
    void *alphaData = malloc( nPixels );

    int red = 0;
    int green = 0;
    int blue = 0;
    int alpha = 255;

    int  currentPixel = 0;
    for ( int i = 0; i < img.height(); ++i )
    {
      for ( int j = 0; j < img.width(); ++j )
      {
        QRgb rgb = img.pixel( j, i );
        red = qRed( rgb ); green = qGreen( rgb ); blue = qBlue( rgb ); alpha = qAlpha( rgb );
        if ( img.format() == QImage::Format_ARGB32_Premultiplied )
        {
          double a = alpha / 255.0;
          red /= a; green /= a; blue /= a;
        }
        memcpy( ( char * )redData + currentPixel, &red, 1 );
        memcpy( ( char * )greenData + currentPixel, &green, 1 );
        memcpy( ( char * )blueData + currentPixel, &blue, 1 );
        memcpy( ( char * )alphaData + currentPixel, &alpha, 1 );
        ++currentPixel;
      }
    }

    //do rasterIO to get image content into geoTiffDS
    GDALRasterIO( redBand, GF_Write, 0, 0, img.width(), img.height(), redData, img.width(), img.height(), GDT_Byte, 0, 0 );
    GDALRasterIO( greenBand, GF_Write, 0, 0, img.width(), img.height(), greenData, img.width(), img.height(), GDT_Byte, 0, 0 );
    GDALRasterIO( blueBand, GF_Write, 0, 0, img.width(), img.height(), blueData, img.width(), img.height(), GDT_Byte, 0, 0 );
    GDALRasterIO( alphaBand, GF_Write, 0, 0, img.width(), img.height(), alphaData, img.width(), img.height(), GDT_Byte, 0, 0 );
    free( redData ); free( greenData ); free( blueData ); free( alphaData );
  }

} // namespace QgsWms


