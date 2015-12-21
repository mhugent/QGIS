/***************************************************************************
     testqgsimagefillsymbollayer.cpp
     --------------------------------------
    Date                 : Dec 18  2015
    Copyright            : (C) 2015 by Marco Hugentobler
    Email                : marco at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>

#include "qgsapplication.h"
#include "qgsmaplayerregistry.h"
#include <qgsrenderchecker.h>
#include "qgsvectorlayer.h"
#include "qgsvectorlayerrenderer.h"

/** \ingroup UnitTests
 * This is a unit test for the Marker Line symbol
 */
class TestQgsImageFillSymbolLayer : public QObject
{
    Q_OBJECT
  public:
    TestQgsImageFillSymbolLayer() {}
    ~TestQgsImageFillSymbolLayer() {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testFourAdjacentTiles_data();
    void testFourAdjacentTiles();

  private:
    static bool compareImages( const QImage& image1, const QImage& image2 );
};

void TestQgsImageFillSymbolLayer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsImageFillSymbolLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsImageFillSymbolLayer::testFourAdjacentTiles_data()
{
  QTest::addColumn<QStringList>( "bboxList" );
  QTest::addColumn<QString>( "resultFile" );
  QTest::addColumn<QString>( "shapeFile" );
  QTest::addColumn<QString>( "qmlFile" );

  QString shapeFile = TEST_DATA_DIR + QString( "/france_parts.shp" );
  QString qmlFile = TEST_DATA_DIR + QString( "/image_fill_symbollayer/line_pattern_30_degree.qml" );
  QString resultFile = TEST_DATA_DIR + QString( "/image_fill_symbollayer/testFourAdjacentTiles1_expected.png" );

  QStringList bboxList1;
  bboxList1 << "-1.5,48,-0.5,49";
  bboxList1 << "-0.5,48,0.5,49";
  bboxList1 << "-1.5,47,-0.5,48";
  bboxList1 << "-0.5,47,0.5,48";

  QTest::newRow( "testFourAdjacentTiles1" ) << bboxList1 << resultFile << shapeFile << qmlFile;
}

void TestQgsImageFillSymbolLayer::testFourAdjacentTiles()
{
  QFETCH( QStringList, bboxList );
  QFETCH( QString, resultFile );
  QFETCH( QString, shapeFile );
  QFETCH( QString, qmlFile );

  QVERIFY( bboxList.size() == 4 );

  //create maplayer, set QML and add to maplayer registry
  QgsVectorLayer* vectorLayer = new QgsVectorLayer( shapeFile, "testshape", "ogr" );

  //todo: read QML
  QFile symbologyFile( qmlFile );
  if ( !symbologyFile.open( QIODevice::ReadOnly ) )
  {
    QFAIL( "Open symbology file failed" );
  }

  QDomDocument qmlDoc;
  if ( !qmlDoc.setContent( &symbologyFile ) )
  {
    QFAIL( "QML file not valid" );
  }

  QString errorMsg;
  if ( !vectorLayer->readSymbology( qmlDoc.documentElement(), errorMsg ) )
  {
    QFAIL( errorMsg.toLocal8Bit().data() );
  }

  QImage globalImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  globalImage.fill( Qt::white );
  QPainter globalPainter( &globalImage );

  for ( int i = 0; i < 4; ++i )
  {
    QImage img( 256, 256, QImage::Format_ARGB32_Premultiplied );
    img.fill( Qt::white );
    QPainter p( &img );

    QgsRenderContext renderContext;
    renderContext.setPainter( &p );

    //extent
    QStringList rectCoords = bboxList.at( i ).split( "," );
    if ( rectCoords.size() != 4 )
    {
      QFAIL( "bbox string invalid" );
    }
    QgsRectangle rect( rectCoords[0].toDouble(), rectCoords[1].toDouble(), rectCoords[2].toDouble(), rectCoords[3].toDouble() );
    renderContext.setExtent( rect );

    //maptopixel
    QgsMapToPixel mapToPixel( rect.width() / 256.0, rect.center().x(), rect.center().y(), 256, 256, 0 );
    renderContext.setMapToPixel( mapToPixel );

    QgsVectorLayerRenderer renderer( vectorLayer, renderContext );
    if ( !renderer.render() )
    {
      QFAIL( "Rendering failed" );
    }

    int globalImageX = ( i % 2 ) * 256;
    int globalImageY = ( i < 2 ) ? 0 : 256;
    globalPainter.drawImage( globalImageX, globalImageY, img );
  }

  delete vectorLayer;

  QString renderedImagePath = QDir::tempPath() + "/" + QTest::currentDataTag() + QString( ".png" );
  globalImage.save( renderedImagePath );

  QgsRenderChecker checker;
  checker.setControlName( QTest::currentDataTag() );
  QVERIFY( checker.compareImages( QTest::currentDataTag(), 0, renderedImagePath ) );
}

bool TestQgsImageFillSymbolLayer::compareImages( const QImage& image1, const QImage& image2 )
{
  if ( image1.width() != image2.width() || image1.height() != image2.height() )
  {
    return false;
  }

  int imageWidth = image1.width();
  int imageHeight = image1.height();

  for ( int i = 0; i < imageHeight; ++i )
  {
    for ( int j = 0; j < imageWidth; ++j )
    {
      if ( image1.pixel( j, i ) != image2.pixel( j, i ) )
      {
        return false;
      }
    }
  }

  return true;
}

QTEST_MAIN( TestQgsImageFillSymbolLayer )
#include "testqgsimagefillsymbollayer.moc"
