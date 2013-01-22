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


GEOSGeometry* QgsGeometryUtils::createGeosLineString( QVector<double>* x, QVector<double>* y, QVector<double>* z, QVector<double>* m )
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
    //MH: for strange reasons, geos3 crashes when removing the coordinate sequence
    //if ( coord )
    //GEOSCoordSeq_destroy( coord );
    return 0;
  }
}

GEOSCoordSequence* QgsGeometryUtils::createGeosCoordSequence( QVector<double>* x, QVector<double>* y, QVector<double>* z, QVector<double>* m )
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
    coord = GEOSCoordSeq_create( nVertices, nDim );

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
