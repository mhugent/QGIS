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

typedef struct GEOSGeom_t GEOSGeometry;
class QgsCoordinateTransform;
class QgsCurveV2;
class QgsMultiCurveV2;
class QgsMultiPointV2;

/**Abstract base class for all geometries*/
class QgsAbstractGeometryV2
{
  public:
    QgsAbstractGeometryV2( int coordDimension, bool hasM );
    virtual ~QgsAbstractGeometryV2();
    virtual QgsAbstractGeometryV2* clone() const = 0;

    int referenceCount() const { return mRefs; }

    //mm-sql interface
    virtual int dimension() const = 0;
    virtual int coordDim() const{ return mCoordDimension; }
    virtual QString geometryType() const = 0;
    /*virtual bool transform( const QgsCoordinateTransform& ct ) =  0;
    virtual bool isEmpty() const = 0;
    virtual bool isSimple() const = 0;
    virtual bool isValid() const = 0;
    virtual bool is3D() const { return ( mCoordDimension > 2 ); }
    virtual bool isMeasure(){ return mHasM; }
    virtual QgsMultiPointV2* locateAlong() const = 0;
    virtual QgsMultiCurveV2* locateBetween() const = 0;
    virtual QgsCurveV2* boundary() const = 0;
    virtual QgsRectangle envelope() const = 0;
    virtual QgsAbstractGeometryV2* intersection( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* combine( const QgsAbstractGeometryV2& geom ) const = 0; //should be union, but this is a C++ keyword
    virtual QgsAbstractGeometryV2* difference( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual QgsAbstractGeometryV2* symDifference( const QgsAbstractGeometryV2& geom ) const = 0;*/
    virtual QString asText( int precision = 17 ) const = 0;
    virtual unsigned char* asBinary( int& binarySize ) const = 0;
    virtual QString asGML() const = 0;
    /*virtual bool intersects( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool touches( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool crosses( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool within( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool contains( const QgsAbstractGeometryV2& geom ) const = 0;
    virtual bool overlaps( const QgsAbstractGeometryV2& geom ) const = 0;*/

    //static import methods //moved to QgsGeometryImport
    /*static QgsAbstractGeometryV2* geomFromWkb( unsigned char * wkb, size_t length );
    static QgsAbstractGeometryV2* geomFromWkt( const QString& wkt );
    static QgsAbstractGeometryV2* geomFromGeos( const GEOSGeometry* geos );*/

    //virtual import methods
    virtual void fromWkb( const unsigned char * wkb, size_t length ) = 0;
    virtual void fromGeos( GEOSGeometry* geos ) = 0;
    virtual void fromWkt( const QString& wkt ) = 0;

  private:
    int mRefs;
    void ref(); // add reference
    void deref(); // remove reference, delete if refs == 0
    friend class QgsGeometry;

  protected:
    int mCoordDimension;
    bool mHasM;
};

#endif //QGSABSTRACTGEOMETRYV2
