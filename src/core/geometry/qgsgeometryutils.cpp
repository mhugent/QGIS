#include "qgsgeometryutils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include <QObject>
#include <QPolygon>
#include <QString>
#include <cstdio>

class GEOSException
{
  public:
    GEOSException( QString theMsg )
    {
      if ( theMsg == "Unknown exception thrown"  && lastMsg.isNull() )
      {
        msg = theMsg;
      }
      else
      {
        msg = theMsg;
        lastMsg = msg;
      }
    }

    // copy constructor
    GEOSException( const GEOSException &rhs )
    {
      *this = rhs;
    }

    ~GEOSException()
    {
      if ( lastMsg == msg )
        lastMsg = QString::null;
    }

    QString what()
    {
      return msg;
    }

  private:
    QString msg;
    static QString lastMsg;
};

QString GEOSException::lastMsg;

static void throwGEOSException( const char *fmt, ... )
{
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );

  QgsDebugMsg( QString( "GEOS exception: %1" ).arg( buffer ) );

  throw GEOSException( QString::fromUtf8( buffer ) );
}

static void printGEOSNotice( const char *fmt, ... )
{
#if defined(QGISDEBUG)
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );

  QgsDebugMsg( QString( "GEOS notice: %1" ).arg( QString::fromUtf8( buffer ) ) );
#else
  Q_UNUSED( fmt );
#endif
}

class GEOSInit
{
  public:
    GEOSInit()
    {
      initGEOS( printGEOSNotice, throwGEOSException );
    }

    ~GEOSInit()
    {
      finishGEOS();
    }
};

static GEOSInit geosinit;


GEOSGeometry* QgsGeometryUtils::createGeosLineString( const QVector<double>* x, const QVector<double>* y, const QVector<double>* z,
    const QVector<double>* m )
{
  GEOSCoordSequence *coord = 0;
  try
  {
    coord = createGeosCoordSequence( x, y, z, m );
    return GEOSGeom_createLineString( coord );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return 0;
  }
}

GEOSGeometry* QgsGeometryUtils::createGeosLinearRing( const QVector<double>* x, const QVector<double>* y, const QVector<double>* z,
    const QVector<double>* m )
{
  GEOSCoordSequence *coord = 0;
  try
  {
    coord = createGeosCoordSequence( x, y, z, m, true );
    return GEOSGeom_createLinearRing( coord );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return 0;
  }
}

GEOSGeometry* QgsGeometryUtils::createGeosPolygon( const QVector< QVector< double > >* ringsX, const QVector< QVector< double > >* ringsY,
    const QVector< QVector< double > >* ringsZ, const QVector< QVector< double > >* ringsM )
{
  if ( !ringsX || !ringsY )
  {
    return 0;
  }

  int size = qMin( ringsX->size(), ringsY->size() );
  if ( ringsZ )
  {
    size = qMin( size, ringsZ->size() );
  }
  if ( ringsM )
  {
    size = qMin( size, ringsM->size() );
  }

  if ( size < 1 )
  {
    return 0;
  }

  //outer ring
  GEOSGeometry* outerRing = createGeosLinearRing( &( ringsX->at( 0 ) ), &( ringsY->at( 0 ) ), ringsZ ? &( ringsZ->at( 0 ) ) : 0,
                            ringsM ? &( ringsM->at( 0 ) ) : 0 );

  GEOSGeometry** holes = 0;
  int nHoles = size - 1;
  if ( nHoles > 0 )
  {
    holes = new GEOSGeometry*[ nHoles];
  }

  for ( int i = 0; i < nHoles; ++i )
  {
    holes[i] = createGeosLinearRing( &( ringsX->at( i + 1 ) ), &( ringsY->at( i + 1 ) ), ringsZ ? &( ringsZ->at( i + 1 ) ) : 0,
                                     ringsM ? &( ringsM->at( i + 1 ) ) : 0 );
  }

  return GEOSGeom_createPolygon( outerRing, holes, nHoles );
}

GEOSCoordSequence* QgsGeometryUtils::createGeosCoordSequence( const QVector<double>* x, const QVector<double>* y,
    const QVector<double>* z, const QVector<double>* m, bool close )
{
  GEOSCoordSequence *coord = 0;
  try
  {
    int nDim = 2;
    int zIndex = -1;
    int mIndex = -1;
    if ( z )
    {
      zIndex = nDim;
      ++nDim;
    }
    if ( m )
    {
      mIndex = nDim;
      ++nDim;
    }

    int nVertices = x->size();
    int nCoordSeqSize = nVertices;
    if ( close )
    {
      ++nCoordSeqSize;
    }
    coord = GEOSCoordSeq_create( nCoordSeqSize, nDim );

    int i;
    for ( i = 0; i < nVertices; ++i )
    {
      GEOSCoordSeq_setX( coord, i, ( *x )[i] );
      GEOSCoordSeq_setY( coord, i, ( *y )[i] );
      if ( z )
      {
        GEOSCoordSeq_setOrdinate( coord, i, zIndex, ( *z )[i] );
      }
      if ( m )
      {
        GEOSCoordSeq_setOrdinate( coord, i, mIndex, ( *m )[i] );
      }
    }

    if ( close ) //close linear ring
    {
      GEOSCoordSeq_setX( coord, nCoordSeqSize - 1, ( *x )[0] );
      GEOSCoordSeq_setY( coord, nCoordSeqSize - 1, ( *y )[0] );
    }
    return coord;
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    throw;
  }
}

bool QgsGeometryUtils::createCoordinateVectors( const GEOSCoordSequence* seq, QVector<double>* x, QVector<double>* y, QVector<double>* z, QVector<double>* m )
{
  if ( !seq )
  {
    return false;
  }

  unsigned int nVertices = 0;
  if ( !GEOSCoordSeq_getSize( seq, &nVertices ) )
  {
    return false;
  }

  unsigned int nDimensions = 0;
  if ( !GEOSCoordSeq_getDimensions( seq, &nDimensions ) )
  {
    return false;
  }

  //todo: possibility to have m without z
  if ( !x || !y || !z || !m )
  {
    return false;
  }

  bool hasZ = nDimensions >= 3;
  bool hasM = nDimensions >= 4;

  x->resize( nVertices );
  y->resize( nVertices );
  z->resize( hasZ ? nVertices : 0 );
  m->resize( hasM ? nVertices : 0 );

  double xCoord = 0;
  double yCoord = 0;
  double zCoord = 0;
  double mCoord = 0;

  for ( unsigned int i = 0; i < nVertices; ++i )
  {
    GEOSCoordSeq_getX( seq, i, &xCoord );
    GEOSCoordSeq_getY( seq, i, &yCoord );
    ( *x )[i] = xCoord;
    ( *y )[i] = yCoord;
    if ( hasZ )
    {
      GEOSCoordSeq_getZ( seq, i, &zCoord );
      ( *z )[i] = zCoord;
    }
    if ( hasM )
    {
      GEOSCoordSeq_getOrdinate( seq, i, 3, &mCoord );
      ( *m )[i] = mCoord;
    }
  }
  return true;
}

QPolygonF QgsGeometryUtils::polygonFromCoordinates( const QVector<double>* x, const QVector<double>* y )
{
  int size = qMin( x->size(), y->size() );
  QPolygonF polygon( size );
  for ( int i = 0; i < size; ++i )
  {
    QPointF& pt = polygon[i];
    pt.rx() = ( *x )[i];
    pt.ry() = ( *y )[i];
  }
  return polygon;
}

QVector<QgsPoint> QgsGeometryUtils::pointVectorFromPolygon( const QPolygonF& polygon )
{
  int size = polygon.size();
  QVector<QgsPoint> pointVector( size );
  for ( int i = 0; i < size; ++i )
  {
    const QPointF& p = polygon[i];
    pointVector[i] = QgsPoint( p.x(), p.y() );
  }
  return pointVector;
}

QVector< QVector<QgsPoint> > QgsGeometryUtils::convertToRings( const QPolygonF& points, QList<QPolygonF>* rings )
{
  int nRings = 1;
  if ( rings )
  {
    nRings += rings->size();
  }

  QVector< QVector<QgsPoint> > polygon( nRings );
  polygon.append( QgsGeometryUtils::pointVectorFromPolygon( points ) );

  //rings
  if ( rings )
  {
    for ( int i = 0; i < rings->size(); ++i )
    {
      polygon.append( QgsGeometryUtils::pointVectorFromPolygon(( *rings )[i] ) );
    }
  }
  return polygon;
}

double QgsGeometryUtils::leftOf( const double& x, const double& y, const double& x1, const double& y1, const double& x2, const double& y2 )
{
  double f1 = x - x1;
  double f2 = y2 - y1;
  double f3 = y - y1;
  double f4 = x2 - x1;
  return f1*f2 - f3*f4;
}
