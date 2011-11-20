#include "qgslinestring.h"
#include "qgsapplication.h"
#include <QPolygonF>

QgsLineString::QgsLineString( QPolygonF* vertices, QVector<double>* z, QVector<double>* m ): mVertices( vertices ), mZValues( z ), mMValues( m )
{
}

QgsLineString::~QgsLineString()
{
  delete mVertices;
  delete mZValues;
  delete mMValues;
}

void QgsLineString::draw( QPainter* p ) const
{
}

void QgsLineString::coordinateTransform( const QgsCoordinateTransform& t )
{
}

void QgsLineString::pixelTransform( const QgsMapToPixel& mtp )
{
}

double QgsLineString::length() const
{
  return 0;
}

bool QgsLineString::isClosed() const
{
  return false;
}

//import
QgsGeometry* QgsLineString::fromWkb( unsigned char * wkb )
{
  unsigned char* ptr = wkb;
  ptr += 5; // size of endian

  unsigned int nPoints = *(( int* )ptr );
  ptr += 4;

  QPolygonF* polygon = new QPolygonF( nPoints );

  // Extract the points from the WKB format into the x and y vectors.
  double* x = 0;
  double* y = 0;
  for ( uint i = 0; i < nPoints; ++i )
  {
          x = ( double * )( ptr );
          ptr += sizeof( double );
          y = ( double * )( ptr );
          ptr += sizeof( double );
          (*polygon)[i] = QPointF( *x, *y );

          /*if (wkbType == QGis::WKBLineString25D)
                  ptr += sizeof( double );*/
  }
  return new QgsLineString( polygon );
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
  unsigned char* wkb = 0;

  if( !mVertices )
  {
    return wkb;
  }

  unsigned int numPoints = mVertices->size();

  // allocate space for the WKB
  int geometrySize = 	1 + // sizeof(byte)
                          sizeof(int) + // type
                          sizeof(int) + // numPoints
                          ((sizeof(double) +
                          sizeof(double)) * numPoints);

  wkb = new unsigned char[geometrySize];
  unsigned char* ptr = wkb;

  // set up byteOrder
  memset(ptr, QgsApplication::endian(), 1);
  ptr += 1;

  // assign wkbType
  int wkbType = QGis::WKBLineString;
  memcpy( ptr, &wkbType, sizeof(int) );
  ptr += sizeof(int);

  // assign numPoints
  memcpy( ptr, &numPoints, sizeof(int) );
  ptr += sizeof(int);

  double x, y;
  QPolygonF::const_iterator polyIt = mVertices->constBegin();
  for(; polyIt != mVertices->constEnd(); ++polyIt )
  {
    x = polyIt->x();
    memcpy( ptr, &x, sizeof( double ) );
    ptr += sizeof( double );
    y = polyIt->y();
    memcpy( ptr, &y, sizeof( double ) );
    ptr += sizeof( double );
  }

  return wkb;
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

QgsGeometry::GeometryType QgsLineString::geometryType() const
{
  return QgsGeometry::LineString;
}

QGis::WkbType QgsLineString::wkbType() const
{
  return QGis::WKBLineString;
}
QGis::GeometryType QgsLineString::type() const
{
  return QGis::Line;
}

QgsRectangle QgsLineString::boundingBox() const
{
  return QgsRectangle(); //soon...
}

bool QgsLineString::convertToMultiType() const
{
  return false;
}

//edit
bool QgsLineString::insertVertex( double x, double y, int beforeVertex )
{
  return false;
}

bool QgsLineString::moveVertex( double x, double y, int atVertex )
{
  return false;
}

bool QgsLineString::deleteVertex( int atVertex )
{
  return false;
}

bool QgsLineString::addRing( const QList<QgsPoint>& ring )
{
  return false;
}

bool QgsLineString::addPart( const QList<QgsPoint> &points )
{
  return false;
}

bool QgsLineString::splitGeometry( const QList<QgsPoint>& splitLine,
                            QList<QgsGeometry*>&newGeometries,
                            bool topological,
                            QList<QgsPoint> &topologyTestPoints )
{
  return false;
}

bool QgsLineString::reshapeGeometry( const QList<QgsPoint>& reshapeWithLine )
{
  return false;
}

bool QgsLineString::makeDifference( QgsGeometry* other )
{
  return false;
}

//analysis
QgsPoint QgsLineString::vertexAt( int atVertex ) const
{
  return QgsPoint(); //soon...
}

double QgsLineString::sqrDistToVertexAt( QgsPoint& point, int atVertex ) const
{
  return 0; //soon...
}

double QgsLineString::closestVertexWithContext( const QgsPoint& point, int& atVertex ) const
{
  return 0; //soon...
}

double QgsLineString::closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& beforeVertex ) const
{
  return 0; //soon...
}

void QgsLineString::adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex ) const
{
  return; //soon...
}

bool QgsLineString::deleteRing( int ringNum, int partNum ) const
{
  return false; //soon...
}

bool QgsLineString::deletePart( int partNum )
{
  return false; //soon...
}

double QgsLineString::distance( QgsGeometry& geom ) const
{
  return 0; //soon...
}

int QgsLineString::avoidIntersections() const
{
  return 0; //soon...
}


