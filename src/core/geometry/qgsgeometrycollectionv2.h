/***************************************************************************
                        qgsgeometrycollectionv2.h
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCOLLECTIONV2_H
#define QGSGEOMETRYCOLLECTIONV2_H

#include "qgsabstractgeometryv2.h"
#include <QVector>

class QgsGeometryCollectionV2: public QgsAbstractGeometryV2
{
  public:
    QgsGeometryCollectionV2();
    QgsGeometryCollectionV2( const QgsGeometryCollectionV2& c );
    QgsGeometryCollectionV2& operator=( const QgsGeometryCollectionV2& c );
    virtual ~QgsGeometryCollectionV2();

    int numGeometries() const;
    const QgsAbstractGeometryV2* geometryN( int n ) const;
    QgsAbstractGeometryV2* geometryN( int n );

    //methods inherited from QgsAbstractGeometry
    virtual int dimension() const;
    virtual QString geometryType() const { return "GeometryCollection"; }

    /**Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometryV2* g );

    virtual void transform( const QgsCoordinateTransform& ct );
    virtual void mapToPixel( const QgsMapToPixel& mtp );
    virtual void translate( double dx, double dy, double dz = 0.0, double dm = 0.0 );
    virtual void clip( const QgsRectangle& rect );
    virtual void draw( QPainter& p ) const;

    void fromWkb( const unsigned char * wkb );
    unsigned char* asBinary( int& binarySize ) const;
    int wkbSize() const;

    virtual QgsRectangle calculateBoundingBox() const;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const;
    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

  protected:
    QVector< QgsAbstractGeometryV2* > mGeometries;
    void removeGeometries();
};

#endif // QGSGEOMETRYCOLLECTIONV2_H
