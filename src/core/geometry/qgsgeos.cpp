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
#include "qgsgeometrycollectionv2.h"
#include "qgsgeometryimport.h"
#include "qgslinestringv2.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgspolygonv2.h"
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

QgsGeos::QgsGeos( const QgsAbstractGeometryV2* geometry ): QgsGeometryEngine( geometry ), mGeos( 0 ), mGeosPrepared( 0 )
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

bool QgsGeos::intersects( const QgsAbstractGeometryV2& geom ) const
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

bool QgsGeos::contains( const QgsAbstractGeometryV2& geom ) const
{
  return relation( geom, CONTAINS );
}

bool QgsGeos::disjoint( const QgsAbstractGeometryV2& geom ) const
{
  return relation( geom, DISJOINT );
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
      return sequenceToLinestring( geos, hasZ, hasM );
    }
    case GEOS_POLYGON:

      QgsPolygonV2* polygon = new QgsPolygonV2();

      const GEOSGeometry* ring = GEOSGetExteriorRing( geos );
      if ( ring )
      {
        polygon->setExteriorRing( sequenceToLinestring( ring, hasZ, hasM ) );
      }

      QList<QgsCurveV2*> interiorRings;
      for ( int i = 0; i < GEOSGetNumInteriorRings( geos ); ++i )
      {
        ring = GEOSGetInteriorRingN( geos, i );
        if ( ring )
        {
          interiorRings.push_back( sequenceToLinestring( ring, hasZ, hasM ) );
        }
      }
      polygon->setInteriorRings( interiorRings );

      return polygon;
  }
  return 0;
}

QgsLineStringV2* QgsGeos::sequenceToLinestring( const GEOSGeometry* geos, bool hasZ, bool hasM )
{
  QList<QgsPointV2> pts;
  const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq( geos );
  unsigned int nPoints;
  GEOSCoordSeq_getSize( cs, &nPoints );
  for ( unsigned int i = 0; i < nPoints; ++i )
  {
    pts.push_back( coordSeqPoint( cs, i, hasZ, hasM ) );
  }
  QgsLineStringV2* line = new QgsLineStringV2();
  line->setPoints( pts );
  return line;
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

  //curve
  const QgsCurveV2* curve = dynamic_cast< const QgsCurveV2* >( geom );
  const QgsCurvePolygonV2* curvePolygon = dynamic_cast< const QgsCurvePolygonV2* >( geom );
  if ( curve )
  {
    QScopedPointer< QgsLineStringV2>  lineString( curve->curveToLine() );
    return createGeosLinestring( lineString.data() );
  }
  else if ( curvePolygon )
  {
    QScopedPointer<QgsPolygonV2> polygon( curvePolygon->toPolygon() );
    return createGeosPolygon( polygon.data() );
  }
  else if ( geom->geometryType() == "Point" )
  {
    return createGeosPoint( geom, coordDims );
  }
  else if ( geom->geometryType() == "MultiPoint" )
  {
    const QgsGeometryCollectionV2* c = dynamic_cast<const QgsGeometryCollectionV2*>( geom );
    if ( !c )
    {
      return 0;
    }

    GEOSGeometry **geomarr = new GEOSGeometry*[ c->numGeometries()];
    for ( int i = 0; i < c->numGeometries(); ++i )
    {
      geomarr[i] = createGeosPoint( c->geometryN( i ), coordDims );
    }
    return GEOSGeom_createCollection( GEOS_MULTIPOINT, geomarr, c->numGeometries() ); //todo: geos exceptions
  }
  else if ( geom->geometryType() == "MultiCurve" )
  {
    const QgsGeometryCollectionV2* c = dynamic_cast<const QgsGeometryCollectionV2*>( geom );
    if ( !c )
    {
      return 0;
    }

    GEOSGeometry **geomarr = new GEOSGeometry*[ c->numGeometries()];
    for ( int i = 0; i < c->numGeometries(); ++i )
    {
      geomarr[i] = createGeosLinestring( c->geometryN( i ) );
    }
    return GEOSGeom_createCollection( GEOS_MULTILINESTRING, geomarr, c->numGeometries() ); //todo: geos exceptions
  }
  else if ( geom->geometryType() == "MultiSurface" )
  {
    const QgsGeometryCollectionV2* c = dynamic_cast<const QgsGeometryCollectionV2*>( geom );
    if ( !c )
    {
      return 0;
    }

    GEOSGeometry **geomarr = new GEOSGeometry*[ c->numGeometries()];
    for ( int i = 0; i < c->numGeometries(); ++i )
    {
      geomarr[i] = createGeosPolygon( c->geometryN( i ) );
    }
    return GEOSGeom_createCollection( GEOS_MULTIPOLYGON, geomarr, c->numGeometries() ); //todo: geos exceptions
  }

  //todo: multitypes

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
          result = ( GEOSPreparedIntersects( mGeosPrepared, geosGeom ) == 1 );
          break;
        case TOUCHES:
          result = ( GEOSPreparedTouches( mGeosPrepared, geosGeom ) == 1 );
          break;
        case CROSSES:
          result = ( GEOSPreparedCrosses( mGeosPrepared, geosGeom ) == 1 );
          break;
        case WITHIN:
          result = ( GEOSPreparedWithin( mGeosPrepared, geosGeom ) == 1 );
          break;
        case CONTAINS:
          result = ( GEOSPreparedContains( mGeosPrepared, geosGeom ) == 1 );
          break;
        case DISJOINT:
          result = ( GEOSPreparedDisjoint( mGeosPrepared, geosGeom ) == 1 );
          break;
        default:
          GEOSGeom_destroy( geosGeom );
          return false;
      }
      GEOSGeom_destroy( geosGeom );
      return result;
    }

    switch ( r )
    {
      case INTERSECTS:
        result = ( GEOSIntersects( mGeos, geosGeom ) == 1 );
        break;
      case TOUCHES:
        result = ( GEOSTouches( mGeos, geosGeom ) == 1 );
        break;
      case CROSSES:
        result = ( GEOSCrosses( mGeos, geosGeom ) == 1 );
        break;
      case WITHIN:
        result = ( GEOSWithin( mGeos, geosGeom ) == 1 );
        break;
      case CONTAINS:
        result = ( GEOSContains( mGeos, geosGeom ) == 1 );
        break;
      case DISJOINT:
        result = ( GEOSDisjoint( mGeos, geosGeom ) == 1 );
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

  GEOSGeom_destroy( geosGeom );
  return result;
}

GEOSCoordSequence* QgsGeos::createCoordinateSequence( const QgsCurveV2* curve )
{
  bool segmentize = false;
  const QgsLineStringV2* line = dynamic_cast<const QgsLineStringV2*>( curve );

  if ( !line )
  {
    line = curve->curveToLine();
    segmentize = true;
  }

  if ( !line )
  {
    return 0;
  }

  bool hasZ = line->is3D();
  bool hasM = line->isMeasure();
  int coordDims = 2;
  if ( hasZ )
  {
    ++coordDims;
  }
  if ( hasM )
  {
    ++coordDims;
  }

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

  if ( segmentize )
  {
    delete line;
  }
  return coordSeq;
}

GEOSGeometry* QgsGeos::createGeosPoint( const QgsAbstractGeometryV2* point, int coordDims )
{
  const QgsPointV2* pt = dynamic_cast<const QgsPointV2*>( point );
  if ( pt )
  {
    GEOSCoordSequence* coordSeq = GEOSCoordSeq_create( 1, coordDims );
    GEOSCoordSeq_setX( coordSeq, 0, pt->x() );
    GEOSCoordSeq_setY( coordSeq, 0, pt->y() );
    if ( pt->is3D() )
    {
      GEOSCoordSeq_setOrdinate( coordSeq, 0, 2, pt->z() );
    }
    if ( pt->isMeasure() )
    {
      GEOSCoordSeq_setOrdinate( coordSeq, 0, 3, pt->m() );
    }
    return GEOSGeom_createPoint( coordSeq );
  }
}

GEOSGeometry* QgsGeos::createGeosLinestring( const QgsAbstractGeometryV2* curve )
{
  const QgsCurveV2* c = dynamic_cast<const QgsCurveV2*>( curve );
  if ( c )
  {
    GEOSCoordSequence* coordSeq = createCoordinateSequence( c );
    if ( !coordSeq )
    {
      return 0;
    }
    return GEOSGeom_createLineString( coordSeq );
  }
}

GEOSGeometry* QgsGeos::createGeosPolygon( const QgsAbstractGeometryV2* poly )
{
  const QgsCurvePolygonV2* polygon = dynamic_cast<const QgsCurvePolygonV2*>( poly );
  if ( polygon )
  {
    const QgsCurveV2* exteriorRing = polygon->exteriorRing();
    GEOSGeometry* exteriorRingGeos = GEOSGeom_createLinearRing( createCoordinateSequence( exteriorRing ) );

    int nHoles = polygon->numInteriorRings();
    GEOSGeometry** holes = 0;
    if ( nHoles > 0 )
    {
      holes = new GEOSGeometry*[ nHoles ];
    }

    for ( int i = 0; i < nHoles; ++i )
    {
      const QgsCurveV2* interiorRing = polygon->interiorRing( i );
      holes[i] = GEOSGeom_createLinearRing( createCoordinateSequence( interiorRing ) );
    }
    GEOSGeometry* geosPolygon = GEOSGeom_createPolygon( exteriorRingGeos, holes, nHoles );
    delete[] holes;
    return geosPolygon;
  }
}
