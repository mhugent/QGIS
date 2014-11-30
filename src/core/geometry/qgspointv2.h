/***************************************************************************
                         qgspointv2.h
                         --------------
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

#ifndef QGSPOINTV2_H
#define QGSPOINTV2_H

#include "qgsabstractgeometryv2.h"

class QgsPointV2: public QgsAbstractGeometryV2
{
  public:
    QgsPointV2( double x = 0.0, double y = 0.0 );
    QgsPointV2( double x, double y, double mz, bool hasM );
    QgsPointV2( double x, double y, double z, double m );
    ~QgsPointV2();

    bool operator==( const QgsPointV2& pt ) const;

    virtual QgsAbstractGeometryV2* clone() const;

    double x() const { return mX; }
    double y() const { return mY; }
    double z() const { return mZ; }
    double m() const { return mM; }

    void setX( double x ) { mX = x; }
    void setY( double y ) { mY = y; }
    void setZ( double z ) { mZ = z; }
    void setM( double m ) { mM = m; }

    virtual QString geometryType() const { return "Point"; }

    //implementation of inherited methods
    virtual int dimension() const { return 0; }
    /*virtual QString geometryType() const = 0;
    virtual bool transform( const QgsCoordinateTransform& ct ) =  0;
    virtual bool isEmpty() const = 0;
    virtual bool isSimple() const = 0;
    virtual bool isValid() const = 0;
    virtual QgsMultiPointV2* locateAlong() const = 0;
    virtual QgsMultiCurveV2* locateBetween() const = 0;
    virtual QgsCurveV2* boundary() const = 0;
    virtual QgsRectangle envelope() const = 0;
    virtual QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const = 0; //should be union, but this is a C++ keyword
    virtual QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QString asText() const = 0;
    virtual unsigned char* asBinary( int& binarySize ) const = 0;
    virtual QString asGML() const = 0;
    virtual bool intersects( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool touches( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool crosses( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool within( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool contains( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool overlaps( const QgsAbstractGeometryV2& geom ) const = 0;*/

    virtual void fromWkb( const unsigned char* wkb );
    virtual void fromWkt( const QString& wkt );


    virtual QString asText( int precision = 17 ) const;
    virtual unsigned char* asBinary( int& binarySize ) const;
    virtual int wkbSize() const;
    virtual QString asGML() const;

    virtual QgsRectangle calculateBoundingBox() const { return QgsRectangle( mX, mY, mX, mY );}

    void draw( QPainter& p ) const {}
    void transform( const QgsCoordinateTransform& ct );
    void mapToPixel( const QgsMapToPixel& mtp );

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) { return false; }
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) { *this = newPos; return true; }
    virtual bool deleteVertex( const QgsVertexId& position ) { return false; }

  private:
    double mX;
    double mY;
    double mZ;
    double mM;

    //sets wkbtype to unknown and coords to 0
    void reset();
};

#endif // QGSPOINTV2_H
