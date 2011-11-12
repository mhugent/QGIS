#include "qgsgeometry.h"

QgsGeometry* QgsGeometry::fromWkb( unsigned char * wkb, size_t length )
{
  return 0;
}

QgsGeometry* QgsGeometry::fromWkt( const QString& wkt )
{
  return 0;
}

QgsGeometry* QgsGeometry::fromGeos( const GEOSGeometry* geos )
{
  return 0;
}

/** Test for intersection with a rectangle (uses GEOS) */
bool QgsGeometry::intersects( const QgsRectangle& r )
{
  return false;
}

/** Test for intersection with a geometry (uses GEOS) */
bool QgsGeometry::intersects( QgsGeometry* geometry )
{
  return false;
}

/** Test for containment of a point (uses GEOS) */
bool QgsGeometry::contains( QgsPoint* p )
{
  return false;
}

/** Test for if geometry is contained in an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsGeometry::contains( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry is disjoint of an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsGeometry::disjoint( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry equals an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsGeometry::equals( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry touch an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsGeometry::touches( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry overlaps an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsGeometry::overlaps( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry is within an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsGeometry::within( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry crosses an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsGeometry::crosses( QgsGeometry* geometry )
{
  return false;
}

bool QgsGeometry::isGeosEqual( QgsGeometry & ) const
{
  return false;
}

bool QgsGeometry::isGeosValid() const
{
  return false;
}

//linestring
QgsLineString::QgsLineString()
{
}

QgsLineString::~QgsLineString()
{
}

void QgsLineString::draw( QPainter* p ) const
{
}

void QgsLineString::transform( const QgsCoordinateTransform& t )
{
}

void QgsLineString::mapToPixel( const QgsMapToPixel& mtp )
{
}

//import
QgsGeometry* QgsLineString::fromWkb( unsigned char * wkb, size_t length )
{
  return 0;
}

QgsGeometry* QgsLineString::fromWkt( const QString& wkt )
{
  return 0;
}

QgsGeometry* QgsLineString::fromGeos( const GEOSGeometry* geos )
{
  return 0;
}

//export
unsigned char* QgsLineString::asWkb( int& wkbSize ) const
{
  wkbSize = 0;
  return 0;
}

GEOSGeometry* QgsLineString::asGeos() const
{
  return 0;
}

QString QgsLineString::asWkt() const
{
  return "";
}

QgsGeometry* QgsLineString::clone() const
{
  return 0;
}

void QgsLineString::addToPainterPath( QPainterPath& path ) const
{
  //move to first point
  //line to other points
}

#if 0
QgsCircularString::QgsCircularString()
{
}

QgsCircularString::~QgsCircularString()
{
}

void QgsCircularString::addToPainterPath( QPainterPath& path ) const
{
  //use series of arcTo()
}

QgsCompoundCurve::QgsCompoundCurve()
{
}

QgsCompoundCurve::~QgsCompoundCurve()
{
}

void QgsCompoundCurve::addToPainterPath( QPainterPath& path ) const
{
  //call addToPainterPath(path) for all subcurves
}
#endif //0

