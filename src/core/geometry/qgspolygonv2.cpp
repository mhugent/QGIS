#include "qgspolygonv2.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometryutils.h"
#include "qgsmaptopixel.h"
#include <QPainter>

QgsPolygonV2::QgsPolygonV2( QVector< QVector< double > >* ringsX, QVector< QVector< double > >* ringsY,
                            QVector< QVector< double > >* ringsZ, QVector< QVector< double > >* ringsM )
    : mRingsX( ringsX ), mRingsY( ringsY ), mRingsZ( ringsZ ), mRingsM( ringsM )
{

}

QgsPolygonV2::~QgsPolygonV2()
{
  delete mRingsX;
  delete mRingsY;
  delete mRingsZ;
  delete mRingsM;
}

void QgsPolygonV2::draw( QPainter* p ) const
{
  int nRings = ringCount();
  if ( nRings < 1 )
  {
    return;
  }

  if ( nRings == 1 )
  {
    QPolygonF poly = QgsGeometryUtils::polygonFromCoordinates( &( *mRingsX )[0], &( *mRingsY )[0] );
    p->drawPolygon( poly );
  }
  else //use painterpath
  {
    QPainterPath path;
    addToPainterPath( path );
    p->drawPath( path );
  }
}

void QgsPolygonV2::drawVertexMarkers( QPainter* p, QgsGeometry::VertexMarkerType type, int size ) const
{
  int nRings = ringCount();
  for ( int i = 0; i < nRings; ++i )
  {
    const QVector< double >& xVector = mRingsX->at( i );
    const QVector< double >& yVector = mRingsY->at( i );
    QVector< double >::const_iterator xIt = xVector.constBegin();
    QVector< double >::const_iterator yIt = yVector.constBegin();

    for ( ; xIt != xVector.constEnd() && yIt != yVector.constEnd(); ++xIt, ++yIt )
    {
      drawVertexMarker( *xIt, *yIt, p, type, size );
    }
  }
}

void QgsPolygonV2::addToPainterPath( QPainterPath& path ) const
{
  int nRings = ringCount();
  for ( int i = 0; i < nRings; ++i )
  {
    path.addPolygon( QgsGeometryUtils::polygonFromCoordinates( &( *mRingsX )[i], &( *mRingsY )[i] ) );
  }
}

void QgsPolygonV2::coordinateTransform( const QgsCoordinateTransform& t )
{
  int nRings = ringCount();
  int nPoints;
  for ( int i = 0; i < nRings; ++i )
  {
    nPoints = ( *mRingsX )[i].size();
    QVector<double> z( nPoints, 0.0 );
    t.transformInPlace(( *mRingsX )[i], ( *mRingsY )[i], z );
  }
}

void QgsPolygonV2::pixelTransform( const QgsMapToPixel& mtp )
{
  int nRings = ringCount();
  for ( int i = 0; i < nRings; ++i )
  {
    mtp.transformInPlace(( *mRingsX )[i], ( *mRingsY )[i] );
  }
}

int QgsPolygonV2::translate( double dx, double dy )
{
  return 1;
}

double QgsPolygonV2::area() const
{
  return 0;
}

int QgsPolygonV2::ringCount() const
{
  if ( !mRingsX || !mRingsY )
  {
    return 0;
  }

  return qMin( mRingsX->size(), mRingsY->size() );
}

QgsAbstractGeometry* QgsPolygonV2::fromWkb( unsigned char * wkb )
{
  unsigned char* geometry = wkb;
  geometry += 1; //skip endian

  int type = *(( int* )geometry );
  bool hasZ = false;
  bool hasM = false;
  //0003  1003  2003  3003
  if ( type == 1003 || type == 3003 )
  {
    hasZ = true;
  }
  if ( type == 2003 || type == 3003 )
  {
    hasM = true;
  }
  geometry += sizeof( int );

  int* nRings = ( int* )geometry;
  geometry += sizeof( int );

  QVector< QVector< double > >* ringsX = new QVector< QVector< double > >( *nRings );
  QVector< QVector< double > >* ringsY = new QVector< QVector< double > >( *nRings );
  QVector< QVector< double > >* ringsZ = 0;
  if ( hasZ )
  {
    ringsZ = new QVector< QVector< double > >( *nRings );
  }
  QVector< QVector< double > >* ringsM = 0;
  if ( hasM )
  {
    ringsM = new QVector< QVector< double > >( *nRings );
  }

  int* nPoints = 0;
  double* x = 0;
  double* y = 0;
  double* z = 0;
  double* m = 0;

  for ( int i = 0; i < *nRings; ++i )
  {
    nPoints = ( int* )geometry;
    ( *ringsX )[i] = QVector< double >( *nPoints );
    ( *ringsY )[i] = QVector< double >( *nPoints );
    if ( hasZ )
    {
      ( *ringsZ )[i] = QVector< double >( *nPoints );
    }
    if ( hasM )
    {
      ( *ringsM )[i] = QVector< double >( *nPoints );
    }


    geometry += sizeof( int );
    for ( int j = 0; j < *nPoints; ++j )
    {
      x = ( double* )geometry;
      ( *ringsX )[i][j] = *x;
      geometry += sizeof( double );
      y = ( double* )geometry;
      ( *ringsY )[i][j] = *y;
      geometry += sizeof( double );
      if ( hasZ )
      {
        z = ( double* )geometry;
        ( *ringsZ )[i][j] = *y;
        geometry += sizeof( double );
      }
      if ( hasM )
      {
        m = ( double* )geometry;
        ( *ringsM )[i][j] = *y;
        geometry += sizeof( double );
      }
    }
  }

  return new QgsPolygonV2( ringsX, ringsY, ringsM, ringsZ );
}

QgsAbstractGeometry* QgsPolygonV2::fromWkt( const QString& wkt )
{
  return 0;
}

QgsAbstractGeometry* QgsPolygonV2::fromGeos( const GEOSGeometry* geos )
{
  return 0;
}

//export
unsigned char* QgsPolygonV2::asWkb( int& wkbSize ) const
{
  return 0;
}

GEOSGeometry* QgsPolygonV2::asGeos() const
{
  return QgsGeometryUtils::createGeosPolygon( mRingsX, mRingsY, mRingsZ, mRingsM );
}

QString QgsPolygonV2::asWkt() const
{
  return QString();
}

QDomElement QgsPolygonV2::asGML2( QDomDocument& doc ) const
{
  return QDomElement();
}

QgsGeometry* QgsPolygonV2::clone() const
{
  return 0;
}

QgsGeometry::GeometryType QgsPolygonV2::geometryType() const
{
  return QgsGeometry::Polygon;
}

QGis::WkbType QgsPolygonV2::wkbType() const
{
  return QGis::WKBPolygon;
}

QGis::GeometryType QgsPolygonV2::type() const
{
  return QGis::Polygon;
}

QgsRectangle QgsPolygonV2::boundingBox() const
{
  return QgsRectangle();
}

bool QgsPolygonV2::convertToMultiType() const
{
  return false;
}

//edit
bool QgsPolygonV2::insertVertex( double x, double y, int beforeVertex ) //m and z?
{
  return false;
}

bool QgsPolygonV2::moveVertex( double x, double y, int atVertex ) //keep m and z
{
  return false;
}

bool QgsPolygonV2::deleteVertex( int atVertex )
{
  return false;
}

bool QgsPolygonV2::addRing( const QList<QgsPoint>& ring )
{
  return false;
}

bool QgsPolygonV2::addPart( const QList<QgsPoint> &points )
{
  return false;
}

bool QgsPolygonV2::splitGeometry( const QList<QgsPoint>& splitLine,
                                  QList<QgsGeometry*>&newGeometries,
                                  bool topological,
                                  QList<QgsPoint> &topologyTestPoints )
{
  return false;
}

bool QgsPolygonV2::reshapeGeometry( const QList<QgsPoint>& reshapeWithLine )
{
  return false;
}

bool QgsPolygonV2::makeDifference( QgsGeometry* other )
{
  return false;
}

//analysis
QgsPoint QgsPolygonV2::vertexAt( int atVertex ) const //return m and z
{
  return QgsPoint();
}

double QgsPolygonV2::sqrDistToVertexAt( QgsPoint& point, int atVertex ) const
{
  return -1;
}

double QgsPolygonV2::closestVertexWithContext( const QgsPoint& point, int& atVertex ) const
{
  return -1;
}

double QgsPolygonV2::closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& afterVertex,
    double* leftOf, double epsilon ) const
{
  return -1;
}

void QgsPolygonV2::adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex ) const
{
  //todo...
}

bool QgsPolygonV2::deleteRing( int ringNum, int partNum ) const
{
  return false;
}

bool QgsPolygonV2::deletePart( int partNum )
{
  return false;
}

double QgsPolygonV2::distance( QgsGeometry& geom ) const
{
  return -1;
}

int QgsPolygonV2::avoidIntersections( QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures ) const
{
  return 1;
}
