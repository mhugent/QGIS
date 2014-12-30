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
#include "qgsmulticurvev2.h"
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
    GEOSContextHandle_t ctxt;

    GEOSInit()
    {
      ctxt = initGEOS_r( printGEOSNotice, throwGEOSException );
    }

    ~GEOSInit()
    {
      finishGEOS_r( ctxt );
    }
};

static GEOSInit geosinit;

QgsGeos::QgsGeos( const QgsAbstractGeometryV2* geometry ): QgsGeometryEngine( geometry ), mGeos( 0 ), mGeosPrepared( 0 )
{
  cacheGeos();
}

QgsGeos::~QgsGeos()
{
  geometryChanged(); //remove caches
}

void QgsGeos::geometryChanged()
{
  GEOSGeom_destroy_r( geosinit.ctxt, mGeos );
  mGeos = 0;
  GEOSPreparedGeom_destroy_r( geosinit.ctxt, mGeosPrepared );
  mGeosPrepared = 0;
  cacheGeos();
}

void QgsGeos::prepareGeometry()
{
  GEOSPreparedGeom_destroy_r( geosinit.ctxt, mGeosPrepared );
  mGeosPrepared = 0;
  if ( mGeos )
  {
    mGeosPrepared = GEOSPrepare_r( geosinit.ctxt, mGeos );
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

int QgsGeos::splitGeometry( const QgsLineStringV2& splitLine,
                            QList<QgsAbstractGeometryV2*>&newGeometries,
                            bool topological,
                            QList<QgsPointV2> &topologyTestPoints ) const
{

  int returnCode = 0;
  if ( !mGeometry || !mGeos )
  {
    return 1;
  }

  //return if this type is point/multipoint
  if ( mGeometry->dimension() == 0 )
  {
    return 1; //cannot split points
  }

  if ( !GEOSisValid_r( geosinit.ctxt, mGeos ) )
    return 7;

  //make sure splitLine is valid
  if (( mGeometry->dimension() == 1 && splitLine.numPoints() < 1 ) ||
      ( mGeometry->dimension() == 2 && splitLine.numPoints() < 2 ) )
    return 1;

  newGeometries.clear();
  GEOSGeometry* splitLineGeos = 0;

  try
  {
    if ( splitLine.numPoints() > 1 )
    {
      splitLineGeos = createGeosLinestring( &splitLine );
    }
    else if ( splitLine.numPoints() == 1 )
    {
      QgsPointV2  pt = splitLine.pointN( 0 );
      splitLineGeos = createGeosPoint( &pt, 2 );
    }
    else
    {
      return 1;
    }

    if ( !GEOSisValid_r( geosinit.ctxt, splitLineGeos ) || !GEOSisSimple_r( geosinit.ctxt, splitLineGeos ) )
    {
      GEOSGeom_destroy_r( geosinit.ctxt, splitLineGeos );
      return 1;
    }

    if ( topological )
    {
      //find out candidate points for topological corrections
      if ( topologicalTestPointsSplit( splitLineGeos, topologyTestPoints ) != 0 )
        return 1;
    }

    //call split function depending on geometry type
    if ( mGeometry->dimension() == 1 )
    {
      returnCode = splitLinearGeometry( splitLineGeos, newGeometries );
      GEOSGeom_destroy_r( geosinit.ctxt, splitLineGeos );
    }
    else if ( mGeometry->dimension() == 2 )
    {
      returnCode = splitPolygonGeometry( splitLineGeos, newGeometries );
      GEOSGeom_destroy_r( geosinit.ctxt, splitLineGeos );
    }
    else
    {
      return 1;
    }
  }
  CATCH_GEOS( 2 )

  return returnCode;
}

int QgsGeos::topologicalTestPointsSplit( const GEOSGeometry* splitLine, QList<QgsPointV2>& testPoints ) const
{
  //Find out the intersection points between splitLineGeos and this geometry.
  //These points need to be tested for topological correctness by the calling function
  //if topological editing is enabled

  if ( !mGeos )
  {
    return 1;
  }

  testPoints.clear();
  GEOSGeometry* intersectionGeom = GEOSIntersection_r( geosinit.ctxt, mGeos, splitLine );
  if ( !intersectionGeom )
    return 1;

  bool simple = false;
  int nIntersectGeoms = 1;
  if ( GEOSGeomTypeId_r( geosinit.ctxt, intersectionGeom ) == GEOS_LINESTRING
       || GEOSGeomTypeId_r( geosinit.ctxt, intersectionGeom ) == GEOS_POINT )
    simple = true;

  if ( !simple )
    nIntersectGeoms = GEOSGetNumGeometries_r( geosinit.ctxt, intersectionGeom );

  for ( int i = 0; i < nIntersectGeoms; ++i )
  {
    const GEOSGeometry* currentIntersectGeom;
    if ( simple )
      currentIntersectGeom = intersectionGeom;
    else
      currentIntersectGeom = GEOSGetGeometryN_r( geosinit.ctxt, intersectionGeom, i );

    const GEOSCoordSequence* lineSequence = GEOSGeom_getCoordSeq_r( geosinit.ctxt, currentIntersectGeom );
    unsigned int sequenceSize = 0;
    double x, y;
    if ( GEOSCoordSeq_getSize_r( geosinit.ctxt, lineSequence, &sequenceSize ) != 0 )
    {
      for ( unsigned int i = 0; i < sequenceSize; ++i )
      {
        if ( GEOSCoordSeq_getX_r( geosinit.ctxt, lineSequence, i, &x ) != 0 )
        {
          if ( GEOSCoordSeq_getY_r( geosinit.ctxt, lineSequence, i, &y ) != 0 )
          {
            testPoints.push_back( QgsPointV2( x, y ) );
          }
        }
      }
    }
  }
  GEOSGeom_destroy_r( geosinit.ctxt, intersectionGeom );
  return 0;
}

GEOSGeometry* QgsGeos::linePointDifference( GEOSGeometry* GEOSsplitPoint ) const
{
  int type = GEOSGeomTypeId_r( geosinit.ctxt, mGeos );

  QgsMultiCurveV2* multiCurve = 0;
  if ( type == GEOS_MULTILINESTRING )
  {
    multiCurve = dynamic_cast<QgsMultiCurveV2*>( mGeometry->clone() );
  }
  else if ( type == GEOS_LINESTRING )
  {
    multiCurve = new QgsMultiCurveV2();
    multiCurve->addGeometry( mGeometry->clone() );
  }
  else
  {
    return 0;
  }

  if ( !multiCurve )
  {
    return 0;
  }


  QgsAbstractGeometryV2* splitGeom = fromGeos( GEOSsplitPoint );
  QgsPointV2* splitPoint = dynamic_cast<QgsPointV2*>( splitGeom );
  if ( !splitPoint )
  {
    delete splitGeom; return 0;
  }

  QgsMultiCurveV2 lines;

  //For each part
  for ( int i = 0; i < multiCurve->numGeometries(); ++i )
  {
    const QgsLineStringV2* line = dynamic_cast<const QgsLineStringV2*>( multiCurve->geometryN( i ) );
    if ( line )
    {
      //For each segment
      QgsLineStringV2 newLine;
      newLine.addVertex( line->pointN( 0 ) );
      int nVertices = newLine.numPoints();
      for ( int j = 1; j < ( nVertices - 1 ); ++j )
      {
        QgsPointV2 currentPoint = line->pointN( j );
        newLine.addVertex( currentPoint );
        if ( currentPoint == *splitPoint )
        {
          lines.addGeometry( newLine.clone() );
          newLine = QgsLineStringV2();
          newLine.addVertex( currentPoint );
        }
      }
      newLine.addVertex( line->pointN( nVertices - 1 ) );
      lines.addGeometry( newLine.clone() );
    }
  }

  delete splitGeom;
  delete multiCurve;
  return asGeos( &lines );
}

int QgsGeos::splitLinearGeometry( GEOSGeometry* splitLine, QList<QgsAbstractGeometryV2*>& newGeometries ) const
{
  if ( !splitLine )
    return 2;

  if ( !mGeos )
    return 5;

  //first test if linestring intersects geometry. If not, return straight away
  if ( !GEOSIntersects_r( geosinit.ctxt, splitLine, mGeos ) )
    return 1;

  //check that split line has no linear intersection
  int linearIntersect = GEOSRelatePattern_r( geosinit.ctxt, mGeos, splitLine, "1********" );
  if ( linearIntersect > 0 )
    return 3;

  int splitGeomType = GEOSGeomTypeId_r( geosinit.ctxt, splitLine );

  GEOSGeometry* splitGeom;
  if ( splitGeomType == GEOS_POINT )
  {
    splitGeom = linePointDifference( splitLine );
  }
  else
  {
    splitGeom = GEOSDifference_r( geosinit.ctxt, mGeos, splitLine );
  }
  QVector<GEOSGeometry*> lineGeoms;

  int splitType = GEOSGeomTypeId_r( geosinit.ctxt, splitGeom );
  if ( splitType == GEOS_MULTILINESTRING )
  {
    int nGeoms = GEOSGetNumGeometries_r( geosinit.ctxt, splitGeom );
    for ( int i = 0; i < nGeoms; ++i )
      lineGeoms << GEOSGeom_clone_r( geosinit.ctxt, GEOSGetGeometryN_r( geosinit.ctxt, splitGeom, i ) );

  }
  else
  {
    lineGeoms << GEOSGeom_clone_r( geosinit.ctxt, splitGeom );
  }

  mergeGeometriesMultiTypeSplit( lineGeoms );

  for ( int i = 0; i < lineGeoms.size(); ++i )
  {
    newGeometries << fromGeos( lineGeoms[i] );
  }

  GEOSGeom_destroy_r( geosinit.ctxt, splitGeom );
  return 0;
}

int QgsGeos::splitPolygonGeometry( GEOSGeometry* splitLine, QList<QgsAbstractGeometryV2*>& newGeometries ) const
{
  if ( !splitLine )
    return 2;

  if ( !mGeos )
    return 5;

  //first test if linestring intersects geometry. If not, return straight away
  if ( !GEOSIntersects_r( geosinit.ctxt, splitLine, mGeos ) )
    return 1;

  //first union all the polygon rings together (to get them noded, see JTS developer guide)
  GEOSGeometry *nodedGeometry = nodeGeometries( splitLine, mGeos );
  if ( !nodedGeometry )
    return 2; //an error occured during noding

  GEOSGeometry *polygons = GEOSPolygonize_r( geosinit.ctxt, &nodedGeometry, 1 );
  if ( !polygons || numberOfGeometries( polygons ) == 0 )
  {
    if ( polygons )
      GEOSGeom_destroy_r( geosinit.ctxt, polygons );

    GEOSGeom_destroy_r( geosinit.ctxt, nodedGeometry );

    return 4;
  }

  GEOSGeom_destroy_r( geosinit.ctxt, nodedGeometry );

  //test every polygon if contained in original geometry
  //include in result if yes
  QVector<GEOSGeometry*> testedGeometries;
  GEOSGeometry *intersectGeometry = 0;

  //ratio intersect geometry / geometry. This should be close to 1
  //if the polygon belongs to the input geometry

  for ( int i = 0; i < numberOfGeometries( polygons ); i++ )
  {
    const GEOSGeometry *polygon = GEOSGetGeometryN_r( geosinit.ctxt, polygons, i );
    intersectGeometry = GEOSIntersection_r( geosinit.ctxt, mGeos, polygon );
    if ( !intersectGeometry )
    {
      QgsDebugMsg( "intersectGeometry is NULL" );
      continue;
    }

    double intersectionArea;
    GEOSArea_r( geosinit.ctxt, intersectGeometry, &intersectionArea );

    double polygonArea;
    GEOSArea_r( geosinit.ctxt, polygon, &polygonArea );

    const double areaRatio = intersectionArea / polygonArea;
    if ( areaRatio > 0.99 && areaRatio < 1.01 )
      testedGeometries << GEOSGeom_clone_r( geosinit.ctxt, polygon );

    GEOSGeom_destroy_r( geosinit.ctxt, intersectGeometry );
  }

  bool splitDone = true;
  int nGeometriesThis = numberOfGeometries( mGeos ); //original number of geometries
  if ( testedGeometries.size() == nGeometriesThis )
  {
    splitDone = false;
  }

  mergeGeometriesMultiTypeSplit( testedGeometries );

  //no split done, preserve original geometry
  if ( !splitDone )
  {
    for ( int i = 0; i < testedGeometries.size(); ++i )
    {
      GEOSGeom_destroy_r( geosinit.ctxt, testedGeometries[i] );
    }
    return 1;
  }

  int i;
  for ( i = 0; i < testedGeometries.size() && GEOSisValid_r( geosinit.ctxt, testedGeometries[i] ); ++i )
    ;

  if ( i < testedGeometries.size() )
  {
    for ( i = 0; i < testedGeometries.size(); ++i )
      GEOSGeom_destroy_r( geosinit.ctxt, testedGeometries[i] );

    return 3;
  }

  for ( i = 0; i < testedGeometries.size(); ++i )
    newGeometries << fromGeos( testedGeometries[i] );

  GEOSGeom_destroy_r( geosinit.ctxt, polygons );
  return 0;
}

GEOSGeometry* QgsGeos::nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *geom )
{
  if ( !splitLine || !geom )
    return 0;

  GEOSGeometry *geometryBoundary = 0;
  if ( GEOSGeomTypeId( geom ) == GEOS_POLYGON || GEOSGeomTypeId( geom ) == GEOS_MULTIPOLYGON )
    geometryBoundary = GEOSBoundary_r( geosinit.ctxt, geom );
  else
    geometryBoundary = GEOSGeom_clone_r( geosinit.ctxt, geom );

  GEOSGeometry *splitLineClone = GEOSGeom_clone_r( geosinit.ctxt, splitLine );
  GEOSGeometry *unionGeometry = GEOSUnion_r( geosinit.ctxt, splitLineClone, geometryBoundary );
  GEOSGeom_destroy( splitLineClone );

  GEOSGeom_destroy_r( geosinit.ctxt, geometryBoundary );
  return unionGeometry;
}

int QgsGeos::mergeGeometriesMultiTypeSplit( QVector<GEOSGeometry*>& splitResult ) const
{
  if ( !mGeos )
    return 1;

  //convert mGeos to geometry collection
  int type = GEOSGeomTypeId_r( geosinit.ctxt, mGeos );
  if ( type != GEOS_GEOMETRYCOLLECTION &&
       type != GEOS_MULTILINESTRING &&
       type != GEOS_MULTIPOLYGON &&
       type != GEOS_MULTIPOINT )
    return 0;

  QVector<GEOSGeometry*> copyList = splitResult;
  splitResult.clear();

  //collect all the geometries that belong to the initial multifeature
  QVector<GEOSGeometry*> unionGeom;

  for ( int i = 0; i < copyList.size(); ++i )
  {
    //is this geometry a part of the original multitype?
    bool isPart = false;
    for ( int j = 0; j < GEOSGetNumGeometries_r( geosinit.ctxt, mGeos ); j++ )
    {
      if ( GEOSEquals_r( geosinit.ctxt, copyList[i], GEOSGetGeometryN_r( geosinit.ctxt, mGeos, j ) ) )
      {
        isPart = true;
        break;
      }
    }

    if ( isPart )
    {
      unionGeom << copyList[i];
    }
    else
    {
      QVector<GEOSGeometry*> geomVector;
      geomVector << copyList[i];

      if ( type == GEOS_MULTILINESTRING )
        splitResult << createGeosCollection( GEOS_MULTILINESTRING, geomVector );
      else if ( type == GEOS_MULTIPOLYGON )
        splitResult << createGeosCollection( GEOS_MULTIPOLYGON, geomVector );
      else
        GEOSGeom_destroy( copyList[i] );
    }
  }

  //make multifeature out of unionGeom
  if ( unionGeom.size() > 0 )
  {
    if ( type == GEOS_MULTILINESTRING )
      splitResult << createGeosCollection( GEOS_MULTILINESTRING, unionGeom );
    else if ( type == GEOS_MULTIPOLYGON )
      splitResult << createGeosCollection( GEOS_MULTIPOLYGON, unionGeom );
  }
  else
  {
    unionGeom.clear();
  }

  return 0;
}

GEOSGeometry* QgsGeos::createGeosCollection( int typeId, QVector<GEOSGeometry*> geoms ) const
{
  GEOSGeometry **geomarr = new GEOSGeometry*[ geoms.size()];
  if ( !geomarr )
    return 0;

  for ( int i = 0; i < geoms.size(); i++ )
    geomarr[i] = geoms[i];

  GEOSGeometry *geom = 0;

  try
  {
    geom = GEOSGeom_createCollection_r( geosinit.ctxt, typeId, geomarr, geoms.size() );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
  }

  delete [] geomarr;

  return geom;
}

QgsAbstractGeometryV2* QgsGeos::fromGeos( const GEOSGeometry* geos )
{
  if ( !geos )
  {
    return 0;
  }

  int nCoordDims = GEOSGeom_getCoordinateDimension_r( geosinit.ctxt, geos );
  int nDims = GEOSGeom_getDimensions_r( geosinit.ctxt, geos );
  bool hasZ = ( nCoordDims == 3 );
  bool hasM = (( nDims - nCoordDims ) == 1 );

  switch ( GEOSGeomTypeId_r( geosinit.ctxt, geos ) )
  {
    case GEOS_POINT:                 // a point
    {
      const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq_r( geosinit.ctxt, geos );
      return ( coordSeqPoint( cs, 0, hasZ, hasM ).clone() );
    }
    case GEOS_LINESTRING:
    {
      return sequenceToLinestring( geos, hasZ, hasM );
    }
    case GEOS_POLYGON:

      QgsPolygonV2* polygon = new QgsPolygonV2();

      const GEOSGeometry* ring = GEOSGetExteriorRing_r( geosinit.ctxt, geos );
      if ( ring )
      {
        polygon->setExteriorRing( sequenceToLinestring( ring, hasZ, hasM ) );
      }

      QList<QgsCurveV2*> interiorRings;
      for ( int i = 0; i < GEOSGetNumInteriorRings_r( geosinit.ctxt, geos ); ++i )
      {
        ring = GEOSGetInteriorRingN_r( geosinit.ctxt, geos, i );
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
  const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq_r( geosinit.ctxt, geos );
  unsigned int nPoints;
  GEOSCoordSeq_getSize_r( geosinit.ctxt, cs, &nPoints );
  for ( unsigned int i = 0; i < nPoints; ++i )
  {
    pts.push_back( coordSeqPoint( cs, i, hasZ, hasM ) );
  }
  QgsLineStringV2* line = new QgsLineStringV2();
  line->setPoints( pts );
  return line;
}

int QgsGeos::numberOfGeometries( GEOSGeometry* g )
{
  if ( !g )
    return 0;

  int geometryType = GEOSGeomTypeId_r( geosinit.ctxt, g );
  if ( geometryType == GEOS_POINT || geometryType == GEOS_LINESTRING || geometryType == GEOS_LINEARRING
       || geometryType == GEOS_POLYGON )
    return 1;

  //calling GEOSGetNumGeometries is save for multi types and collections also in geos2
  return GEOSGetNumGeometries_r( geosinit.ctxt, g );
}

QgsPointV2 QgsGeos::coordSeqPoint( const GEOSCoordSequence* cs, int i, bool hasZ, bool hasM )
{
  if ( !cs )
  {
    return QgsPointV2();
  }

  double x, y, z, m;
  GEOSCoordSeq_getX_r( geosinit.ctxt, cs, i, &x );
  GEOSCoordSeq_getY_r( geosinit.ctxt, cs, i, &y );
  if ( hasZ )
  {
    GEOSCoordSeq_getZ_r( geosinit.ctxt, cs, i, &z );
  }
  if ( hasM )
  {
    GEOSCoordSeq_getOrdinate_r( geosinit.ctxt, cs, i, 3, &m );
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
    return GEOSGeom_createCollection_r( geosinit.ctxt, GEOS_MULTIPOINT, geomarr, c->numGeometries() ); //todo: geos exceptions
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
    return GEOSGeom_createCollection_r( geosinit.ctxt, GEOS_MULTILINESTRING, geomarr, c->numGeometries() ); //todo: geos exceptions
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
    return GEOSGeom_createCollection_r( geosinit.ctxt, GEOS_MULTIPOLYGON, geomarr, c->numGeometries() ); //todo: geos exceptions
  }

  //todo: multitypes

  return 0;
}

QgsAbstractGeometryV2* QgsGeos::overlay( const QgsAbstractGeometryV2& geom, Overlay op ) const
{
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
        opGeom = GEOSIntersection_r( geosinit.ctxt, mGeos, geosGeom );
        break;
      case DIFFERENCE:
        opGeom = GEOSDifference_r( geosinit.ctxt, mGeos, geosGeom );
        break;
      case UNION:
        opGeom = GEOSUnion_r( geosinit.ctxt, mGeos, geosGeom );
        break;
      case SYMDIFFERENCE:
        opGeom = GEOSSymDifference_r( geosinit.ctxt, mGeos, geosGeom );
        break;
      default:    //unknown op
        GEOSGeom_destroy_r( geosinit.ctxt, geosGeom );
        return 0;
    }
    QgsAbstractGeometryV2* opResult = fromGeos( opGeom );
    GEOSGeom_destroy_r( geosinit.ctxt, opGeom );
    GEOSGeom_destroy_r( geosinit.ctxt, geosGeom );
    return opResult;
  }
  catch ( GEOSException &e )
  {
    GEOSGeom_destroy_r( geosinit.ctxt, geosGeom );
    return 0;
  }
}

bool QgsGeos::relation( const QgsAbstractGeometryV2& geom, Relation r ) const
{
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
          result = ( GEOSPreparedIntersects_r( geosinit.ctxt, mGeosPrepared, geosGeom ) == 1 );
          break;
        case TOUCHES:
          result = ( GEOSPreparedTouches_r( geosinit.ctxt, mGeosPrepared, geosGeom ) == 1 );
          break;
        case CROSSES:
          result = ( GEOSPreparedCrosses_r( geosinit.ctxt, mGeosPrepared, geosGeom ) == 1 );
          break;
        case WITHIN:
          result = ( GEOSPreparedWithin_r( geosinit.ctxt, mGeosPrepared, geosGeom ) == 1 );
          break;
        case CONTAINS:
          result = ( GEOSPreparedContains_r( geosinit.ctxt, mGeosPrepared, geosGeom ) == 1 );
          break;
        case DISJOINT:
          result = ( GEOSPreparedDisjoint_r( geosinit.ctxt, mGeosPrepared, geosGeom ) == 1 );
          break;
        default:
          GEOSGeom_destroy_r( geosinit.ctxt, geosGeom );
          return false;
      }
      GEOSGeom_destroy_r( geosinit.ctxt, geosGeom );
      return result;
    }

    switch ( r )
    {
      case INTERSECTS:
        result = ( GEOSIntersects_r( geosinit.ctxt, mGeos, geosGeom ) == 1 );
        break;
      case TOUCHES:
        result = ( GEOSTouches_r( geosinit.ctxt, mGeos, geosGeom ) == 1 );
        break;
      case CROSSES:
        result = ( GEOSCrosses_r( geosinit.ctxt, mGeos, geosGeom ) == 1 );
        break;
      case WITHIN:
        result = ( GEOSWithin_r( geosinit.ctxt, mGeos, geosGeom ) == 1 );
        break;
      case CONTAINS:
        result = ( GEOSContains_r( geosinit.ctxt, mGeos, geosGeom ) == 1 );
        break;
      case DISJOINT:
        result = ( GEOSDisjoint_r( geosinit.ctxt, mGeos, geosGeom ) == 1 );
        break;
      default:
        GEOSGeom_destroy_r( geosinit.ctxt, geosGeom );
        return false;
    }
  }
  catch ( GEOSException &e )
  {
    GEOSGeom_destroy_r( geosinit.ctxt, geosGeom );
    return 0;
  }

  GEOSGeom_destroy_r( geosinit.ctxt, geosGeom );
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
  GEOSCoordSequence* coordSeq = GEOSCoordSeq_create_r( geosinit.ctxt, numPoints, coordDims );
  for ( int i = 0; i < numPoints; ++i )
  {
    QgsPointV2 pt = line->pointN( i ); //todo: create method to get const point reference
    GEOSCoordSeq_setX_r( geosinit.ctxt, coordSeq, i, pt.x() );
    GEOSCoordSeq_setY_r( geosinit.ctxt, coordSeq, i, pt.y() );
    if ( hasZ )
    {
      GEOSCoordSeq_setOrdinate_r( geosinit.ctxt, coordSeq, i, 2, pt.z() );
    }
    if ( hasM )
    {
      GEOSCoordSeq_setOrdinate_r( geosinit.ctxt, coordSeq, i, 3, pt.m() );
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
    GEOSCoordSequence* coordSeq = GEOSCoordSeq_create_r( geosinit.ctxt, 1, coordDims );
    GEOSCoordSeq_setX_r( geosinit.ctxt, coordSeq, 0, pt->x() );
    GEOSCoordSeq_setY_r( geosinit.ctxt, coordSeq, 0, pt->y() );
    if ( pt->is3D() )
    {
      GEOSCoordSeq_setOrdinate_r( geosinit.ctxt, coordSeq, 0, 2, pt->z() );
    }
    if ( pt->isMeasure() )
    {
      GEOSCoordSeq_setOrdinate_r( geosinit.ctxt, coordSeq, 0, 3, pt->m() );
    }
    return GEOSGeom_createPoint_r( geosinit.ctxt, coordSeq );
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
    return GEOSGeom_createLineString_r( geosinit.ctxt, coordSeq );
  }
}

GEOSGeometry* QgsGeos::createGeosPolygon( const QgsAbstractGeometryV2* poly )
{
  const QgsCurvePolygonV2* polygon = dynamic_cast<const QgsCurvePolygonV2*>( poly );
  if ( polygon )
  {
    const QgsCurveV2* exteriorRing = polygon->exteriorRing();
    GEOSGeometry* exteriorRingGeos = GEOSGeom_createLinearRing_r( geosinit.ctxt, createCoordinateSequence( exteriorRing ) );

    int nHoles = polygon->numInteriorRings();
    GEOSGeometry** holes = 0;
    if ( nHoles > 0 )
    {
      holes = new GEOSGeometry*[ nHoles ];
    }

    for ( int i = 0; i < nHoles; ++i )
    {
      const QgsCurveV2* interiorRing = polygon->interiorRing( i );
      holes[i] = GEOSGeom_createLinearRing_r( geosinit.ctxt, createCoordinateSequence( interiorRing ) );
    }
    GEOSGeometry* geosPolygon = GEOSGeom_createPolygon_r( geosinit.ctxt, exteriorRingGeos, holes, nHoles );
    delete[] holes;
    return geosPolygon;
  }
}
