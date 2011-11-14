#include "qgsgeometry.h"
#include "qgslinestring.h"

QgsGeometry* QgsGeometry::fromWkb( unsigned char * wkb, size_t length )
{
  if( length < 1 )
  {
    return 0;
  }

  int type;
  memcpy( &type, wkb + 1, sizeof( int ) );
  switch (type)
  {
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
      return QgsLineString::fromWkb(wkb);
    default:
      return 0;
  }
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

QgsPoint QgsGeometry::closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist ) const
{
  return QgsPoint(); //soon...
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments ) const
{
  return 0;
}

QgsGeometry* QgsGeometry::simplify( double tolerance ) const
{
  return 0;
}

QgsGeometry* QgsGeometry::centroid() const
{
  return 0;
}

QgsGeometry* QgsGeometry::convexHull() const
{
  return 0;
}

QgsGeometry* QgsGeometry::intersection( QgsGeometry* geometry ) const
{
  return 0;
}

QgsGeometry* QgsGeometry::combine( QgsGeometry* geometry ) const
{
  return 0;
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

