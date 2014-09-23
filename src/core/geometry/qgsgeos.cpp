/***************************************************************************
                        qgsgeos.cpp
  -------------------------------------------------------------------
Date                 : 22 Sept 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeos.h"
#include "qgsabstractgeometryv2.h"
#include "qgsgeometryimport.h"
#include "qgslinestringv2.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include <cstdio>

#define CATCH_GEOS(r) \
  catch (GEOSException &e) \
  { \
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr("GEOS") ); \
    return r; \
  }

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

QgsGeos::QgsGeos( QgsAbstractGeometryV2* geometry ): QgsVectorTopology( geometry ), mGeos( 0 ), mGeosPrepared( 0 )
{

}

QgsGeos::~QgsGeos()
{
  geometryChanged(); //remove caches
}

void QgsGeos::geometryChanged()
{
  GEOSGeom_destroy( mGeos );
  mGeos = 0;
  GEOSPreparedGeom_destroy( mGeosPrepared );
  mGeosPrepared = 0;
}

void QgsGeos::prepareGeometry()
{
  GEOSPreparedGeom_destroy( mGeosPrepared );
  mGeosPrepared = 0;
  cacheGeos();
  if ( mGeos )
  {
    mGeosPrepared = GEOSPrepare( mGeos );
  }
}

void QgsGeos::cacheGeos() const
{
  if ( !mGeometry || mGeos )
  {
    return;
  }

  mGeos = asGeos( mGeometry );
}

QgsAbstractGeometryV2* QgsGeos::intersection( const QgsAbstractGeometryV2& geom ) const
{
  return overlay( geom, INTERSECTION );
}

QgsAbstractGeometryV2* QgsGeos::difference( const QgsAbstractGeometryV2& geom ) const
{
  return overlay( geom, DIFFERENCE );
}

QgsAbstractGeometryV2* QgsGeos::combine( const QgsAbstractGeometryV2& geom ) const
{
  return overlay( geom, UNION );
}

QgsAbstractGeometryV2* QgsGeos::symDifference( const QgsAbstractGeometryV2& geom ) const
{
  return overlay( geom, SYMDIFFERENCE );
}

double QgsGeos::distance( const QgsAbstractGeometryV2& geom ) const
{
  return 0.0;
}

bool QgsGeos::instersects( const QgsAbstractGeometryV2& geom ) const
{
  return relation( geom, INTERSECTS );
}

bool QgsGeos::touches( const QgsAbstractGeometryV2& geom ) const
{
  return relation( geom, TOUCHES );
}

bool QgsGeos::crosses( const QgsAbstractGeometryV2& geom ) const
{
  return relation( geom, CROSSES );
}

bool QgsGeos::within( const QgsAbstractGeometryV2& geom ) const
{
  return relation( geom, WITHIN );
}

bool QgsGeos::overlaps( const QgsAbstractGeometryV2& geom ) const
{
  return relation( geom, OVERLAPS );
}

QgsAbstractGeometryV2* QgsGeos::fromGeos( const GEOSGeometry* geos )
{
  if ( !geos )
  {
    return 0;
  }

  int nCoordDims = GEOSGeom_getCoordinateDimension( geos );
  int nDims = GEOSGeom_getDimensions( geos );
  bool hasZ = ( nCoordDims == 3 );
  bool hasM = (( nDims - nCoordDims ) == 1 );

  switch ( GEOSGeomTypeId( geos ) )
  {
    case GEOS_POINT:                 // a point
    {
      const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq( geos );
      return ( coordSeqPoint( cs, 0, hasZ, hasM ).clone() );
    }
    case GEOS_LINESTRING:
    {
      const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq( geos );
      unsigned int nPoints;
      QList<QgsPointV2> points;
      GEOSCoordSeq_getSize( cs, &nPoints );
      for ( unsigned int i = 0; i < nPoints; ++i )
      {
        points.push_back( coordSeqPoint( cs, i, hasZ, hasM ) );
      }
      QgsLineStringV2* line = new QgsLineStringV2();
      line->setPoints( points );
      return line;
    }
  }
  return 0;
}

QgsPointV2 QgsGeos::coordSeqPoint( const GEOSCoordSequence* cs, int i, bool hasZ, bool hasM )
{
  if ( !cs )
  {
    return QgsPointV2();
  }

  double x, y, z, m;
  GEOSCoordSeq_getX( cs, i, &x );
  GEOSCoordSeq_getY( cs, i, &y );
  if ( hasZ )
  {
    GEOSCoordSeq_getZ( cs, i, &z );
  }
  if ( hasM )
  {
    GEOSCoordSeq_getOrdinate( cs, i, 3, &m );
  }

  if ( hasZ && hasM )
  {
    return QgsPointV2( x, y, z, m );
  }
  else if ( hasZ )
  {
    return QgsPointV2( x, y, z, false );
  }
  else if ( hasM )
  {
    return QgsPointV2( x, y, m, true );
  }
  return QgsPointV2( x, y );
}

GEOSGeometry* QgsGeos::asGeos( const QgsAbstractGeometryV2* geom )
{
  int coordDims = 2;
  if ( geom->is3D() )
  {
    ++coordDims;
  }
  if ( geom->isMeasure() )
  {
    ++coordDims;
  }

  bool hasZ = geom->is3D();
  bool hasM = geom->isMeasure();

  //point
  if ( geom->geometryType() == "Point" )
  {
    const QgsPointV2* point = dynamic_cast<const QgsPointV2*>( geom );
    if ( point )
    {
      GEOSCoordSequence* coordSeq = GEOSCoordSeq_create( 1, coordDims );
      GEOSCoordSeq_setX( coordSeq, 0, point->x() );
      GEOSCoordSeq_setY( coordSeq, 0, point->y() );
      if ( hasZ )
      {
        GEOSCoordSeq_setOrdinate( coordSeq, 0, 2, point->z() );
      }
      if ( hasM )
      {
        GEOSCoordSeq_setOrdinate( coordSeq, 0, 3, point->m() );
      }
      return GEOSGeom_createPoint( coordSeq );
    }
  }

  //curve
  const QgsCurveV2* curve = dynamic_cast<const QgsCurveV2*>( geom );
  if ( curve )
  {
    QgsLineStringV2* line = curve->curveToLine();
    int numPoints = line->numPoints();
    GEOSCoordSequence* coordSeq = GEOSCoordSeq_create( numPoints, coordDims );
    for ( int i = 0; i < numPoints; ++i )
    {
      QgsPointV2 pt = line->pointN( i ); //todo: create method to get const point reference
      GEOSCoordSeq_setX( coordSeq, i, pt.x() );
      GEOSCoordSeq_setY( coordSeq, i, pt.y() );
      if ( hasZ )
      {
        GEOSCoordSeq_setOrdinate( coordSeq, i, 2, pt.z() );
      }
      if ( hasM )
      {
        GEOSCoordSeq_setOrdinate( coordSeq, i, 3, pt.m() );
      }
    }
    delete line;
    return GEOSGeom_createLineString( coordSeq );
  }

  //todo: surface, multitypes

  return 0;
}

QgsAbstractGeometryV2* QgsGeos::overlay( const QgsAbstractGeometryV2& geom, Overlay op ) const
{
  cacheGeos();
  if ( !mGeos )
  {
    return 0;
  }

  GEOSGeometry* geosGeom = asGeos( &geom );
  if ( !geosGeom )
  {
    return 0;
  }

  try
  {
    GEOSGeometry* opGeom = 0;
    switch ( op )
    {
      case INTERSECTION:
        opGeom = GEOSIntersection( mGeos, geosGeom );
        break;
      case DIFFERENCE:
        opGeom = GEOSDifference( mGeos, geosGeom );
        break;
      case UNION:
        opGeom = GEOSUnion( mGeos, geosGeom );
        break;
      case SYMDIFFERENCE:
        opGeom = GEOSSymDifference( mGeos, geosGeom );
        break;
      default:    //unknown op
        GEOSGeom_destroy( geosGeom );
        return 0;
    }
    QgsAbstractGeometryV2* opResult = fromGeos( opGeom );
    GEOSGeom_destroy( opGeom );
    GEOSGeom_destroy( geosGeom );
    return opResult;
  }
  catch ( GEOSException &e )
  {
    GEOSGeom_destroy( geosGeom );
    return 0;
  }
}

bool QgsGeos::relation( const QgsAbstractGeometryV2& geom, Relation r ) const
{
  cacheGeos();
  if ( !mGeos )
  {
    return false;
  }

  GEOSGeometry* geosGeom = asGeos( &geom );
  if ( !geosGeom )
  {
    return false;
  }

  bool result = false;

  try
  {
    if ( mGeosPrepared ) //use faster version with prepared geometry
    {
      switch ( r )
      {
        case INTERSECTS:
          result = ( GEOSPreparedIntersects( mGeosPrepared, geosGeom ) == 0 );
          break;
        case TOUCHES:
          result = ( GEOSPreparedTouches( mGeosPrepared, geosGeom ) == 0 );
          break;
        case CROSSES:
          result = ( GEOSPreparedCrosses( mGeosPrepared, geosGeom ) == 0 );
          break;
        case WITHIN:
          result = ( GEOSPreparedWithin( mGeosPrepared, geosGeom ) == 0 );
          break;
        default:
          GEOSGeom_destroy( geosGeom );
          return false;
      }
      return result;
    }

    switch ( r )
    {
      case INTERSECTS:
        result = ( GEOSIntersects( mGeos, geosGeom ) == 0 );
        break;
      case TOUCHES:
        result = ( GEOSTouches( mGeos, geosGeom ) == 0 );
        break;
      case CROSSES:
        result = ( GEOSCrosses( mGeos, geosGeom ) == 0 );
        break;
      case WITHIN:
        result = ( GEOSWithin( mGeos, geosGeom ) == 0 );
        break;
      default:
        GEOSGeom_destroy( geosGeom );
        return false;
    }
  }
  catch ( GEOSException &e )
  {
    GEOSGeom_destroy( geosGeom );
    return 0;
  }

  return false;
}
