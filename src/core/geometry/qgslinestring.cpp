#include "qgslinestring.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometryutils.h"
#include "qgsmaptopixel.h"
#include <QPainter>
#include <QVector>
#include <limits>

QgsLineString::QgsLineString( QVector<double>* x, QVector<double>* y, QVector<double>* z, QVector<double>* m ):
    mXValues( x ), mYValues( y ), mZValues( z ), mMValues( m )
{
}

QgsLineString::~QgsLineString()
{
  delete mXValues;
  delete mYValues;
  delete mZValues;
  delete mMValues;
}

void QgsLineString::draw( QPainter* p, const QgsMapToPixel* mtp ) const
{
  QPolygonF poly = QgsGeometryUtils::polygonFromCoordinates( mXValues, mYValues );
  if ( mtp )
  {
    mtp->transformInPlace( poly );
  }
  p->drawPolyline( poly );
}

void QgsLineString::addToPainterPath( QPainterPath& path, const QgsMapToPixel* mtp ) const
{
  QPolygonF poly = QgsGeometryUtils::polygonFromCoordinates( mXValues, mYValues );
  if ( mtp )
  {
    mtp->transformInPlace( poly );
  }
  path.addPolygon( poly );
}

void QgsLineString::coordinateTransform( const QgsCoordinateTransform& t )
{
  int nPoints = mXValues->size();
  QVector<double> z( nPoints, 0.0 );
  t.transformInPlace( *mXValues, *mYValues, z );
}

void QgsLineString::pixelTransform( const QgsMapToPixel& mtp )
{
  mtp.transformInPlace( *mXValues, *mYValues );
}

int QgsLineString::vertexCount() const
{
  if ( !mXValues || !mYValues )
  {
    return 0;
  }

  return qMin( mXValues->size(), mYValues->size() );
}

int QgsLineString::translate( double dx, double dy )
{
  //soon...
  return 1;
}

void QgsLineString::vertices( QList<QgsPoint>& vertexList ) const
{
  int nVertices = vertexCount();
  for ( int i = 0; i < nVertices; ++i )
  {
    vertexList.append( QgsPoint( mXValues->at( i ), mYValues->at( i ) ) );
  }
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
QgsAbstractGeometry* QgsLineString::fromWkb( unsigned char * wkb )
{
  unsigned char* ptr = wkb;
  ptr += 5; // size of endian

  unsigned int nPoints = *(( int* )ptr );
  ptr += 4;

  QVector<double>* xVector = new QVector<double>( nPoints );
  QVector<double>* yVector = new QVector<double>( nPoints );

  // Extract the points from the WKB format into the x and y vectors.
  double* x = 0;
  double* y = 0;
  for ( uint i = 0; i < nPoints; ++i )
  {
    ( *xVector )[i] = *( double * )( ptr );
    ptr += sizeof( double );
    ( *yVector )[i] = *( double * )( ptr );
    ptr += sizeof( double );
    /*if (wkbType == QGis::WKBLineString25D)
            ptr += sizeof( double );*/
  }
  return new QgsLineString( xVector, yVector );
}

QgsAbstractGeometry* QgsLineString::fromWkt( const QString& wkt )
{
  return 0;
}

QgsAbstractGeometry* QgsLineString::fromGeos( const GEOSGeometry* geos )
{
  if ( !geos || !GEOSGeomTypeId( geos ) == 1 )
  {
    return 0;
  }

  const GEOSCoordSequence* seq = GEOSGeom_getCoordSeq( geos );
  if ( !seq )
  {
    return 0;
  }

  QVector<double>* xVector = new QVector<double>();
  QVector<double>* yVector = new QVector<double>();
  QVector<double>* zVector = new QVector<double>();
  QVector<double>* mVector = new QVector<double>();
  if ( !QgsGeometryUtils::createCoordinateVectors( seq, xVector, yVector, zVector, mVector ) )
  {
    return 0;
  }

  return new QgsLineString( xVector, yVector, zVector->size() > 0 ? zVector : 0, mVector->size() > 0 ? mVector : 0 );
}

QgsPolyline QgsLineString::asPolyline() const
{
  QgsPolyline line;
  int nVertices = vertexCount();
  for ( int i = 0; i < nVertices; ++i )
  {
    line.append( QgsPoint( mXValues->at( i ), mYValues->at( i ) ) );
  }
  return line;
}

//export
unsigned char* QgsLineString::asWkb( int& wkbSize ) const
{
  wkbSize = 0;
  unsigned char* wkb = 0;

  if ( !mXValues )
  {
    return wkb;
  }

  unsigned int numPoints = mXValues->size();

  // allocate space for the WKB
  int geometrySize =  1 + // sizeof(byte)
                      sizeof( int ) + // type
                      sizeof( int ) + // numPoints
                      (( sizeof( double ) +
                         sizeof( double ) ) * numPoints );

  wkb = new unsigned char[geometrySize];
  unsigned char* ptr = wkb;

  // set up byteOrder
  memset( ptr, QgsApplication::endian(), 1 );
  ptr += 1;

  // assign wkbType
  int wkbType = QGis::WKBLineString;
  memcpy( ptr, &wkbType, sizeof( int ) );
  ptr += sizeof( int );

  // assign numPoints
  memcpy( ptr, &numPoints, sizeof( int ) );
  ptr += sizeof( int );

  double x, y;
  QVector<double>::const_iterator xIt = mXValues->constBegin();
  QVector<double>::const_iterator yIt = mYValues->constBegin();
  for ( ; xIt != mXValues->constEnd(); ++xIt, ++yIt )
  {
    x = *xIt;
    memcpy( ptr, &x, sizeof( double ) );
    ptr += sizeof( double );
    y = *yIt;
    memcpy( ptr, &y, sizeof( double ) );
    ptr += sizeof( double );
  }

  return wkb;
}

GEOSGeometry* QgsLineString::asGeos() const
{
  return QgsGeometryUtils::createGeosLineString( mXValues, mYValues, mZValues );
}

QString QgsLineString::asWkt() const
{
  return "";
}

QDomElement QgsLineString::asGML2( QDomDocument& doc ) const
{
  return QDomElement();
}

QgsAbstractGeometry* QgsLineString::clone() const
{
  QVector<double>* xVector = new QVector<double>( *mXValues );
  QVector<double>* yVector = new QVector<double>( *mYValues );
  QVector<double>* zVector = mZValues ? new QVector<double>( *mZValues ) : 0;
  QVector<double>* mVector = mMValues ? new QVector<double>( *mMValues ) : 0;
  return new QgsLineString( xVector, yVector, zVector );
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
  if ( !mXValues || !mYValues )
  {
    return false;
  }
  if ( atVertex < 0 || atVertex >= mXValues->size() || atVertex  >= mYValues->size() )
  {
    return false; //out ot range
  }

  ( *mXValues )[ atVertex ] = x;
  ( *mYValues )[ atVertex ] = y;

  updateGeos(); //keep cached geos object in sync if present
  return true;
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
  int nVertices = vertexCount();
  if ( atVertex < 0 && atVertex >= nVertices )
  {
    return QgsPoint();
  }

  return QgsPoint( mXValues->at( atVertex ), mYValues->at( atVertex ) );
}

double QgsLineString::sqrDistToVertexAt( QgsPoint& point, int atVertex ) const
{
  return 0; //soon...
}

double QgsLineString::closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& afterVertex,
    double* leftOf, double epsilon ) const
{
  int nVertices = vertexCount();
  if ( nVertices < 1 )
  {
    return -1;
  }

  double sqrDist = std::numeric_limits<double>::max();
  double currentDist = 0;
  QgsPoint distPoint;
  double thisX = 0;
  double thisY = 0;
  double prevX = 0;
  double prevY = 0;

  for ( int i = 0; i < nVertices; ++i )
  {
    if ( i > 0 )
    {
      prevX = thisX;
      prevY = thisY;
    }
    thisX = mXValues->at( i );
    thisY = mYValues->at( i );

    if (( currentDist = point.sqrDistToSegment( prevX, prevY, thisX, thisY, distPoint, epsilon ) ) < sqrDist )
    {
      afterVertex = i;
      sqrDist = currentDist;
      minDistPoint = distPoint;
      if ( leftOf )
      {
        *leftOf = QgsGeometryUtils::leftOf( point.x(), point.y(), prevX, prevY, thisX, thisY );
      }
    }
  }

  return sqrDist;
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

int QgsLineString::avoidIntersections( QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures ) const
{
  return 0; //soon...
}


