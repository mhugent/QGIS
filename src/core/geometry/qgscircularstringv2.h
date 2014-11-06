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
    virtual void fromWkt( const QString& wkt );

    virtual QString asText( int precision = 17 ) const;
    virtual unsigned char* asBinary( int& binarySize ) const;
    virtual int wkbSize() const;
    virtual QString asGML() const;

    int numPoints() const;
    QgsPointV2 pointN( int i ) const;
    void points( QList<QgsPointV2>& pts ) const;
    void setPoints( const QList<QgsPointV2> points );


    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual bool isClosed() const;
    virtual bool isRing() const;
    virtual QgsLineStringV2* curveToLine() const;

    virtual QgsRectangle calculateBoundingBox() const { return QgsRectangle(); }

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct );
    void mapToPixel( const QgsMapToPixel& mtp );
    void clip( const QgsRectangle& rect );
    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    //helper methods for curveToLine
    void segmentize( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& p3, QList<QgsPointV2>& points ) const;
    static void circleCenterRadius( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3, double& radius,
                                    double& centerX, double& centerY );
    int segmentSide( const QgsPointV2& pt1, const QgsPointV2& pt3, const QgsPointV2& pt2 ) const;
    double interpolateArc( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 ) const;
    static void arcTo( QPainterPath& path, const QPointF& pt1, const QPointF& pt2, const QPointF& pt3 );
    static double ccwAngle( double dy, double dx );
};

#endif // QGSCIRCULARSTRING_H
