/***************************************************************************
                              qgsgeometry.cpp
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


#include "qgsgeometry.h"
#include "qgslinestring.h"

/*************************geometry wrapper*****************************************/
QgsGeometry::QgsGeometry(): mGeometry( 0 )
{
}

QgsGeometry::~QgsGeometry()
{
    if( mGeometry )
    {
        mGeometry->deref();
    }
}

QgsGeometry::QgsGeometry( QgsAbstractGeometry* geom ): mGeometry( geom )
{
    if( mGeometry )
    {
        mGeometry->ref();
    }
}

QgsGeometry::QgsGeometry( const QgsGeometry& other ): mGeometry( 0 )
{
    mGeometry = other.mGeometry;
    if( mGeometry )
    {
        mGeometry->ref();
    }
}

QgsGeometry& QgsGeometry::operator=( QgsGeometry const& other )
{
    if( this != &other )
    {
        if( mGeometry )
        {
            mGeometry->deref();
        }
        mGeometry = other.mGeometry;
        if( mGeometry )
        {
            mGeometry->ref();
        }
    }
    return *this;
}

//delegate other methods to mGeometry
void QgsGeometry::draw( QPainter* p ) const
{
    if( mGeometry )
        mGeometry->draw( p );
}

void QgsGeometry::coordinateTransform( const QgsCoordinateTransform& t )
{
    if( mGeometry )
        mGeometry->coordinateTransform( t );
}

void QgsGeometry::pixelTransform( const QgsMapToPixel& mtp )
{
    if( mGeometry )
        mGeometry->pixelTransform( mtp );
}

void QgsGeometry::fromWkb( unsigned char * wkb, size_t length )
{
    if( mGeometry )
    {
        mGeometry->deref();
    }

    mGeometry = QgsAbstractGeometry::fromWkb( wkb, length );

    if( mGeometry )
    {
        mGeometry->ref();
    }
}

QgsGeometry* QgsGeometry::fromWkt( QString wkt )
{
    return new QgsGeometry( QgsAbstractGeometry::fromWkt( wkt ) );
}

void QgsGeometry::fromGeos( GEOSGeometry* geos )
{
    if( mGeometry )
    {
        mGeometry->deref();
    }
    mGeometry = QgsAbstractGeometry::fromGeos( geos );
    if( mGeometry )
    {
        mGeometry->ref();
    }
}

QgsGeometry* QgsGeometry::fromGML2( const QDomNode& geometryNode )
{
    return new QgsGeometry( QgsAbstractGeometry::fromGML2( geometryNode ) );
}

QgsGeometry* QgsGeometry::fromPoint( const QgsPoint& point )
{
    return new QgsGeometry( QgsAbstractGeometry::fromPoint( point ) );
}

QgsGeometry* QgsGeometry::fromMultiPoint( const QgsMultiPoint& multipoint )
{
    return new QgsGeometry( QgsAbstractGeometry::fromMultiPoint( multipoint ) );
}

QgsGeometry* QgsGeometry::fromPolyline( const QgsPolyline& polyline )
{
    return new QgsGeometry( QgsAbstractGeometry::fromPolyline( polyline ) );
}

QgsGeometry* QgsGeometry::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
    return new QgsGeometry( QgsAbstractGeometry::fromMultiPolyline( multiline ) );
}

QgsGeometry* QgsGeometry::fromPolygon( const QgsPolygon& polygon )
{
    return new QgsGeometry( QgsAbstractGeometry::fromPolygon( polygon ) );
}

QgsGeometry* QgsGeometry::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
    return new QgsGeometry( QgsAbstractGeometry::fromMultiPolygon( multipoly ) );
}

QgsGeometry* QgsGeometry::fromRect( const QgsRectangle& rect )
{
    return new QgsGeometry( QgsAbstractGeometry::fromRect( rect ) );
}

unsigned char* QgsGeometry::asWkb()
{
    if( mGeometry )
    {
        int size = 0; //unused
        return mGeometry->asWkb( size );
    }
    return 0;
}

GEOSGeometry* QgsGeometry::asGeos()
{
    if( mGeometry )
    {
        return mGeometry->asGeos();
    }
    return 0;
}

size_t QgsGeometry::wkbSize()
{
    if( !mGeometry )
    {
        return 0;
    }

    int wkbSize;
    unsigned char* wkb = mGeometry->asWkb( wkbSize );
    delete[] wkb;
    return wkbSize;
}

bool QgsGeometry::hasZ() const
{
    return ( mGeometry ? mGeometry->hasZ() : false );
}

bool QgsGeometry::hasM() const
{
    return ( mGeometry ?  mGeometry->hasM() : false );
}

QGis::GeometryType QgsGeometry::geometryType() const
{
    return ( mGeometry ? mGeometry->type() : QGis::NoGeometry );
}

QGis::WkbType QgsGeometry::wkbType()
{
    return ( mGeometry ? mGeometry->wkbType() : QGis::WKBNoGeometry );
}

QGis::GeometryType QgsGeometry::type()
{
    return ( mGeometry ? mGeometry->type() : QGis::NoGeometry );
}

QgsRectangle QgsGeometry::boundingBox()
{
    return ( mGeometry ? mGeometry->boundingBox() : QgsRectangle() );
}

bool QgsGeometry::isMultipart()
{
    return ( mGeometry ? mGeometry->isMultipart() : false );
}

bool QgsGeometry::convertToMultiType()
{
    return false; //todo...
}

int QgsGeometry::avoidIntersections( QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures )
{
    return ( mGeometry ? mGeometry->avoidIntersections( ignoreFeatures ) : 1 );
}

bool QgsGeometry::isGeosEqual( QgsGeometry& geom )
{
    return ( mGeometry ? mGeometry->isGeosEqual( geom ) : false );
}

bool QgsGeometry::isGeosValid()
{
    return ( mGeometry ? mGeometry->isGeosValid() : false );
}

bool QgsGeometry::isGeosEmpty()
{
   // return ( mGeometry ? mGeometry->isGeosEmpty() : false );
    return false;
}

double QgsGeometry::area()
{
    //return ( mGeometry ? mGeometry->area() : 0.0 );
    return 0.0;
}

double QgsGeometry::length()
{
    //return ( mGeometry ? mGeometry->length() : 0.0 );
    return 0.0;
}

double QgsGeometry::distance( QgsGeometry& geom )
{
    return ( mGeometry ? mGeometry->distance( geom ) : 0.0 );
}

QgsPoint QgsGeometry::closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist )
{
    return ( mGeometry ? mGeometry->closestVertex( point, atVertex, beforeVertex, afterVertex, sqrDist ) : QgsPoint() );
}

void QgsGeometry::adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex )
{
    if( mGeometry )
    {
        mGeometry->adjacentVertices( atVertex, beforeVertex, afterVertex );
    }
}

bool QgsGeometry::insertVertex( double x, double y, int beforeVertex )
{
    return ( mGeometry ? mGeometry->insertVertex( x, y, beforeVertex ) : false );
}

bool QgsGeometry::moveVertex( double x, double y, int atVertex )
{
    return ( mGeometry ? mGeometry->moveVertex( x, y, atVertex ) : false );
}

bool QgsGeometry::deleteVertex( int atVertex )
{
    return ( mGeometry ? mGeometry->deleteVertex( atVertex ) : false );
}

QgsPoint QgsGeometry::vertexAt( int atVertex )
{
    return ( mGeometry ? mGeometry->vertexAt( atVertex ) : QgsPoint() );
}

double QgsGeometry::sqrDistToVertexAt( QgsPoint& point, int atVertex )
{
    return ( mGeometry ? mGeometry->sqrDistToVertexAt( point, atVertex ) : 0.0 );
}

double QgsGeometry::closestVertexWithContext( const QgsPoint& point, int& atVertex )
{
    return ( mGeometry ? mGeometry->closestVertexWithContext( point, atVertex ) : 0.0 );
}

double QgsGeometry::closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& afterVertex, double* leftOf, double epsilon )
{
    return ( mGeometry ? mGeometry->closestSegmentWithContext( point, minDistPoint, afterVertex, leftOf, epsilon ) : 0.0 );
}

int QgsGeometry::addRing( const QList<QgsPoint>& ring )
{
    return ( mGeometry ? mGeometry->addRing( ring ) : 1 );
}

int QgsGeometry::addPart( const QList<QgsPoint> &points )
{
    return ( mGeometry ? mGeometry->addPart( points ) : 1 );
}

int QgsGeometry::translate( double dx, double dy )
{
    return ( mGeometry ? mGeometry->translate( dx, dy ) : 1 );
}

int QgsGeometry::transform( const QgsCoordinateTransform& ct )
{
    return 1; //( mGeometry ? mGeometry->transform( ct ) : 1 );
}

int QgsGeometry::splitGeometry( const QList<QgsPoint>& splitLine,
                   QList<QgsGeometry*>&newGeometries,
                   bool topological,
                   QList<QgsPoint> &topologyTestPoints )
{
    return ( mGeometry ? mGeometry->splitGeometry( splitLine, newGeometries, topological, topologyTestPoints ) : 1 );
}

int QgsGeometry::reshapeGeometry( const QList<QgsPoint>& reshapeWithLine )
{
    return ( mGeometry ? mGeometry->reshapeGeometry( reshapeWithLine ) : 1.0 );
}

int QgsGeometry::makeDifference( QgsGeometry* other )
{
    return ( mGeometry ? mGeometry->makeDifference( other ) : 1 );
}

bool QgsGeometry::intersects( const QgsRectangle& r )
{
    return ( mGeometry ? mGeometry->intersects( r ) : false );
}

bool QgsGeometry::intersects( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->intersects( geometry ) : false );
}

bool QgsGeometry::contains( QgsPoint* p )
{
    return ( mGeometry ? mGeometry->contains( p ) : false );
}

bool QgsGeometry::contains( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->contains( geometry ) : false );
}

bool QgsGeometry::disjoint( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->disjoint( geometry ) : true );
}

bool QgsGeometry::equals( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->equals( geometry ) : false );
}

bool QgsGeometry::touches( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->touches( geometry ) : false );
}

bool QgsGeometry::overlaps( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->overlaps( geometry ) : false );
}

bool QgsGeometry::within( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->within( geometry ) : false );
}

bool QgsGeometry::crosses( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->crosses( geometry ) : false );
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments )
{
    return ( mGeometry ? mGeometry->buffer( distance, segments ) : 0 );
}

QgsGeometry* QgsGeometry::simplify( double tolerance )
{
    return ( mGeometry ? mGeometry->simplify( tolerance ) : 0 );
}

QgsGeometry* QgsGeometry::centroid()
{
    return ( mGeometry ? mGeometry->centroid() : 0 );
}

QgsGeometry* QgsGeometry::convexHull()
{
    return ( mGeometry ? mGeometry->convexHull() : 0 );
}

QgsGeometry* QgsGeometry::interpolate( double distance )
{
    return 0; //( mGeometry ? mGeometry->interpolate( distance ) : 0 );
}


QgsGeometry* QgsGeometry::intersection( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->intersection( geometry ) : 0 );
}

QgsGeometry* QgsGeometry::combine( QgsGeometry* geometry )
{
    return ( mGeometry ? mGeometry->combine( geometry ) : 0 );
}

QgsGeometry* QgsGeometry::difference( QgsGeometry* geometry )
{
    return 0; //( mGeometry ? mGeometry->difference( geometry ) : 0 );
}

QgsGeometry* QgsGeometry::symDifference( QgsGeometry* geometry )
{
    return 0; //( mGeometry ? mGeometry->symDifference( geometry ) : 0 );
}

QString QgsGeometry::exportToWkt()
{
    return QString(); //return ( mGeometry ? mGeometry->exportToWkt() : QString() );
}

QString QgsGeometry::exportToGeoJSON()
{
    return QString(); //( mGeometry ? mGeometry->exportToGeoJSon() : QString() );
}

QDomElement QgsGeometry::exportToGML2( QDomDocument& doc )
{
    return QDomElement(); //( mGeometry ? mGeometry->exportToGML2( doc ) : QDomElement() );
}

QgsPoint QgsGeometry::asPoint()
{
    return ( mGeometry ? mGeometry->asPoint() : QgsPoint() );
}

QgsPolyline QgsGeometry::asPolyline()
{
    return ( mGeometry ? mGeometry->asPolyline() : QgsPolyline() );
}

QgsPolygon QgsGeometry::asPolygon()
{
    return ( mGeometry ? mGeometry->asPolygon() : QgsPolygon() );
}

QgsMultiPoint QgsGeometry::asMultiPoint()
{
    return ( mGeometry ? mGeometry->asMultiPoint() : QgsMultiPoint() );
}

QgsMultiPolyline QgsGeometry::asMultiPolyline()
{
    return ( mGeometry ? mGeometry->asMultiPolyline() : QgsMultiPolyline() );
}

QgsMultiPolygon QgsGeometry::asMultiPolygon()
{
    return ( mGeometry ? mGeometry->asMultiPolygon() : QgsMultiPolygon() );
}

QList<QgsGeometry*> QgsGeometry::asGeometryCollection()
{
    return QList<QgsGeometry*>(); //todo...
}

bool QgsGeometry::deleteRing( int ringNum, int partNum )
{
    return mGeometry ? mGeometry->deleteRing( ringNum, partNum ) : false;
}

bool QgsGeometry::deletePart( int partNum )
{
    return mGeometry ? mGeometry->deletePart( partNum ) : false;
}


/*************************abstract geometry****************************************/

QgsAbstractGeometry::QgsAbstractGeometry(): refs( 0 )
{

}

QgsAbstractGeometry::~QgsAbstractGeometry()
{

}

void QgsAbstractGeometry::ref()
{
    ++refs;
}

void QgsAbstractGeometry::deref()
{
    --refs;
    if( refs <= 0 )
    {
        delete this;
    }
}

QgsAbstractGeometry* QgsAbstractGeometry::fromWkb( unsigned char * wkb, size_t length )
{
  if ( length < 1 )
  {
    return 0;
  }

  int type;
  memcpy( &type, wkb + 1, sizeof( int ) );
  switch ( type )
  {
    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
      return QgsLineString::fromWkb( wkb );
    default:
      return 0;
  }
}

QgsAbstractGeometry* QgsAbstractGeometry::fromWkt( const QString& wkt )
{
  return 0;
}

QgsAbstractGeometry* QgsAbstractGeometry::fromGeos( const GEOSGeometry* geos )
{
  return 0;
}

QgsAbstractGeometry* QgsAbstractGeometry::fromGML2( const QDomNode& geometryNode )
{
  return 0;
}

QgsAbstractGeometry* QgsAbstractGeometry::fromPoint( const QgsPoint& point )
{
  return 0;
}

QgsAbstractGeometry* QgsAbstractGeometry::fromMultiPoint( const QgsMultiPoint& multipoint )
{
  return 0;
}

QgsAbstractGeometry* QgsAbstractGeometry::fromPolyline( const QgsPolyline& polyline )
{
    //create QgsLineString
    int nVertices = polyline.size();
    QVector<double>* xVector = new QVector<double>( nVertices );
    QVector<double>* yVector = new QVector<double>( nVertices );

    for( int i = 0; i < nVertices; ++i )
    {
        const QgsPoint& pt = polyline[i];
        (*xVector)[i] = pt.x();
        (*yVector)[i] = pt.y();
    }

    return new QgsLineString( xVector, yVector );
}

QgsAbstractGeometry* QgsAbstractGeometry::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  return 0;
}

/** construct geometry from a polygon */
QgsAbstractGeometry* QgsAbstractGeometry::fromPolygon( const QgsPolygon& polygon )
{
  return 0;
}

QgsAbstractGeometry* QgsAbstractGeometry::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  return 0;
}

QgsAbstractGeometry* QgsAbstractGeometry::fromRect( const QgsRectangle& rect )
{
  return 0;
}

/** Test for intersection with a rectangle (uses GEOS) */
bool QgsAbstractGeometry::intersects( const QgsRectangle& r )
{
  return false;
}

/** Test for intersection with a geometry (uses GEOS) */
bool QgsAbstractGeometry::intersects( QgsGeometry* geometry )
{
  return false;
}

/** Test for containment of a point (uses GEOS) */
bool QgsAbstractGeometry::contains( QgsPoint* p )
{
  return false;
}

/** Test for if geometry is contained in an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsAbstractGeometry::contains( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry is disjoint of an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsAbstractGeometry::disjoint( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry equals an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsAbstractGeometry::equals( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry touch an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsAbstractGeometry::touches( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry overlaps an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsAbstractGeometry::overlaps( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry is within an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsAbstractGeometry::within( QgsGeometry* geometry )
{
  return false;
}

/** Test for if geometry crosses an other (uses GEOS)
 *  @note added in 1.5 */
bool QgsAbstractGeometry::crosses( QgsGeometry* geometry )
{
  return false;
}

bool QgsAbstractGeometry::isGeosEqual( QgsGeometry & ) const
{
  return false;
}

bool QgsAbstractGeometry::isGeosValid() const
{
  return false;
}

QgsPoint QgsAbstractGeometry::closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist ) const
{
  return QgsPoint(); //soon...
}

QgsGeometry* QgsAbstractGeometry::buffer( double distance, int segments ) const
{
  return 0;
}

QgsGeometry* QgsAbstractGeometry::simplify( double tolerance ) const
{
  return 0;
}

QgsGeometry* QgsAbstractGeometry::centroid() const
{
  return 0;
}

QgsGeometry* QgsAbstractGeometry::convexHull() const
{
  return 0;
}

QgsGeometry* QgsAbstractGeometry::intersection( QgsGeometry* geometry ) const
{
  return 0;
}

QgsGeometry* QgsAbstractGeometry::combine( QgsGeometry* geometry ) const
{
  return 0;
}

QgsPoint QgsAbstractGeometry::asPoint()
{
  return QgsPoint(); //soon...
}

QgsPolyline QgsAbstractGeometry::asPolyline()
{
  return QgsPolyline(); //soon...
}

QgsPolygon QgsAbstractGeometry::asPolygon()
{
  return QgsPolygon(); //soon...
}

QgsMultiPoint QgsAbstractGeometry::asMultiPoint()
{
  return QgsMultiPoint(); //soon...
}

QgsMultiPolyline QgsAbstractGeometry::asMultiPolyline()
{
  return QgsMultiPolyline(); //soon...
}

QgsMultiPolygon QgsAbstractGeometry::asMultiPolygon()
{
  return QgsMultiPolygon(); //soon...
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

