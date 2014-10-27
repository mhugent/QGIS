/***************************************************************************
                        qgsabstractgeometryv2.h
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
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

#ifndef QGSABSTRACTGEOMETRYV2
#define QGSABSTRACTGEOMETRYV2

#include "qgsrectangle.h"

#include <QString>

class QgsCoordinateTransform;
class QgsMapToPixel;
class QgsCurveV2;
class QgsMultiCurveV2;
class QgsMultiPointV2;
class QgsVectorTopology;
class QPainter;

/**Abstract base class for all geometries*/
class QgsAbstractGeometryV2
{
  public:
    QgsAbstractGeometryV2();
    virtual ~QgsAbstractGeometryV2();
    QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom );
    QgsAbstractGeometryV2& operator=( const QgsAbstractGeometryV2& geom );

    virtual QgsAbstractGeometryV2* clone() const = 0;

    QgsRectangle boundingBox() const;

    //mm-sql interface
    virtual int dimension() const = 0;
    //virtual int coordDim() const { return mCoordDimension; }
    virtual QString geometryType() const = 0;
    QGis::WkbType wkbType() const { return mWkbType; }
    bool is3D() const;
    bool isMeasure() const;

    /*virtual bool transform( const QgsCoordinateTransform& ct ) =  0;
    virtual bool isEmpty() const = 0;
    virtual bool isSimple() const = 0;
    virtual bool isValid() const = 0;
    virtual QgsMultiPointV2* locateAlong() const = 0;
    virtual QgsMultiCurveV2* locateBetween() const = 0;
    virtual QgsCurveV2* boundary() const = 0;
    virtual QgsRectangle envelope() const = 0;*/

    virtual QString asText( int precision = 17 ) const = 0;
    virtual unsigned char* asBinary( int& binarySize ) const = 0;
    virtual int wkbSize() const = 0;
    virtual QString asGML() const = 0;

    //virtual import methods
    virtual void fromWkb( const unsigned char * wkb ) = 0;
    virtual void fromWkt( const QString& wkt ) = 0;

    //redirect to vector topology
    QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const;
    QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const; //should be union, but this is a C++ keyword
    QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const;
    QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const;
    bool intersects( const QgsAbstractGeometryV2& geom ) const;
    bool touches( const QgsAbstractGeometryV2& geom ) const;
    bool crosses( const QgsAbstractGeometryV2& geom ) const;
    bool within( const QgsAbstractGeometryV2& geom ) const;
    bool contains( const QgsAbstractGeometryV2& geom ) const;
    bool overlaps( const QgsAbstractGeometryV2& geom ) const;

    virtual QgsRectangle calculateBoundingBox() const = 0;

    //render pipeline
    virtual void transform( const QgsCoordinateTransform& ct ) = 0;
    virtual void mapToPixel( const QgsMapToPixel& mtp ) {}
    virtual void clip( const QgsRectangle& rect ) {}
    virtual void draw( QPainter& p ) const = 0;

  protected:
    QGis::WkbType mWkbType;
    QgsVectorTopology* mVectorTopology;
    mutable QgsRectangle mBoundingBox;

    void geometryChanged();
};

#endif //QGSABSTRACTGEOMETRYV2
