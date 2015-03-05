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
#include "qgswkbtypes.h"
#include <QString>

class QgsCoordinateTransform;
class QgsMapToPixel;
class QgsCurveV2;
class QgsMultiCurveV2;
class QgsMultiPointV2;
class QgsPointV2;
class QgsConstWkbPtr;
class QgsWkbPtr;
class QPainter;

struct QgsVertexId
{
  QgsVertexId(): part( - 1 ), ring( -1 ), vertex( -1 ) {}
  bool isValid() const { return part >= 0 && ring >= 0 && vertex >= 0; }
  int part;
  int ring;
  int vertex;
};

/**Abstract base class for all geometries*/
class QgsAbstractGeometryV2
{
  public:
    QgsAbstractGeometryV2();
    virtual ~QgsAbstractGeometryV2();
    QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom );
    virtual QgsAbstractGeometryV2& operator=( const QgsAbstractGeometryV2& geom );

    virtual QgsAbstractGeometryV2* clone() const = 0;
    virtual void clear() = 0;

    QgsRectangle boundingBox() const;

    //mm-sql interface
    virtual int dimension() const = 0;
    //virtual int coordDim() const { return mCoordDimension; }
    virtual QString geometryType() const = 0;
    QgsWKBTypes::Type wkbType() const { return mWkbType; }
    QString wktTypeStr() const;
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

    //import
    virtual bool fromWkb( const unsigned char * wkb ) = 0;
    virtual bool fromWkt( const QString& wkt ) = 0;

    //export
    virtual int wkbSize() const = 0;
    virtual unsigned char* asWkb( int& binarySize ) const = 0;
    virtual QString asWkt( int precision = 17 ) const = 0;
    virtual QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const = 0;
    virtual QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const = 0;
    virtual QString asJSON( int precision = 17 ) const = 0;

    virtual QgsRectangle calculateBoundingBox() const;

    //render pipeline
    virtual void transform( const QgsCoordinateTransform& ct ) = 0;
    virtual void transform( const QTransform& t ) = 0;
    virtual void clip( const QgsRectangle& rect ) {}
    virtual void draw( QPainter& p ) const = 0;

    /**Returns next vertex id and coordinates
    @return false if at end*/
    virtual bool nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const = 0;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const = 0;
    int nCoordinates() const;
    QgsPointV2 vertexAt( const QgsVertexId& id ) const;
    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const = 0;

    //low-level editing
    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) = 0;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) = 0;
    virtual bool deleteVertex( const QgsVertexId& position ) = 0;


  protected:
    QgsWKBTypes::Type mWkbType;
    mutable QgsRectangle mBoundingBox;

    void setZMTypeFromSubGeometry( const QgsAbstractGeometryV2* subggeom, QgsWKBTypes::Type baseGeomType );

    /** Serialization and deserialization of a point list */
    static QList<QgsPointV2> pointsFromWKT( const QString& wktCoordinateList, bool is3D, bool isMeasure );
    // Returns a LinearRing { uint32 numPoints; Point points[numPoints]; }
    static void pointsToWKB( QgsWkbPtr &wkb, const QList<QgsPointV2>& points, bool is3D, bool isMeasure );
    // Returns a WKT coordinate list
    static QString pointsToWKT( const QList<QgsPointV2>& points, int precision, bool is3D, bool isMeasure );
    // Returns a gml::coordinates DOM element
    static QDomElement pointsToGML2( const QList<QgsPointV2>& points, QDomDocument &doc, int precision, const QString& ns );
    // Returns a gml::posList DOM element
    static QDomElement pointsToGML3( const QList<QgsPointV2>& points, QDomDocument &doc, int precision, const QString& ns, bool is3D );
    // Returns a geoJSON coordinates string
    static QString pointsToJSON( const QList<QgsPointV2>& points, int precision );

    // "TYPE (contents)" -> Pair(wkbType, "contents")
    static QPair<QgsWKBTypes::Type, QString> wktReadBlock( const QString& wkt );
    // "TYPE1 (contents1), TYPE2 (TYPE3 (contents3), TYPE4 (contents4))" -> List("TYPE1 (contents1)", "TYPE2 (TYPE3 (contents3), TYPE4 (contents4))")
    static QStringList wktGetChildBlocks( const QString& wkt , const QString &defaultType = "" );
    static bool readWkbHeader( QgsConstWkbPtr& wkbPtr, QgsWKBTypes::Type& wkbType, bool& endianSwap, QgsWKBTypes::Type expectedType );
};

#endif //QGSABSTRACTGEOMETRYV2
