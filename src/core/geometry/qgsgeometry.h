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
#include "qgsfeature.h"
#include "qgsrectangle.h"
#include <QMap>
#include <QSet>
#include <QVector>
#include <geos_c.h>

class QgsAbstractGeometry;
class QgsCoordinateTransform;
class QgsMapToPixel;
class QgsVectorLayer;
class QPainter;
class QPainterPath;
class QPolygonF;

/** polyline is represented as a vector of points */
typedef QVector<QgsPoint> QgsPolyline;

/** polygon: first item of the list is outer ring, inner rings (if any) start from second item */
typedef QVector<QgsPolyline> QgsPolygon;

/** a collection of QgsPoints that share a common collection of attributes */
typedef QVector<QgsPoint> QgsMultiPoint;

/** a collection of QgsPolylines that share a common collection of attributes */
typedef QVector<QgsPolyline> QgsMultiPolyline;

/** a collection of QgsPolygons that share a common collection of attributes */
typedef QVector<QgsPolygon> QgsMultiPolygon;

//Wrapper around abstract geometry.
class QgsGeometry
{
  public:

    class Error
    {
        QString message;
        QgsPoint location;
        bool hasLocation;
      public:
        Error() : message( "none" ), hasLocation( false ) {}
        Error( QString m ) : message( m ), hasLocation( false ) {}
        Error( QString m, QgsPoint p ) : message( m ), location( p ), hasLocation( true ) {}

        QString what() { return message; };
        QgsPoint where() { return location; }
        bool hasWhere() { return hasLocation; }
    };

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
      GeometryCollection,
      NoGeometry
    };

    enum VertexMarkerType
    {
      SemiTransparentCircle,
      Cross,
      NoMarker  /* added in version 1.1 */
    };

    //! Constructor
    QgsGeometry();

    QgsGeometry( QgsAbstractGeometry* geom );

    /** copy constructor will prompt a deep copy of the object */
    QgsGeometry( const QgsGeometry & );

    /** assignments will prompt a deep copy of the object
      @note not available in python bindings
      */
    QgsGeometry & operator=( QgsGeometry const & rhs );

    //! Destructor
    ~QgsGeometry();

    void draw( QPainter* p ) const;
    void drawVertexMarkers( QPainter* p, VertexMarkerType type, int size ) const;
    void coordinateTransform( const QgsCoordinateTransform& t );
    void pixelTransform( const QgsMapToPixel& mtp );

    /** static method that creates geometry from Wkt */
    static QgsGeometry* fromWkt( QString wkt );

    /** static method that creates geometry from GML2
      @note added in 1.9
      */
    static QgsGeometry* fromGML2( const QDomNode& geometryNode );

    /** construct geometry from a point */
    static QgsGeometry* fromPoint( const QgsPoint& point );
    /** construct geometry from a multipoint */
    static QgsGeometry* fromMultiPoint( const QgsMultiPoint& multipoint );
    /** construct geometry from a polyline */
    static QgsGeometry* fromPolyline( const QgsPolyline& polyline );
    /** construct geometry from a multipolyline*/
    static QgsGeometry* fromMultiPolyline( const QgsMultiPolyline& multiline );
    /** construct geometry from a polygon */
    static QgsGeometry* fromPolygon( const QgsPolygon& polygon );
    /** construct geometry from a multipolygon */
    static QgsGeometry* fromMultiPolygon( const QgsMultiPolygon& multipoly );
    /** construct geometry from a rectangle */
    static QgsGeometry* fromRect( const QgsRectangle& rect );
    /**
      Set the geometry, feeding in a geometry in GEOS format.
      This class will take ownership of the buffer.
      @note not available in python bindings
     */
    void fromGeos( GEOSGeometry* geos );
    /**
      Set the geometry, feeding in the buffer containing OGC Well-Known Binary and the buffer's length.
      This class will take ownership of the buffer.
     */
    void fromWkb( unsigned char * wkb, size_t length );

    /**
       Returns the buffer containing this geometry in WKB format.
       You may wish to use in conjunction with wkbSize().
    */
    unsigned char * asWkb();

    /**
     * Returns the size of the WKB in asWkb().
     */
    size_t wkbSize();

    bool hasZ() const;

    bool hasM() const;

    /**Returns a geos geomtry. QgsGeometry keeps ownership, don't delete the returned object!
        @note this method was added in version 1.1
        @note not available in python bindings
      */
    GEOSGeometry* asGeos();

    /** Returns type of wkb (point / linestring / polygon etc.) */
    QGis::WkbType wkbType();

    /** Returns type of the vector */
    QGis::GeometryType type();

    QGis::GeometryType geometryType() const;

    /** Returns true if wkb of the geometry is of WKBMulti* type */
    bool isMultipart();

    /** compare geometries using GEOS
      @note added in 1.5
     */
    bool isGeosEqual( QgsGeometry& geom );

    /** check validity using GEOS
      @note added in 1.5
     */
    bool isGeosValid();

    /** check if geometry is empty using GEOS
      @note added in 1.5
     */
    bool isGeosEmpty();

    /** get area of geometry using GEOS
      @note added in 1.5
     */
    double area();

    /** get length of geometry using GEOS
      @note added in 1.5
     */
    double length();

    double distance( QgsGeometry& geom );

    /**
       Returns the vertex closest to the given point, the corresponding vertex index, squared distance snap point / target point
    and the indices of the vertices before/after. The vertices before/after are -1 if not present
    */
    QgsPoint closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist );


    /**
       Returns the indexes of the vertices before and after the given vertex index.

       This function takes into account the following factors:

       1. If the given vertex index is at the end of a linestring,
          the adjacent index will be -1 (for "no adjacent vertex")
       2. If the given vertex index is at the end of a linear ring
          (such as in a polygon), the adjacent index will take into
          account the first vertex is equal to the last vertex (and will
          skip equal vertex positions).
    */
    void adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex );


    /** Insert a new vertex before the given vertex index,
     *  ring and item (first number is index 0)
     *  If the requested vertex number (beforeVertex.back()) is greater
     *  than the last actual vertex on the requested ring and item,
     *  it is assumed that the vertex is to be appended instead of inserted.
     *  Returns false if atVertex does not correspond to a valid vertex
     *  on this geometry (including if this geometry is a Point).
     *  It is up to the caller to distinguish between
     *  these error conditions.  (Or maybe we add another method to this
     *  object to help make the distinction?)
     */
    bool insertVertex( double x, double y, int beforeVertex );

    /** Moves the vertex at the given position number
     *  and item (first number is index 0)
     *  to the given coordinates.
     *  Returns false if atVertex does not correspond to a valid vertex
     *  on this geometry
     */
    bool moveVertex( double x, double y, int atVertex );

    /** Deletes the vertex at the given position number and item
     *  (first number is index 0)
     *  Returns false if atVertex does not correspond to a valid vertex
     *  on this geometry (including if this geometry is a Point),
     *  or if the number of remaining verticies in the linestring
     *  would be less than two.
     *  It is up to the caller to distinguish between
     *  these error conditions.  (Or maybe we add another method to this
     *  object to help make the distinction?)
     */
    bool deleteVertex( int atVertex );

    /**
     *  Returns coordinates of a vertex.
     *  @param atVertex index of the vertex
     *  @return Coordinates of the vertex or QgsPoint(0,0) on error
     */
    QgsPoint vertexAt( int atVertex );

    /**
     *  Returns the squared cartesian distance between the given point
     *  to the given vertex index (vertex at the given position number,
     *  ring and item (first number is index 0))
     */
    double sqrDistToVertexAt( QgsPoint& point, int atVertex );

    /**
     * Searches for the closest vertex in this geometry to the given point.
     * @param point Specifiest the point for search
     * @param atVertex Receives index of the closest vertex
     * @return The squared cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestVertexWithContext( const QgsPoint& point, int& atVertex );

    /**
     * Searches for the closest segment of geometry to the given point
     * @param point Specifies the point for search
     * @param minDistPoint Receives the nearest point on the segment
     * @param afterVertex Receives index of the vertex after the closest segment. The vertex
     * before the closest segment is always afterVertex - 1
     * @param leftOf Out: Returns if the point lies on the left of right side of the segment ( < 0 means left, > 0 means right )
     * @param epsilon epsilon for segment snapping (added in 1.8)
     * @return The squared cartesian distance is also returned in sqrDist, negative number on error
     */
    double closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& afterVertex, double* leftOf = 0, double epsilon = DEFAULT_SEGMENT_EPSILON );

    /**Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     @return 0 in case of success (ring added), 1 problem with geometry type, 2 ring not closed,
     3 ring is not valid geometry, 4 ring not disjoint with existing rings, 5 no polygon found which contained the ring*/
    int addRing( const QList<QgsPoint>& ring );

    /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature*/
    int addPart( const QList<QgsPoint> &points );

    /**Translate this geometry by dx, dy
     @return 0 in case of success*/
    int translate( double dx, double dy );

    /**Transform this geometry as described by CoordinateTranasform ct
     @return 0 in case of success*/
    int transform( const QgsCoordinateTransform& ct );

    /**Splits this geometry according to a given line. Note that the geometry is only split once. If there are several intersections
     between geometry and splitLine, only the first one is considered.
    @param splitLine the line that splits the geometry
    @param[out] newGeometries list of new geometries that have been created with the split
    @param topological true if topological editing is enabled
    @param[out] topologyTestPoints points that need to be tested for topological completeness in the dataset
    @return 0 in case of success, 1 if geometry has not been split, error else*/
    int splitGeometry( const QList<QgsPoint>& splitLine,
                       QList<QgsGeometry*>&newGeometries,
                       bool topological,
                       QList<QgsPoint> &topologyTestPoints );

    /**Replaces a part of this geometry with another line
      @return 0 in case of success
      @note: this function was added in version 1.3*/
    int reshapeGeometry( const QList<QgsPoint>& reshapeWithLine );

    /**Changes this geometry such that it does not intersect the other geometry
       @param other geometry that should not be intersect
       @return 0 in case of success*/
    int makeDifference( QgsGeometry* other );

    /**Returns the bounding box of this feature*/
    QgsRectangle boundingBox();

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

    /** Returns a buffer region around this geometry having the given width and with a specified number
        of segments used to approximate curves */
    QgsGeometry* buffer( double distance, int segments );

    /** Returns a simplified version of this geometry using a specified tolerance value */
    QgsGeometry* simplify( double tolerance );

    /** Returns the center of mass of a geometry
    * @note for line based geometries, the center point of the line is returned,
    * and for point based geometries, the point itself is returned */
    QgsGeometry* centroid();

    /** Returns the smallest convex polygon that contains all the points in the geometry. */
    QgsGeometry* convexHull();

    /* Return interpolated point on line at distance
     * @note added in 1.9
     */
    QgsGeometry* interpolate( double distance );

    /** Returns a geometry representing the points shared by this geometry and other. */
    QgsGeometry* intersection( QgsGeometry* geometry );

    /** Returns a geometry representing all the points in this geometry and other (a
     * union geometry operation).
     * @note this operation is not called union since its a reserved word in C++.*/
    QgsGeometry* combine( QgsGeometry* geometry );

    /** Returns a geometry representing the points making up this geometry that do not make up other. */
    QgsGeometry* difference( QgsGeometry* geometry );

    /** Returns a Geometry representing the points making up this Geometry that do not make up other. */
    QgsGeometry* symDifference( QgsGeometry* geometry );

    /** Exports the geometry to mWkt
     *  @return true in case of success and false else
     */
    QString exportToWkt();

    /** Exports the geometry to mGeoJSON
     *  @return true in case of success and false else
     *  @note added in 1.8
     *  @note python binding added in 1.9
     */
    QString exportToGeoJSON();

    /** Exports the geometry to mGML2
        @return true in case of success and false else
     *  @note added in 1.9
     */
    QDomElement exportToGML2( QDomDocument& doc );

    /* Accessor functions for getting geometry data */

    /** return contents of the geometry as a point
        if wkbType is WKBPoint, otherwise returns [0,0] */
    QgsPoint asPoint();

    /** return contents of the geometry as a polyline
        if wkbType is WKBLineString, otherwise an empty list */
    QgsPolyline asPolyline();

    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsPolygon asPolygon();

    /** return contents of the geometry as a multi point
        if wkbType is WKBMultiPoint, otherwise an empty list */
    QgsMultiPoint asMultiPoint();

    /** return contents of the geometry as a multi linestring
        if wkbType is WKBMultiLineString, otherwise an empty list */
    QgsMultiPolyline asMultiPolyline();

    /** return contents of the geometry as a multi polygon
        if wkbType is WKBMultiPolygon, otherwise an empty list */
    QgsMultiPolygon asMultiPolygon();

    /** return contents of the geometry as a list of geometries
     @note added in version 1.1 */
    QList<QgsGeometry*> asGeometryCollection();

    /** delete a ring in polygon or multipolygon.
      Ring 0 is outer ring and can't be deleted.
      @return true on success
      @note added in version 1.2 */
    bool deleteRing( int ringNum, int partNum = 0 );

    /** delete part identified by the part number
      @return true on success
      @note added in version 1.2 */
    bool deletePart( int partNum );

    /**Converts single type geometry into multitype geometry
     e.g. a polygon into a multipolygon geometry with one polygon
    @return true in case of success and false else*/
    bool convertToMultiType();

    /** Modifies geometry to avoid intersections with the layers specified in project properties
     *  @return 0 in case of success,
     *          1 if geometry is not of polygon type,
     *          2 if avoid intersection would change the geometry type,
     *          3 other error during intersection removal
     *  @param ignoreFeatures possibility to give a list of features where intersections should be ignored (not available in python bindings)
     *  @note added in 1.5
     */
    int avoidIntersections( QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures = ( QMap<QgsVectorLayer*, QSet<QgsFeatureId> >() ) );

    /** Validate geometry and produce a list of geometry errors
     * @note added in 1.5
     * @note python binding added in 1.6
     **/
    void validateGeometry( QList<Error> &errors );

    const QgsAbstractGeometry* geometry() { return mGeometry; }

  private:
    QgsAbstractGeometry* mGeometry;
};

class QgsAbstractGeometry
{
  public:
    QgsAbstractGeometry();
    virtual ~QgsAbstractGeometry();

    virtual void draw( QPainter* p ) const = 0;
    virtual void drawVertexMarkers( QPainter* p, QgsGeometry::VertexMarkerType type, int size ) const = 0;

    //changes the geometry in place
    virtual void coordinateTransform( const QgsCoordinateTransform& t ) = 0;
    virtual void pixelTransform( const QgsMapToPixel& mtp ) = 0;
    virtual int translate( double dx, double dy ) = 0;

    //import
    static QgsAbstractGeometry* fromWkb( unsigned char * wkb, size_t length );
    static QgsAbstractGeometry* fromWkt( const QString& wkt );
    static QgsAbstractGeometry* fromGeos( const GEOSGeometry* geos );
    static QgsAbstractGeometry* fromGML2( const QDomNode& geometryNode );
    /** construct geometry from a point */
    static QgsAbstractGeometry* fromPoint( const QgsPoint& point );
    /** construct geometry from a multipoint */
    static QgsAbstractGeometry* fromMultiPoint( const QgsMultiPoint& multipoint );
    /** construct geometry from a polyline */
    static QgsAbstractGeometry* fromPolyline( const QgsPolyline& polyline );
    /** construct geometry from a multipolyline*/
    static QgsAbstractGeometry* fromMultiPolyline( const QgsMultiPolyline& multiline );
    /** construct geometry from a polygon */
    static QgsAbstractGeometry* fromPolygon( const QgsPolygon& polygon );
    /** construct geometry from a multipolygon */
    static QgsAbstractGeometry* fromMultiPolygon( const QgsMultiPolygon& multipoly );
    /** construct geometry from a rectangle */
    static QgsAbstractGeometry* fromRect( const QgsRectangle& rect );

    //export
    virtual unsigned char* asWkb( int& wkbSize ) const = 0;
    virtual GEOSGeometry* asGeos() const = 0;
    virtual QString asWkt() const = 0;
    virtual QDomElement asGML2( QDomDocument& doc ) const = 0;
    virtual QgsGeometry* clone() const = 0;

    virtual bool hasZ() const = 0;
    virtual bool hasM() const = 0;
    virtual QgsGeometry::GeometryType geometryType() const = 0;
    virtual QGis::WkbType wkbType() const = 0;
    virtual QGis::GeometryType type() const = 0;

    virtual QgsRectangle boundingBox() const = 0;

    virtual bool isMultipart() const = 0;

    virtual bool convertToMultiType() const = 0;

    /** Test for intersection with a rectangle (uses GEOS) */
    bool intersects( const QgsRectangle& r ) const;

    /** Test for intersection with a geometry (uses GEOS) */
    bool intersects( const QgsAbstractGeometry* geometry ) const;

    /** Test for containment of a point (uses GEOS) */
    bool contains( QgsPoint* p );

    /** Test for if geometry is contained in an other (uses GEOS)
     *  @note added in 1.5 */
    bool contains( const QgsAbstractGeometry* geometry );

    /** Test for if geometry is disjoint of an other (uses GEOS)
     *  @note added in 1.5 */
    bool disjoint( const QgsAbstractGeometry* geometry );

    /** Test for if geometry equals an other (uses GEOS)
     *  @note added in 1.5 */
    bool equals( const QgsAbstractGeometry* geometry );

    /** Test for if geometry touch an other (uses GEOS)
     *  @note added in 1.5 */
    bool touches( const QgsAbstractGeometry* geometry );

    /** Test for if geometry overlaps an other (uses GEOS)
     *  @note added in 1.5 */
    bool overlaps( const QgsAbstractGeometry* geometry );

    /** Test for if geometry is within an other (uses GEOS)
     *  @note added in 1.5 */
    bool within( const QgsAbstractGeometry* geometry );

    /** Test for if geometry crosses an other (uses GEOS)
     *  @note added in 1.5 */
    bool crosses( const QgsAbstractGeometry* geometry );

    bool isGeosEqual( QgsGeometry & ) const;
    bool isGeosValid() const;

    QgsPoint closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist ) const;
    QgsAbstractGeometry* buffer( double distance, int segments ) const;
    QgsAbstractGeometry* simplify( double tolerance ) const;
    QgsAbstractGeometry* centroid() const;
    QgsAbstractGeometry* convexHull() const;
    QgsAbstractGeometry* intersection( const QgsAbstractGeometry* geometry ) const;
    QgsAbstractGeometry* combine( const QgsAbstractGeometry* geometry ) const;

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
    virtual double closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& afterVertex,
        double* leftOf = 0, double epsilon = DEFAULT_SEGMENT_EPSILON ) const = 0;
    virtual void adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex ) const = 0;
    virtual bool deleteRing( int ringNum, int partNum = 0 ) const = 0;
    virtual bool deletePart( int partNum ) = 0;
    virtual double distance( QgsGeometry& geom ) const = 0;

    virtual int avoidIntersections( QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures = ( QMap<QgsVectorLayer*, QSet<QgsFeatureId> >() ) ) const = 0;

    //compatibility with old code
    /** return contents of the geometry as a point
        if wkbType is WKBPoint, otherwise returns [0,0] */
    QgsPoint asPoint();

    /** return contents of the geometry as a polyline
        if wkbType is WKBLineString, otherwise an empty list */
    QgsPolyline asPolyline();

    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsPolygon asPolygon();

    /** return contents of the geometry as a multi point
        if wkbType is WKBMultiPoint, otherwise an empty list */
    QgsMultiPoint asMultiPoint();

    /** return contents of the geometry as a multi linestring
        if wkbType is WKBMultiLineString, otherwise an empty list */
    QgsMultiPolyline asMultiPolyline();

    /** return contents of the geometry as a multi polygon
        if wkbType is WKBMultiPolygon, otherwise an empty list */
    QgsMultiPolygon asMultiPolygon();


  protected:
    /**Updated cached geos*/
    void cacheGeos() const;
    const GEOSGeometry* geosGeom() const;
    static void drawVertexMarker( double x, double y, QPainter* p, QgsGeometry::VertexMarkerType type, int size );

  private:
    // reference counting
    int refs;
    void ref(); // add reference
    void deref(); // remove reference, delete if refs == 0
    friend class QgsGeometry;

    //cached geos object
    mutable GEOSGeometry* mGeosGeom;
};

class QgsCurve: public QgsAbstractGeometry
{
  public:
    virtual double length() const = 0;
    virtual bool isClosed() const = 0;

  protected:
    virtual void addToPainterPath( QPainterPath& path ) const = 0;
};

class QgsSurface: public QgsAbstractGeometry
{
  public:
    virtual double area() const = 0;

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

class QgsSurface: public QgsAbstractGeometry
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

