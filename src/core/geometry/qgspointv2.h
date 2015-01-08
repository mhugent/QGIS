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
    void translate( double dx, double dy, double dz = 0.0, double dm = 0.0 );

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) { return false; }
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) { *this = newPos; return true; }
    virtual bool deleteVertex( const QgsVertexId& position ) { return false; }

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;

  private:
    double mX;
    double mY;
    double mZ;
    double mM;

    //sets wkbtype to unknown and coords to 0
    void reset();
};

#endif // QGSPOINTV2_H
