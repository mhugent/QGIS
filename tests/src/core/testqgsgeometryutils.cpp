/***************************************************************************
     testqgsgeometryutils.cpp
     --------------------------------------
    Date                 : 16 Jan 2015
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

#include <QtTest>
#include <QObject>
#include "qgsgeometryutils.h"

class TestQgsGeometryUtils: public QObject
{
    Q_OBJECT
  private slots:
    void testCircleClockwise_data();
    void testCircleClockwise();
    void testAngleOnCircle_data();
    void testAngleOnCircle();
};

void TestQgsGeometryUtils::testCircleClockwise_data()
{
  QTest::addColumn<double>( "angle1" );
  QTest::addColumn<double>( "angle2" );
  QTest::addColumn<double>( "angle3" );
  QTest::addColumn<bool>( "expectedResult" );

  QTest::newRow( "circleClockwise1" ) << 10.0 << 300.0 << 270.0 << true;
  QTest::newRow( "circleClockwise2" ) << 270.0 << 300.0 << 10.0 << false;
  QTest::newRow( "circleClockwise3" ) << 260.0 << 245.0 << 243.0 << true;
}

void TestQgsGeometryUtils::testCircleClockwise()
{
  QFETCH( double, angle1 );
  QFETCH( double, angle2 );
  QFETCH( double, angle3 );
  QFETCH( bool, expectedResult );

  QCOMPARE( QgsGeometryUtils::circleClockwise( angle1, angle2, angle3 ), expectedResult );
}

void TestQgsGeometryUtils::testAngleOnCircle_data()
{
  QTest::addColumn<double>( "angle" );
  QTest::addColumn<double>( "angle1" );
  QTest::addColumn<double>( "angle2" );
  QTest::addColumn<double>( "angle3" );
  QTest::addColumn<bool>( "expectedResult" );

  QTest::newRow( "angleOnCircle1" ) << 280.0 << 10.0 << 300.0 << 270.0 << true;
  QTest::newRow( "angleOnCircle2" ) << 235.0 << 10.0 << 300.0 << 270.0 << false;
  QTest::newRow( "angleOnCircle3" ) << 280.0 << 45.0 << 175.0 << 35.0 << true;
  QTest::newRow( "angleOnCircle4" ) << 40.0 << 45.0 << 175.0 << 35.0 << false;
  QTest::newRow( "angleOnCircle5" ) << 335.0 << 350.0 << 175.0 << 320.0 << false;
  QTest::newRow( "angleOnCircle6" ) << 170.0 << 140.0 << 355.0 << 205.0 << false;
}

void TestQgsGeometryUtils::testAngleOnCircle()
{
  QFETCH( double, angle );
  QFETCH( double, angle1 );
  QFETCH( double, angle2 );
  QFETCH( double, angle3 );
  QFETCH( bool, expectedResult );

  QCOMPARE( QgsGeometryUtils::angleOnCircle( angle, angle1, angle2, angle3 ), expectedResult );
}

QTEST_MAIN( TestQgsGeometryUtils )
#include "moc_testqgsgeometryutils.cxx"
