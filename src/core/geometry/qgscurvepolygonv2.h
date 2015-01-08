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

    virtual QgsRectangle calculateBoundingBox() const;

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

    /**Sets exterior ring (takes ownership)*/
    void setExteriorRing( QgsCurveV2* ring );
    /**Sets interior rings (takes ownership)*/
    void setInteriorRings( QList<QgsCurveV2*> rings );
    void addInteriorRing( QgsCurveV2* ring );

    virtual void draw( QPainter& p ) const;
    void mapToPixel( const QgsMapToPixel& mtp );
    void transform( const QgsCoordinateTransform& ct );
    void translate( double dx, double dy, double dz = 0.0, double dm = 0.0 );

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const;
    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;

  protected:

    QgsCurveV2* mExteriorRing;
    QList<QgsCurveV2*> mInteriorRings;

    void removeRings();
    void addRingWkb( unsigned char** wkb, const QgsCurveV2* ring ) const;

    /**Returns two vertices in case of start/endpoint*/
    QList< QgsVertexId > ringVertexIds( const QgsVertexId& id ) const;
};

#endif // QGSCURVEPOLYGONV2_H
