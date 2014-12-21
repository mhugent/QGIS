/***************************************************************************
     testqgscircularstringv2.cpp
     --------------------------------------
    Date                 : 21 December 2014
    Copyright            : (C) 2014 by Marco Hugentobler
    Email                : marco@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscircularstringv2.h"

#include <QtTest>
#include <QObject>

class TestQgsCircularStringV2: public QObject
{
    Q_OBJECT
  private slots:
    void testBoundingBox_data();
    void testBoundingBox();
};

void TestQgsCircularStringV2::testBoundingBox_data()
{
  QTest::addColumn<QList<QVariant> >( "pointList" );
  QTest::addColumn<double>( "resultXMin" );
  QTest::addColumn<double>( "resultYMin" );
  QTest::addColumn<double>( "resultXMax" );
  QTest::addColumn<double>( "resultYMax" );

  QList<QVariant> pointList;
  pointList << QPointF( 0.5, 0.0 ) << QPointF( 0.0, 0.5 ) << QPointF( 1.0, 0.5 );
  QTest::newRow( "bboxTest1" ) << pointList << 0.0 << 0.0 << 1.0 << 1.0;
  pointList.clear();
  pointList << QPointF( 1.5, 0 ) << QPointF( 2.0, 0.5 ) << QPointF( 1.0, 0.5 ) << QPointF( 0.5, 0.0 ) << QPointF( 0.5, 1.0 );
  QTest::newRow( "bboxTest2" ) << pointList << 0.0 << 0.0 << 2.0 << 1.0;
}

void TestQgsCircularStringV2::testBoundingBox()
{
  QFETCH( QList<QVariant>, pointList );
  QFETCH( double, resultXMin );
  QFETCH( double, resultYMin );
  QFETCH( double, resultXMax );
  QFETCH( double, resultYMax );

  //convert points
  QList<QgsPointV2> points;
  QList<QVariant>::const_iterator it = pointList.constBegin();
  for ( ; it != pointList.constEnd(); ++it )
  {
    QPointF pt = it->toPointF();
    points << QgsPointV2( pt.x(), pt.y() );
  }

  QgsCircularStringV2 cs;
  cs.setPoints( points );
  QgsRectangle bbox = cs.calculateBoundingBox();
  QVERIFY( qgsDoubleNear( bbox.xMinimum(), resultXMin ) );
  QVERIFY( qgsDoubleNear( bbox.yMinimum(), resultYMin ) );
  QVERIFY( qgsDoubleNear( bbox.xMaximum(), resultXMax ) );
  QVERIFY( qgsDoubleNear( bbox.yMaximum(), resultYMax ) );
}

QTEST_MAIN( TestQgsCircularStringV2 )
#include "moc_testqgscircularstringv2.cxx"
