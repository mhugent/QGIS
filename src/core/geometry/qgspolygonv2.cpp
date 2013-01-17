#include "qgspolygonv2.h"

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
  //todo...
}

void QgsPolygonV2::addToPainterPath( QPainterPath& path ) const
{
  //todo...
}

void QgsPolygonV2::coordinateTransform( const QgsCoordinateTransform& t )
{
  //todo...
}

void QgsPolygonV2::pixelTransform( const QgsMapToPixel& mtp )
{
  //todo...
}

int QgsPolygonV2::translate( double dx, double dy )
{
  return 1;
}

double QgsPolygonV2::area() const
{
  return 0;
}

QgsAbstractGeometry* QgsPolygonV2::fromWkb( unsigned char * wkb )
{
  return 0;
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
  return 0;
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
