/***************************************************************************
  qgsgeometry.h - Geometry (stored as Open Geospatial Consortium WKB)
  -------------------------------------------------------------------
Date                 : 02 May 2005
Copyright            : (C) 2005 by Brendan Morley
email                : morb at ozemail dot com dot au
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRY_H
#define QGSGEOMETRY_H

#include <QString>
#include <QVector>
#include <QDomDocument>

#include "qgis.h"

#include <geos_c.h>

#if defined(GEOS_VERSION_MAJOR) && (GEOS_VERSION_MAJOR<3)
#define GEOSGeometry struct GEOSGeom_t
#define GEOSCoordSequence struct GEOSCoordSeq_t
#endif

#include "qgsabstractgeometryv2.h"
#include "qgspoint.h"
#include "qgscoordinatetransform.h"
#include "qgsfeature.h"

#include <QSet>

class QgsGeometryEngine;
class QgsVectorLayer;
class QgsMapToPixel;
class QPainter;
class QgsPolygonV2;

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

class QgsRectangle;

/** \ingroup core
 * A geometry is the spatial representation of a feature.
 * Represents a geometry with input and output in formats specified by
 * (at least) the Open Geospatial Consortium (WKB / Wkt), and containing
 * various functions for geoprocessing of the geometry.
 *
 * The geometry is represented internally by the OGC WKB format or
 * as GEOS geometry. Some functions use WKB for their work, others
 * use GEOS.
 *
 * TODO: migrate completely to GEOS and only support WKB/Wkt import/export.
 *
 * @author Brendan Morley
 */
class QgsConstWkbPtr;

struct QgsGeometryData;

class CORE_EXPORT QgsGeometry
{
  public:
    //! Constructor
    QgsGeometry();

    /** copy constructor will prompt a deep copy of the object */
    QgsGeometry( const QgsGeometry & );

    /** assignments will prompt a deep copy of the object
      @note not available in python bindings
      */
    QgsGeometry & operator=( QgsGeometry const & rhs );

    QgsGeometry( QgsAbstractGeometryV2* geom );

    //! Destructor
    ~QgsGeometry();

    const QgsAbstractGeometryV2* geometry() const;

    /** static method that creates geometry from Wkt */
    static QgsGeometry* fromWkt( QString wkt );

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
    const unsigned char* asWkb() const;

    /**
     * Returns the size of the WKB in asWkb().
     */
    size_t wkbSize() const;

    /**Returns a geos geomtry. QgsGeometry keeps ownership, don't delete the returned object!
        @note this method was added in version 1.1
        @note not available in python bindings
      */
    const GEOSGeometry* asGeos() const;

    /** Returns type of wkb (point / linestring / polygon etc.) */
    QGis::WkbType wkbType() const;

    /** Returns type of the vector */
    QGis::GeometryType type() const;

    /** Returns true if wkb of the geometry is of WKBMulti* type */
    bool isMultipart() const;

    /** compare geometries using GEOS
      @note added in 1.5
     */
    bool isGeosEqual( QgsGeometry & );

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

    /** Moves the vertex at the given position number
     *  and item (first number is index 0)
     *  to the given coordinates.
    //     *  Returns false if atVertex does not correspond to a valid vertex
     *  on this geometry
     */
    bool moveVertex( const QgsPointV2& p, int atVertex );

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

    /**Adds a new ring to this geometry. This makes only sense for polygon and multipolygons.
     @return 0 in case of success (ring added), 1 problem with geometry type, 2 ring not closed,
     3 ring is not valid geometry, 4 ring not disjoint with existing rings, 5 no polygon found which contained the ring*/
    int addRing( QgsCurveV2* ring );

    /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature*/
    int addPart( const QList<QgsPoint> &points, QGis::GeometryType geomType = QGis::UnknownGeometry );

    /**Adds a new part to this geometry (takes ownership)
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature*/
    int addPart( QgsCurveV2* part );

    /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature
     @note not available in python bindings
     */
    int addPart( GEOSGeometry *newPart );

    /**Adds a new island polygon to a multipolygon feature
     @return 0 in case of success, 1 if not a multipolygon, 2 if ring is not a valid geometry, 3 if new polygon ring
     not disjoint with existing polygons of the feature
     @note available in python bindings as addPartGeometry (added in 2.2)
     */
    int addPart( const QgsGeometry *newPart );

    /**Translate this geometry by dx, dy
     @return 0 in case of success*/
    int translate( double dx, double dy );

    /**Splits this geometry according to a given line.
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
    QgsRectangle boundingBox() const;

    /** Test for intersection with a rectangle (uses GEOS) */
    bool intersects( const QgsRectangle& r ) const;

    /** Test for intersection with a geometry (uses GEOS) */
    bool intersects( const QgsGeometry* geometry ) const;

    /** Test for containment of a point (uses GEOS) */
    bool contains( const QgsPoint* p ) const;

    /** Test for if geometry is contained in another (uses GEOS)
     *  @note added in 1.5 */
    bool contains( const QgsGeometry* geometry ) const;

    /** Test for if geometry is disjoint of another (uses GEOS)
     *  @note added in 1.5 */
    bool disjoint( const QgsGeometry* geometry ) const;

    /** Test for if geometry equals another (uses GEOS)
     *  @note added in 1.5 */
    bool equals( const QgsGeometry* geometry ) const;

    /** Test for if geometry touch another (uses GEOS)
     *  @note added in 1.5 */
    bool touches( const QgsGeometry* geometry ) const;

    /** Test for if geometry overlaps another (uses GEOS)
     *  @note added in 1.5 */
    bool overlaps( const QgsGeometry* geometry ) const;

    /** Test for if geometry is within another (uses GEOS)
     *  @note added in 1.5 */
    bool within( const QgsGeometry* geometry ) const;

    /** Test for if geometry crosses another (uses GEOS)
     *  @note added in 1.5 */
    bool crosses( const QgsGeometry* geometry ) const;

    /** Returns a buffer region around this geometry having the given width and with a specified number
        of segments used to approximate curves */
    QgsGeometry* buffer( double distance, int segments );

    /** Returns a buffer region around the geometry, with additional style options.
     * @param distance    buffer distance
     * @param segments    For round joins, number of segments to approximate quarter-circle
     * @param endCapStyle Round (1) / Flat (2) / Square (3) end cap style
     * @param joinStyle   Round (1) / Mitre (2) / Bevel (3) join style
     * @param mitreLimit  Limit on the mitre ratio used for very sharp corners
     * @note added in 2.4
     * @note needs GEOS >= 3.3 - otherwise always returns 0
     */
    QgsGeometry* buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit );

    /** Returns an offset line at a given distance and side from an input line.
     * See buffer() method for details on parameters.
     * @note added in 2.4
     * @note needs GEOS >= 3.3 - otherwise always returns 0
     */
    QgsGeometry* offsetCurve( double distance, int segments, int joinStyle, double mitreLimit );

    /** Returns a simplified version of this geometry using a specified tolerance value */
    QgsGeometry* simplify( double tolerance );

    /** Returns the center of mass of a geometry
    * @note for line based geometries, the center point of the line is returned,
    * and for point based geometries, the point itself is returned */
    QgsGeometry* centroid();

    /** Returns a point within a geometry */
    QgsGeometry* pointOnSurface();

    /** Returns the smallest convex polygon that contains all the points in the geometry. */
    QgsGeometry* convexHull();

    /* Return interpolated point on line at distance
     * @note added in 1.9
     */
    QgsGeometry* interpolate( double distance );

    /** Returns a geometry representing the points shared by this geometry and other. */
    QgsGeometry* intersection( QgsGeometry* geometry ) const;

    /** Returns a geometry representing all the points in this geometry and other (a
     * union geometry operation).
     * @note this operation is not called union since its a reserved word in C++.*/
    QgsGeometry* combine( QgsGeometry* geometry );

    /** Returns a geometry representing the points making up this geometry that do not make up other. */
    QgsGeometry* difference( QgsGeometry* geometry );

    /** Returns a Geometry representing the points making up this Geometry that do not make up other. */
    QgsGeometry* symDifference( QgsGeometry* geometry );

    /** Exports the geometry to WKT
     *  @note precision parameter added in 2.4
     *  @return true in case of success and false else
     */
    QString exportToWkt( const int &precision = 17 ) const;

    /** Exports the geometry to GeoJSON
     *  @return a QString representing the geometry as GeoJSON
     *  @note added in 1.8
     *  @note python binding added in 1.9
     *  @note precision parameter added in 2.4
     */
    QString exportToGeoJSON( const int &precision = 17 ) const;

    /** try to convert the geometry to the requested type
     * @param destType the geometry type to be converted to
     * @param destMultipart determines if the output geometry will be multipart or not
     * @return the converted geometry or NULL pointer if the conversion fails.
     * @note added in 2.2
     */
    QgsGeometry* convertToType( QGis::GeometryType destType, bool destMultipart = false );


    /* Accessor functions for getting geometry data */

    /** return contents of the geometry as a point
        if wkbType is WKBPoint, otherwise returns [0,0] */
    QgsPoint asPoint() const;

    /** return contents of the geometry as a polyline
        if wkbType is WKBLineString, otherwise an empty list */
    QgsPolyline asPolyline() const;

    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsPolygon asPolygon() const;

    /** return contents of the geometry as a multi point
        if wkbType is WKBMultiPoint, otherwise an empty list */
    QgsMultiPoint asMultiPoint() const;

    /** return contents of the geometry as a multi linestring
        if wkbType is WKBMultiLineString, otherwise an empty list */
    QgsMultiPolyline asMultiPolyline() const;

    /** return contents of the geometry as a multi polygon
        if wkbType is WKBMultiPolygon, otherwise an empty list */
    QgsMultiPolygon asMultiPolygon() const;

    /** return contents of the geometry as a list of geometries
     @note added in version 1.1 */
    QList<QgsGeometry*> asGeometryCollection() const;

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

    /** Validate geometry and produce a list of geometry errors
     * @note added in 1.5
     * @note python binding added in 1.6
     **/
    void validateGeometry( QList<Error> &errors );

    /** compute the unary union on a list of geometries. May be faster than an iterative union on a set of geometries.
        @param geometryList a list of QgsGeometry* as input
        @returns the new computed QgsGeometry, or null
    */
    static QgsGeometry *unaryUnion( const QList<QgsGeometry*>& geometryList );

    void convertToStraightSegment();

    int transform( const QgsCoordinateTransform& ct );
    void mapToPixel( const QgsMapToPixel& mtp );
    void clip( const QgsRectangle& rect );
    void draw( QPainter& p ) const;

    bool vertexIdFromVertexNr( int nr, QgsVertexId& id ) const;
    int vertexNrFromVertexId( const QgsVertexId& i ) const;

  private:

    QgsGeometryData* d; //implicitely shared data pointer
    mutable const unsigned char* mWkb; //store wkb pointer for backward compatibility
    mutable int mWkbSize;
    mutable GEOSGeometry* mGeos;

    void detach( bool cloneGeom = true ); //make sure mGeometry only referenced from this instance
    void removeWkbGeos();

    //convert point list from v1 to v2
    static void convertPointList( const QList<QgsPoint>& input, QList<QgsPointV2>& output );
    static void convertPointList( const QList<QgsPointV2>& input, QList<QgsPoint>& output );
    static void convertToPolyline( const QList<QgsPointV2>& input, QgsPolyline& output );
    static void convertPolygon( const QgsPolygonV2& input, QgsPolygon& output );
}; // class QgsGeometry

Q_DECLARE_METATYPE( QgsGeometry );

#endif
