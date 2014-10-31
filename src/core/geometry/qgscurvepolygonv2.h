/***************************************************************************
                         qgscurvepolygonv2.h
                         -------------------
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

#ifndef QGSCURVEPOLYGONV2_H
#define QGSCURVEPOLYGONV2_H

#include "qgssurfacev2.h"

class QgsPolygonV2;

class QgsCurvePolygonV2: public QgsSurfaceV2
{
  public:
    QgsCurvePolygonV2();
    QgsCurvePolygonV2( const QgsCurvePolygonV2& p );
    QgsCurvePolygonV2& operator=( const QgsCurvePolygonV2& p );
    ~QgsCurvePolygonV2();

    virtual QString geometryType() const { return "CurvePolygon"; }
    virtual int dimension() const { return 2; }
    virtual QgsAbstractGeometryV2* clone() const;

    virtual void fromWkb( const unsigned char* wkb );
    virtual void fromWkt( const QString& wkt );

    virtual QString asText( int precision = 17 ) const;
    virtual unsigned char* asBinary( int& binarySize ) const;
    virtual int wkbSize() const;
    virtual QString asGML() const;

    //surface interface
    virtual double area() const;
    virtual double perimeter() const;
    QgsPointV2 centroid() const;
    QgsPointV2 pointOnSurface() const;

    //curve polygon interface
    int numInteriorRings() const;
    const QgsCurveV2* exteriorRing() const;
    const QgsCurveV2* interiorRing( int i ) const;
    virtual QgsPolygonV2* toPolygon() const;

    void setExteriorRing( QgsCurveV2* ring );
    void setInteriorRings( QList<QgsCurveV2*> rings );

    virtual QgsRectangle calculateBoundingBox() const;

    virtual void draw( QPainter& p ) const;
    void mapToPixel( const QgsMapToPixel& mtp );
    void transform( const QgsCoordinateTransform& ct );

  protected:

    QgsCurveV2* mExteriorRing;
    QList<QgsCurveV2*> mInteriorRings;

    void removeRings();
    void addRingWkb( unsigned char** wkb, const QgsCurveV2* ring ) const;
};

#endif // QGSCURVEPOLYGONV2_H
