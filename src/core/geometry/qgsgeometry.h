/***************************************************************************
                              qgsgeometry.h
                              --------------
  begin                : November 2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRY_H
#define QGSGEOMETRY_H

#include "qgis.h"
#include "qgsrectangle.h"
#include <QVector>
#include <geos_c.h>


class QgsCoordinateTransform;
class QgsMapToPixel;
class QPainter;
class QPainterPath;
class QPolygonF;

class QgsGeometry
{
  public:
    enum GeometryType
    {
      Point = 0,
      MultiPoint,
      LineString,
      CircularString,
      CompoundCurve,
      MultiCurve,
      Polygon,
      CurvePolygon,
      MultiSurface,
      GeometryCollection
    };

    virtual void draw( QPainter* p ) const = 0;

    //changes the geometry in place
    virtual void coordinateTransform( const QgsCoordinateTransform& t ) = 0;
    virtual void pixelTransform( const QgsMapToPixel& mtp ) = 0;

    //import
    static QgsGeometry* fromWkb( unsigned char * wkb, size_t length );
    static QgsGeometry* fromWkt( const QString& wkt );
    static QgsGeometry* fromGeos( const GEOSGeometry* geos );

    //export
    virtual unsigned char* asWkb( int& wkbSize ) const = 0;
    virtual GEOSGeometry* asGeos() const = 0;
    virtual QString asWkt() const = 0;
    virtual QgsGeometry* clone() const = 0;

    //todo: support asPoint, asPolygon, ... such that client classes don't have to cast?

    virtual bool hasZ() const = 0;
    virtual bool hasM() const = 0;
    virtual GeometryType geometryType() const = 0;
    virtual QGis::WkbType wkbType() const = 0;
    virtual QGis::GeometryType type() const = 0;

    virtual QgsRectangle boundingBox() const = 0;

    virtual bool isMultipart() const = 0;

    virtual bool convertToMultiType() const = 0;

    /** Test for intersection with a rectangle (uses GEOS) */
    bool intersects( const QgsRectangle& r );

    /** Test for intersection with a geometry (uses GEOS) */
    bool intersects( QgsGeometry* geometry );

    /** Test for containment of a point (uses GEOS) */
    bool contains( QgsPoint* p );

    /** Test for if geometry is contained in an other (uses GEOS)
     *  @note added in 1.5 */
    bool contains( QgsGeometry* geometry );

    /** Test for if geometry is disjoint of an other (uses GEOS)
     *  @note added in 1.5 */
    bool disjoint( QgsGeometry* geometry );

    /** Test for if geometry equals an other (uses GEOS)
     *  @note added in 1.5 */
    bool equals( QgsGeometry* geometry );

    /** Test for if geometry touch an other (uses GEOS)
     *  @note added in 1.5 */
    bool touches( QgsGeometry* geometry );

    /** Test for if geometry overlaps an other (uses GEOS)
     *  @note added in 1.5 */
    bool overlaps( QgsGeometry* geometry );

    /** Test for if geometry is within an other (uses GEOS)
     *  @note added in 1.5 */
    bool within( QgsGeometry* geometry );

    /** Test for if geometry crosses an other (uses GEOS)
     *  @note added in 1.5 */
    bool crosses( QgsGeometry* geometry );

    bool isGeosEqual( QgsGeometry & ) const;
    bool isGeosValid() const;

    QgsPoint closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist ) const;
    QgsGeometry* buffer( double distance, int segments ) const;
    QgsGeometry* simplify( double tolerance ) const;
    QgsGeometry* centroid() const;
    QgsGeometry* convexHull() const;
    QgsGeometry* intersection( QgsGeometry* geometry ) const;
    QgsGeometry* combine( QgsGeometry* geometry ) const;

    //edit
    virtual bool insertVertex( double x, double y, int beforeVertex ) = 0; //m and z?
    virtual bool moveVertex( double x, double y, int atVertex ) = 0; //keep m and z
    virtual bool deleteVertex( int atVertex ) = 0;
    virtual bool addRing( const QList<QgsPoint>& ring ) = 0;
    virtual bool addPart( const QList<QgsPoint> &points ) = 0;
    virtual bool splitGeometry( const QList<QgsPoint>& splitLine,
                                QList<QgsGeometry*>&newGeometries,
                                bool topological,
                                QList<QgsPoint> &topologyTestPoints ) = 0;
    virtual bool reshapeGeometry( const QList<QgsPoint>& reshapeWithLine ) = 0;
    virtual bool makeDifference( QgsGeometry* other ) = 0;

    //analysis
    virtual QgsPoint vertexAt( int atVertex ) const = 0; //return m and z
    virtual double sqrDistToVertexAt( QgsPoint& point, int atVertex ) const = 0;
    virtual double closestVertexWithContext( const QgsPoint& point, int& atVertex ) const = 0;
    virtual double closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& beforeVertex ) const = 0;
    virtual void adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex ) const = 0;
    virtual bool deleteRing( int ringNum, int partNum = 0 ) const = 0;
    virtual bool deletePart( int partNum ) = 0;
    virtual double distance( QgsGeometry& geom ) const = 0;

    virtual int avoidIntersections() const = 0;

};

class QgsCurve: public QgsGeometry
{
  public:
    virtual double length() const = 0;
    virtual bool isClosed() const = 0;

  protected:
    virtual void addToPainterPath( QPainterPath& path ) const = 0;
};

#if 0
class QgsCircularString: public QgsCurve
{
  public:
    QgsCircularString();
    ~QgsCircularString();

  protected:
    void addToPainterPath( QPainterPath& path ) const;

  private:
    int mNVertices;
    QPointF* mVertices;
    //Z-Values or 0 for 2D geometries
    double* mZValues;
    //Measure values or 0
    double* mMValues;
};

class QgsCompoundCurve: public QgsCurve
{
  public:
    QgsCompoundCurve();
    ~QgsCompoundCurve();

  protected:
    void addToPainterPath( QPainterPath& path ) const;

  private:
    int mNCurveParts;
    QgsCurve* mCurveParts;
};

class QgsSurface: public QgsGeometry
{
  public:
    QgsSurface();
    ~QgsSurface();

    double area() const = 0;
    double perimeter() const = 0;
};

/**Polygon with straight lined rings*/
class QgsPolygon: public QgsSurface
{
    QgsPolygon();
    ~QgsPolygon();

    //linear rings
};

class QgsCurvePolygon: public QgsSurface
{
  public:
    QgsCurvePolygon();
    ~QgsCurvePolygon();

    //may contain any (closed) curvetypes
};

//multitypes
class QgsMultiPoint
{
};

class QgsMultiCurve
{
};

class QgsMultiSurface
{
};

class QgsGeometryCollection
{
};

#endif //0

#endif // QGSGEOMETRY_H

