/***************************************************************************
                         qgscircularstringv2.h
                         ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCIRCULARSTRING_H
#define QGSCIRCULARSTRING_H

#include "qgscurvev2.h"
#include <QVector>

class QgsCircularStringV2: public QgsCurveV2
{
  public:
    QgsCircularStringV2();
    ~QgsCircularStringV2();

    virtual QString geometryType() const { return "CircularString"; }
    virtual int dimension() const { return 1; }
    virtual QgsAbstractGeometryV2* clone() const;

    virtual void fromWkb( const unsigned char * wkb );
    virtual void fromGeos( GEOSGeometry* geos );
    virtual void fromWkt( const QString& wkt );

    virtual QString asText( int precision = 17 ) const;
    virtual unsigned char* asBinary( int& binarySize ) const;
    virtual int wkbSize() const;
    virtual QString asGML() const;

    int numPoints() const;
    QgsPointV2 pointN( int i ) const;
    QList<QgsPointV2> points() const;
    void setPoints( const QList<QgsPointV2> points );


    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual bool isClosed() const;
    virtual bool isRing() const;
    virtual QgsLineStringV2* curveToLine() const;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    //helper methods for curveToLine
    void segmentize( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& p3, QList<QgsPointV2>& points ) const;
    void circleCenterRadius( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3, double& radius,
                             double& centerX, double& centerY ) const;
    int segmentSide( const QgsPointV2& pt1, const QgsPointV2& pt3, const QgsPointV2& pt2 ) const;
    double interpolateArc( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 ) const;
};

#endif // QGSCIRCULARSTRING_H
