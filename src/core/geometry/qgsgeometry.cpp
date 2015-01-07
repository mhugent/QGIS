/***************************************************************************
  qgsgeometry.cpp - Geometry (stored as Open Geospatial Consortium WKB)
  -------------------------------------------------------------------
Date                 : 02 May 2005
Copyright            : (C) 2005 by Brendan Morley
email                : morb at ozemail dot com dot au
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <cstdarg>
#include <cstdio>
#include <cmath>

#include "qgis.h"
#include "qgsgeometry.h"
#include "qgsgeometryeditor.h"
#include "qgsgeometryimport.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgspoint.h"
#include "qgsrectangle.h"

#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgsgeometryvalidator.h"

#include "qgsmulticurvev2.h"
#include "qgsmultipointv2.h"
#include "qgsmultisurfacev2.h"
#include "qgspointv2.h"
#include "qgspolygonv2.h"
#include "qgslinestringv2.h"

#ifndef Q_WS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

struct QgsGeometryData
{
  QAtomicInt ref;
  QgsAbstractGeometryV2* geometry;
};

QgsGeometry::QgsGeometry(): d( new QgsGeometryData() ), mWkb( 0 ), mWkbSize( 0 ), mGeos( 0 )
{
  d->geometry = 0;
  d->ref = QAtomicInt( 1 );
}

QgsGeometry::~QgsGeometry()
{
  if ( d )
  {
    if ( !d->ref.deref() )
    {
      delete d->geometry;
      delete d;
    }
  }
  removeWkbGeos();
}

QgsGeometry::QgsGeometry( QgsAbstractGeometryV2* geom ): d( new QgsGeometryData() ), mWkb( 0 ), mWkbSize( 0 ), mGeos( 0 )
{
  d->geometry = geom;
  d->ref = QAtomicInt( 1 );
}

QgsGeometry::QgsGeometry( const QgsGeometry& other ): mWkb( 0 ), mWkbSize( 0 ), mGeos( 0 )
{
  d = other.d;
  d->ref.ref();
}

QgsGeometry& QgsGeometry::operator=( QgsGeometry const & other )
{
  if ( !d->ref.deref() )
  {
    delete d->geometry;
    delete d;
  }

  removeWkbGeos();

  d = other.d;
  d->ref.ref();
  return *this;
}

void QgsGeometry::detach()
{
  if ( !d )
  {
    return;
  }

  removeWkbGeos();

  if ( d->ref > 1 )
  {
    d->ref.deref();
    QgsAbstractGeometryV2* cloneGeom = 0;
    if ( d->geometry )
    {
      cloneGeom = d->geometry->clone();
    }
    d = new QgsGeometryData();
    d->geometry = cloneGeom;
    d->ref = QAtomicInt( 1 );
  }
}

void QgsGeometry::removeWkbGeos()
{
  delete[] mWkb;
  mWkbSize = 0;
  GEOSGeom_destroy( mGeos );
  mGeos = 0;
}

const QgsAbstractGeometryV2* QgsGeometry::geometry() const
{
  if ( !d )
  {
    return 0;
  }
  return d->geometry;
}

QgsGeometry* QgsGeometry::fromWkt( QString wkt )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::geomFromWkt( wkt );
  return new QgsGeometry( geom );
}

QgsGeometry* QgsGeometry::fromPoint( const QgsPoint& point )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromPoint( point );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromPolyline( const QgsPolyline& polyline )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromPolyline( polyline );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromPolygon( const QgsPolygon& polygon )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromPolygon( polygon );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromMultiPoint( const QgsMultiPoint& multipoint )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromMultiPoint( multipoint );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromMultiPolyline( multiline );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromMultiPolygon( multipoly );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromRect( const QgsRectangle& rect )
{
  QgsPolyline ring;
  ring.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  ring.append( QgsPoint( rect.xMaximum(), rect.yMinimum() ) );
  ring.append( QgsPoint( rect.xMaximum(), rect.yMaximum() ) );
  ring.append( QgsPoint( rect.xMinimum(), rect.yMaximum() ) );
  ring.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );

  QgsPolygon polygon;
  polygon.append( ring );

  return fromPolygon( polygon );
}

void QgsGeometry::fromWkb( unsigned char *wkb, size_t length )
{
  Q_UNUSED( length );
  if ( !d )
  {
    return;
  }

  detach();

  if ( d->geometry )
  {
    delete d->geometry;
    removeWkbGeos();
  }
  d->geometry = QgsGeometryImport::geomFromWkb( wkb );
  delete[] wkb;
}

const unsigned char *QgsGeometry::asWkb() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  if ( !mWkb )
  {
    mWkb = d->geometry->asBinary( mWkbSize );
  }
  return mWkb;
}

size_t QgsGeometry::wkbSize() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  if ( !mWkb )
  {
    mWkb = d->geometry->asBinary( mWkbSize );
  }
  return mWkbSize;
}

const GEOSGeometry* QgsGeometry::asGeos() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  if ( !mGeos )
  {
    mGeos = QgsGeos::asGeos( d->geometry );
  }
  return mGeos;
}


QGis::WkbType QgsGeometry::wkbType() const
{
  if ( !d || !d->geometry )
  {
    return QGis::WKBUnknown;
  }
  else
  {
    return d->geometry->wkbType();
  }
}


QGis::GeometryType QgsGeometry::type() const
{
  switch ( wkbType() )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBPointZ:
    case QGis::WKBPointM:
    case QGis::WKBPointZM:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPointZ:
    case QGis::WKBMultiPointM:
    case QGis::WKBMultiPointZM:
      return QGis::Point;

    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
      return QGis::Line;

    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygonZ:
    case QGis::WKBPolygonM:
    case QGis::WKBPolygonZM:
    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
      return QGis::Polygon;

    default:
      return QGis::UnknownGeometry;
  }
}

bool QgsGeometry::isMultipart() const
{
  return QGis::isMultiType( wkbType() );
}

void QgsGeometry::fromGeos( GEOSGeometry *geos )
{
#if 0
  // TODO - make this more heap-friendly

  if ( mGeos )
  {
    GEOSGeom_destroy( mGeos );
    mGeos = 0;
  }

  if ( mGeometry )
  {
    delete [] mGeometry;
    mGeometry = 0;
  }

  mGeos = geos;

  mDirtyWkb   = true;
  mDirtyGeos  = false;
#endif //0
}

QgsPoint QgsGeometry::closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist )
{
  if ( !d || !d->geometry )
  {
    return QgsPoint( 0, 0 );
  }
  QgsPointV2 pt( point.x(), point.y() );
  QgsVertexId id;

  QgsGeometryEditor geomEdit( d->geometry );
  QgsPointV2 vp = geomEdit.closestVertex( pt, id );
  if ( !id.isValid() )
  {
    sqrDist = -1;
    return QgsPoint( 0, 0 );
  }
  sqrDist = QgsGeometryUtils::sqrDistance2D( pt, vp );

  atVertex = vertexNrFromVertexId( id );
  adjacentVertices( atVertex, beforeVertex, afterVertex );
  return QgsPoint( vp.x(), vp.y() );
}

void QgsGeometry::adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex )
{
  if ( !d || !d->geometry )
  {
    return;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return;
  }

  QgsGeometryEditor geomEdit( d->geometry );
  QgsVertexId beforeVertexId, afterVertexId;
  geomEdit.adjacentVertices( id, beforeVertexId, afterVertexId );
  beforeVertex = vertexNrFromVertexId( beforeVertexId );
  afterVertex = vertexNrFromVertexId( afterVertexId );
}

bool QgsGeometry::moveVertex( double x, double y, int atVertex )
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  return d->geometry->moveVertex( id, QgsPointV2( x, y ) );
}

bool QgsGeometry::deleteVertex( int atVertex )
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  return d->geometry->deleteVertex( id );
}

bool QgsGeometry::insertVertex( double x, double y, int beforeVertex )
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( beforeVertex, id ) )
  {
    return false;
  }

  return d->geometry->insertVertex( id, QgsPointV2( x, y ) );
}

QgsPoint QgsGeometry::vertexAt( int atVertex )
{
  return QgsPoint( 0, 0 ); //todo...

#if 0
  if ( atVertex < 0 )
    return QgsPoint( 0, 0 );

  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return QgsPoint( 0, 0 );
  }

  QgsConstWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  bool hasZValue = false;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      if ( atVertex != 0 )
        return QgsPoint( 0, 0 );

      double x, y;
      wkbPtr >> x >> y;

      return QgsPoint( x, y );
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      // get number of points in the line
      int nPoints;
      wkbPtr >> nPoints;

      if ( atVertex >= nPoints )
        return QgsPoint( 0, 0 );

      // copy the vertex coordinates
      wkbPtr += atVertex * ( hasZValue ? 3 : 2 ) * sizeof( double );

      double x, y;
      wkbPtr >> x >> y;

      return QgsPoint( x, y );
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;

      for ( int ringnr = 0, pointIndex = 0; ringnr < nRings; ++ringnr )
      {
        int nPoints;
        wkbPtr >> nPoints;

        if ( atVertex >= pointIndex + nPoints )
        {
          wkbPtr += nPoints * ( hasZValue ? 3 : 2 ) * sizeof( double );
          pointIndex += nPoints;
          continue;
        }

        wkbPtr += ( atVertex - pointIndex ) * ( hasZValue ? 3 : 2 ) * sizeof( double );

        double x, y;
        wkbPtr >> x >> y;
        return QgsPoint( x, y );
      }

      return QgsPoint( 0, 0 );
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      // get number of points in the line
      int nPoints;
      wkbPtr >> nPoints;

      if ( atVertex >= nPoints )
        return QgsPoint( 0, 0 );

      wkbPtr += atVertex * ( 1 + sizeof( int ) + ( hasZValue ? 3 : 2 ) * sizeof( double ) ) + 1 + sizeof( int );

      double x, y;
      wkbPtr >> x >> y;
      return QgsPoint( x, y );
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;

      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        wkbPtr += 1 + sizeof( int );
        int nPoints;
        wkbPtr >> nPoints;

        if ( atVertex >= pointIndex + nPoints )
        {
          wkbPtr += nPoints * ( hasZValue ? 3 : 2 ) * sizeof( double );
          pointIndex += nPoints;
          continue;
        }

        wkbPtr += ( atVertex - pointIndex ) * ( hasZValue ? 3 : 2 ) * sizeof( double );

        double x, y;
        wkbPtr >> x >> y;

        return QgsPoint( x, y );
      }

      return QgsPoint( 0, 0 );
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolygons;
      wkbPtr >> nPolygons;

      for ( int polynr = 0, pointIndex = 0; polynr < nPolygons; ++polynr )
      {
        wkbPtr += 1 + sizeof( int );

        int nRings;
        wkbPtr >> nRings;
        for ( int ringnr = 0; ringnr < nRings; ++ringnr )
        {
          int nPoints;
          wkbPtr >> nPoints;

          if ( atVertex >= pointIndex + nPoints )
          {
            wkbPtr += nPoints * ( hasZValue ? 3 : 2 ) * sizeof( double );
            pointIndex += nPoints;
            continue;
          }

          wkbPtr += ( atVertex - pointIndex ) * ( hasZValue ? 3 : 2 ) * sizeof( double );

          double x, y;
          wkbPtr >> x >> y;

          return QgsPoint( x, y );
        }
      }
      return QgsPoint( 0, 0 );
    }

    default:
      QgsDebugMsg( "error: mGeometry type not recognized" );
      return QgsPoint( 0, 0 );
  }
#endif //0
}

double QgsGeometry::sqrDistToVertexAt( QgsPoint& point, int atVertex )
{
  return 0; //todo...
#if 0
  QgsPoint pnt = vertexAt( atVertex );
  if ( pnt != QgsPoint( 0, 0 ) )
  {
    QgsDebugMsg( "Exiting with distance to " + pnt.toString() );
    return point.sqrDist( pnt );
  }
  else
  {
    QgsDebugMsg( "Exiting with std::numeric_limits<double>::max()." );
    // probably safest to bail out with a very large number
    return std::numeric_limits<double>::max();
  }
#endif //0
}

double QgsGeometry::closestVertexWithContext( const QgsPoint& point, int& atVertex )
{
  return 0; //todo...

#if 0
  double sqrDist = std::numeric_limits<double>::max();

  try
  {
    // Initialise some stuff
    int closestVertexIndex = 0;

    // set up the GEOS geometry
    if ( mDirtyGeos )
      exportWkbToGeos();

    if ( !mGeos )
      return -1;

    const GEOSGeometry *g = GEOSGetExteriorRing( mGeos );
    if ( !g )
      return -1;

    const GEOSCoordSequence *sequence = GEOSGeom_getCoordSeq( g );

    unsigned int n;
    GEOSCoordSeq_getSize( sequence, &n );

    for ( unsigned int i = 0; i < n; i++ )
    {
      double x, y;
      GEOSCoordSeq_getX( sequence, i, &x );
      GEOSCoordSeq_getY( sequence, i, &y );

      double testDist = point.sqrDist( x, y );
      if ( testDist < sqrDist )
      {
        closestVertexIndex = i;
        sqrDist = testDist;
      }
    }

    atVertex = closestVertexIndex;
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return -1;
  }

  return sqrDist;
#endif //0
}

double QgsGeometry::closestSegmentWithContext(
  const QgsPoint& point,
  QgsPoint& minDistPoint,
  int& afterVertex,
  double *leftOf,
  double epsilon )
{
  return 0; //todo...

#if 0
  QgsDebugMsg( "Entering." );

  // TODO: implement with GEOS
  if ( mDirtyWkb ) //convert latest geos to mGeometry
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return -1;
  }

  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  // Initialise some stuff
  double sqrDist = std::numeric_limits<double>::max();

  QgsPoint distPoint;
  int closestSegmentIndex = 0;
  bool hasZValue = false;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    {
      // Points have no lines
      return -1;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;

      double prevx = 0.0, prevy = 0.0;
      for ( int index = 0; index < nPoints; ++index )
      {
        double thisx, thisy;
        wkbPtr >> thisx >> thisy;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        if ( index > 0 )
        {
          double testdist = point.sqrDistToSegment( prevx, prevy, thisx, thisy, distPoint, epsilon );
          if ( testdist < sqrDist )
          {
            closestSegmentIndex = index;
            sqrDist = testdist;
            minDistPoint = distPoint;
            if ( leftOf )
            {
              *leftOf = QgsGeometry::leftOf( point.x(), point.y(), prevx, prevy, thisx, thisy );
            }
          }
        }

        prevx = thisx;
        prevy = thisy;
      }
      afterVertex = closestSegmentIndex;
      break;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;
      for ( int linenr = 0, pointIndex = 0; linenr < nLines; ++linenr )
      {
        wkbPtr += 1 + sizeof( int );
        int nPoints;
        wkbPtr >> nPoints;

        double prevx = 0.0, prevy = 0.0;
        for ( int pointnr = 0; pointnr < nPoints; ++pointnr )
        {
          double thisx, thisy;
          wkbPtr >> thisx >> thisy;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          if ( pointnr > 0 )
          {
            double testdist = point.sqrDistToSegment( prevx, prevy, thisx, thisy, distPoint, epsilon );
            if ( testdist < sqrDist )
            {
              closestSegmentIndex = pointIndex;
              sqrDist = testdist;
              minDistPoint = distPoint;
              if ( leftOf )
              {
                *leftOf = QgsGeometry::leftOf( point.x(), point.y(), prevx, prevy, thisx, thisy );
              }
            }
          }

          prevx = thisx;
          prevy = thisy;
          ++pointIndex;
        }
      }
      afterVertex = closestSegmentIndex;
      break;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      int nRings;
      wkbPtr >> nRings;

      for ( int ringnr = 0, pointIndex = 0; ringnr < nRings; ++ringnr )//loop over rings
      {
        int nPoints;
        wkbPtr >> nPoints;

        double prevx = 0.0, prevy = 0.0;
        for ( int pointnr = 0; pointnr < nPoints; ++pointnr )//loop over points in a ring
        {
          double thisx, thisy;
          wkbPtr >> thisx >> thisy;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          if ( pointnr > 0 )
          {
            double testdist = point.sqrDistToSegment( prevx, prevy, thisx, thisy, distPoint, epsilon );
            if ( testdist < sqrDist )
            {
              closestSegmentIndex = pointIndex;
              sqrDist = testdist;
              minDistPoint = distPoint;
              if ( leftOf )
              {
                *leftOf = QgsGeometry::leftOf( point.x(), point.y(), prevx, prevy, thisx, thisy );
              }
            }
          }

          prevx = thisx;
          prevy = thisy;
          ++pointIndex;
        }
      }
      afterVertex = closestSegmentIndex;
      break;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolygons;
      wkbPtr >> nPolygons;
      for ( int polynr = 0, pointIndex = 0; polynr < nPolygons; ++polynr )
      {
        wkbPtr += 1 + sizeof( int );
        int nRings;
        wkbPtr >> nRings;
        for ( int ringnr = 0; ringnr < nRings; ++ringnr )
        {
          int nPoints;
          wkbPtr >> nPoints;

          double prevx = 0.0, prevy = 0.0;
          for ( int pointnr = 0; pointnr < nPoints; ++pointnr )
          {
            double thisx, thisy;
            wkbPtr >> thisx >> thisy;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            if ( pointnr > 0 )
            {
              double testdist = point.sqrDistToSegment( prevx, prevy, thisx, thisy, distPoint, epsilon );
              if ( testdist < sqrDist )
              {
                closestSegmentIndex = pointIndex;
                sqrDist = testdist;
                minDistPoint = distPoint;
                if ( leftOf )
                {
                  *leftOf = QgsGeometry::leftOf( point.x(), point.y(), prevx, prevy, thisx, thisy );
                }
              }
            }

            prevx = thisx;
            prevy = thisy;
            ++pointIndex;
          }
        }
      }
      afterVertex = closestSegmentIndex;
      break;
    }

    case QGis::WKBUnknown:
    default:
      return -1;
      break;
  } // switch (wkbType)

  QgsDebugMsg( QString( "Exiting with nearest point %1, dist %2." )
               .arg( point.toString() ).arg( sqrDist ) );

  return sqrDist;
#endif //0
}

int QgsGeometry::addRing( const QList<QgsPoint>& ring )
{
  if ( !d || !d->geometry )
  {
    return 1;
  }

  QgsLineStringV2* ringLine = new QgsLineStringV2();
  QList< QgsPointV2 > ringPoints;
  convertPointList( ring, ringPoints );
  ringLine->setPoints( ringPoints );

  QgsGeometryEditor geomEditor( d->geometry );
  return geomEditor.addRing( ringLine );
}

int QgsGeometry::addPart( const QList<QgsPoint> &points, QGis::GeometryType geomType )
{
  if ( !d || !d->geometry )
  {
    return 2;
  }

  QgsAbstractGeometryV2* partGeom = 0;
  if ( points.size() == 1 )
  {
    partGeom = new QgsPointV2( points[0].x(), points[0].y() );
  }
  else if ( points.size() > 1 )
  {
    QgsLineStringV2* ringLine = new QgsLineStringV2();
    QList< QgsPointV2 > partPoints;
    convertPointList( points, partPoints );
    ringLine->setPoints( partPoints );
    partGeom = ringLine;
  }
  QgsGeometryEditor geomEditor( d->geometry );
  return geomEditor.addPart( partGeom );
}

int QgsGeometry::addPart( QgsGeometry *newPart )
{
  if ( !d || !d->geometry || !newPart || !newPart->d || !newPart->d->geometry )
  {
    return 1;
  }

  QgsAbstractGeometryV2* geom = newPart->d->geometry->clone();
  QgsGeometryEditor geomEditor( d->geometry );
  return geomEditor.addPart( geom );
}

int QgsGeometry::addPart( GEOSGeometry *newPart )
{
  if ( !d || !d->geometry || !newPart )
  {
    return 1;
  }

  QgsAbstractGeometryV2* geom = QgsGeos::fromGeos( newPart );
  QgsGeometryEditor geomEditor( d->geometry );
  return geomEditor.addPart( geom );
}

int QgsGeometry::translate( double dx, double dy )
{
  if ( !d || !d->geometry )
  {
    return 1;
  }

  d->geometry->translate( dx, dy, 0, 0 );
  return 0;
}

int QgsGeometry::splitGeometry( const QList<QgsPoint>& splitLine, QList<QgsGeometry*>& newGeometries, bool topological, QList<QgsPoint> &topologyTestPoints )
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QList<QgsAbstractGeometryV2*> newGeoms;
  QgsLineStringV2 splitLineString;
  QList<QgsPointV2> splitLinePointsV2;
  convertPointList( splitLine, splitLinePointsV2 );
  splitLineString.setPoints( splitLinePointsV2 );
  QList<QgsPointV2> tp;

  QgsGeos geos( d->geometry );
  int result = geos.splitGeometry( splitLineString, newGeoms, topological, tp );

  newGeometries.clear();
  for ( int i = 0; i < newGeoms.size(); ++i )
  {
    newGeometries.push_back( new QgsGeometry( newGeoms.at( i ) ) );
  }

  convertPointList( tp, topologyTestPoints );
  return result;
}

/**Replaces a part of this geometry with another line*/
int QgsGeometry::reshapeGeometry( const QList<QgsPoint>& reshapeWithLine )
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QList<QgsPointV2> reshapeLine;
  convertPointList( reshapeWithLine, reshapeLine );
  QgsLineStringV2 reshapeLineString;
  reshapeLineString.setPoints( reshapeLine );

  QgsGeos geos( d->geometry );
  int errorCode = 0;
  QgsAbstractGeometryV2* geom = geos.reshapeGeometry( reshapeLineString, &errorCode );
  if ( errorCode == 0 && geom )
  {
    delete d->geometry;
    d->geometry = geom;
    return 0;
  }
  return errorCode;
}

int QgsGeometry::makeDifference( QgsGeometry* other )
{
  return 0;

#if 0
  //make sure geos geometry is up to date
  if ( !other )
    return 1;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 1;

  if ( !GEOSisValid( mGeos ) )
    return 2;

  if ( !GEOSisSimple( mGeos ) )
    return 3;

  //convert other geometry to geos
  if ( other->mDirtyGeos )
    other->exportWkbToGeos();

  if ( !other->mGeos )
    return 4;

  //make geometry::difference
  try
  {
    if ( GEOSIntersects( mGeos, other->mGeos ) )
    {
      //check if multitype before and after
      bool multiType = isMultipart();

      mGeos = GEOSDifference( mGeos, other->mGeos );
      mDirtyWkb = true;

      if ( multiType && !isMultipart() )
      {
        convertToMultiType();
        exportWkbToGeos();
      }
    }
    else
    {
      return 0; //nothing to do
    }
  }
  CATCH_GEOS( 5 )

  if ( !mGeos )
  {
    mDirtyGeos = true;
    return 6;
  }

  return 0;
#endif //0
}

QgsRectangle QgsGeometry::boundingBox() const
{
  if ( d && d->geometry )
  {
    return d->geometry->boundingBox();
  }
  return QgsRectangle();
}

bool QgsGeometry::intersects( const QgsRectangle& r ) const
{
  QgsGeometry* g = fromRect( r );
  bool res = intersects( g );
  delete g;
  return res;
}

bool QgsGeometry::intersects( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.intersects( *( geometry->d->geometry ) );
}

bool QgsGeometry::contains( const QgsPoint* p ) const
{
  if ( !d || !d->geometry || !p )
  {
    return false;
  }

  QgsPointV2 pt( p->x(), p->y() );
  QgsGeos geos( d->geometry );
  return geos.contains( pt );
}

bool QgsGeometry::contains( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.contains( *( geometry->d->geometry ) );
}

bool QgsGeometry::disjoint( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.disjoint( *( geometry->d->geometry ) );
}

bool QgsGeometry::equals( const QgsGeometry* geometry ) const
{
  return false; //todo //return geosRelOp( GEOSEquals, this, geometry );
}

bool QgsGeometry::touches( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.touches( *( geometry->d->geometry ) );
}

bool QgsGeometry::overlaps( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.overlaps( *( geometry->d->geometry ) );
}

bool QgsGeometry::within( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.within( *( geometry->d->geometry ) );
}

bool QgsGeometry::crosses( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.crosses( *( geometry->d->geometry ) );
}

QString QgsGeometry::exportToWkt( const int &precision ) const
{
  return QString();
#if 0
  QgsDebugMsg( "entered." );

  // TODO: implement with GEOS
  if ( mDirtyWkb )
  {
    exportGeosToWkb();
  }

  if ( !mGeometry || wkbSize() < 5 )
  {
    QgsDebugMsg( "WKB geometry not available or too short!" );
    return QString::null;
  }

  bool hasZValue = false;
  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;

  QString wkt;

  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      double x, y;
      wkbPtr >> x >> y;
      wkt += "POINT(" + qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision ) + ")";
      return wkt;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      int nPoints;
      wkbPtr >> nPoints;

      wkt += "LINESTRING(";
      // get number of points in the line
      for ( int idx = 0; idx < nPoints; ++idx )
      {
        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        if ( idx != 0 )
          wkt += ", ";

        wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
      }
      wkt += ")";
      return wkt;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {
      wkt += "POLYGON(";
      // get number of rings in the polygon
      int nRings;
      wkbPtr >> nRings;
      if ( nRings == 0 )  // sanity check for zero rings in polygon
        return QString();

      for ( int idx = 0; idx < nRings; idx++ )
      {
        if ( idx != 0 )
          wkt += ",";

        wkt += "(";
        // get number of points in the ring
        int nPoints;
        wkbPtr >> nPoints;

        for ( int jdx = 0; jdx < nPoints; jdx++ )
        {
          if ( jdx != 0 )
            wkt += ",";

          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
        }
        wkt += ")";
      }
      wkt += ")";
      return wkt;
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      int nPoints;
      wkbPtr >> nPoints;

      wkt += "MULTIPOINT(";
      for ( int idx = 0; idx < nPoints; ++idx )
      {
        wkbPtr += 1 + sizeof( int );
        if ( idx != 0 )
          wkt += ", ";

        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
      }
      wkt += ")";
      return wkt;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      int nLines;
      wkbPtr >> nLines;

      wkt += "MULTILINESTRING(";
      for ( int jdx = 0; jdx < nLines; jdx++ )
      {
        if ( jdx != 0 )
          wkt += ", ";

        wkt += "(";
        wkbPtr += 1 + sizeof( int ); // skip type since we know its 2
        int nPoints;
        wkbPtr >> nPoints;
        for ( int idx = 0; idx < nPoints; idx++ )
        {
          if ( idx != 0 )
            wkt += ", ";

          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
        }
        wkt += ")";
      }
      wkt += ")";
      return wkt;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      int nPolygons;
      wkbPtr >> nPolygons;

      wkt += "MULTIPOLYGON(";
      for ( int kdx = 0; kdx < nPolygons; kdx++ )
      {
        if ( kdx != 0 )
          wkt += ",";

        wkt += "(";
        wkbPtr += 1 + sizeof( int );
        int nRings;
        wkbPtr >> nRings;
        for ( int idx = 0; idx < nRings; idx++ )
        {
          if ( idx != 0 )
            wkt += ",";

          wkt += "(";
          int nPoints;
          wkbPtr >> nPoints;
          for ( int jdx = 0; jdx < nPoints; jdx++ )
          {
            if ( jdx != 0 )
              wkt += ",";

            double x, y;
            wkbPtr >> x >> y;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            wkt += qgsDoubleToString( x, precision ) + " " + qgsDoubleToString( y, precision );
          }
          wkt += ")";
        }
        wkt += ")";
      }
      wkt += ")";
      return wkt;
    }

    default:
      QgsDebugMsg( "error: mGeometry type not recognized" );
      return QString::null;
  }
#endif //0
}

QString QgsGeometry::exportToGeoJSON( const int &precision ) const
{
  return QString(); //todo...
#if 0
  QgsDebugMsg( "entered." );

  // TODO: implement with GEOS
  if ( mDirtyWkb )
    exportGeosToWkb();

  if ( !mGeometry )
  {
    QgsDebugMsg( "WKB geometry not available!" );
    return QString::null;
  }

  QgsWkbPtr wkbPtr( mGeometry + 1 );
  QGis::WkbType wkbType;
  wkbPtr >> wkbType;
  bool hasZValue = false;

  QString wkt;

  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {

      double x, y;
      wkbPtr >> x >> y;

      wkt += "{ \"type\": \"Point\", \"coordinates\": ["
             + qgsDoubleToString( x, precision ) + ", "
             + qgsDoubleToString( y, precision )
             + "] }";
      return wkt;
    }

    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {

      wkt += "{ \"type\": \"LineString\", \"coordinates\": [ ";
      // get number of points in the line
      int nPoints;
      wkbPtr >> nPoints;
      for ( int idx = 0; idx < nPoints; ++idx )
      {
        if ( idx != 0 )
          wkt += ", ";

        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
      }
      wkt += " ] }";
      return wkt;
    }

    case QGis::WKBPolygon25D:
      hasZValue = true;
    case QGis::WKBPolygon:
    {

      wkt += "{ \"type\": \"Polygon\", \"coordinates\": [ ";
      // get number of rings in the polygon
      int nRings;
      wkbPtr >> nRings;
      if ( nRings == 0 )  // sanity check for zero rings in polygon
        return QString();

      for ( int idx = 0; idx < nRings; idx++ )
      {
        if ( idx != 0 )
          wkt += ", ";

        wkt += "[ ";
        // get number of points in the ring
        int nPoints;
        wkbPtr >> nPoints;

        for ( int jdx = 0; jdx < nPoints; jdx++ )
        {
          if ( jdx != 0 )
            wkt += ", ";

          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
        }
        wkt += " ]";
      }
      wkt += " ] }";
      return wkt;
    }

    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      wkt += "{ \"type\": \"MultiPoint\", \"coordinates\": [ ";
      int nPoints;
      wkbPtr >> nPoints;
      for ( int idx = 0; idx < nPoints; ++idx )
      {
        wkbPtr += 1 + sizeof( int );
        if ( idx != 0 )
          wkt += ", ";

        double x, y;
        wkbPtr >> x >> y;
        if ( hasZValue )
          wkbPtr += sizeof( double );

        wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
      }
      wkt += " ] }";
      return wkt;
    }

    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      wkt += "{ \"type\": \"MultiLineString\", \"coordinates\": [ ";

      int nLines;
      wkbPtr >> nLines;
      for ( int jdx = 0; jdx < nLines; jdx++ )
      {
        if ( jdx != 0 )
          wkt += ", ";

        wkt += "[ ";
        wkbPtr += 1 + sizeof( int ); // skip type since we know its 2

        int nPoints;
        wkbPtr >> nPoints;
        for ( int idx = 0; idx < nPoints; idx++ )
        {
          if ( idx != 0 )
            wkt += ", ";

          double x, y;
          wkbPtr >> x >> y;
          if ( hasZValue )
            wkbPtr += sizeof( double );

          wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
        }
        wkt += " ]";
      }
      wkt += " ] }";
      return wkt;
    }

    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {

      wkt += "{ \"type\": \"MultiPolygon\", \"coordinates\": [ ";

      int nPolygons;
      wkbPtr >> nPolygons;
      for ( int kdx = 0; kdx < nPolygons; kdx++ )
      {
        if ( kdx != 0 )
          wkt += ", ";

        wkt += "[ ";

        wkbPtr += 1 + sizeof( int );

        int nRings;
        wkbPtr >> nRings;
        for ( int idx = 0; idx < nRings; idx++ )
        {
          if ( idx != 0 )
            wkt += ", ";

          wkt += "[ ";

          int nPoints;
          wkbPtr >> nPoints;
          for ( int jdx = 0; jdx < nPoints; jdx++ )
          {
            if ( jdx != 0 )
              wkt += ", ";

            double x, y;
            wkbPtr >> x >> y;
            if ( hasZValue )
              wkbPtr += sizeof( double );

            wkt += "[" + qgsDoubleToString( x, precision ) + ", " + qgsDoubleToString( y, precision ) + "]";
          }
          wkt += " ]";
        }
        wkt += " ]";
      }
      wkt += " ] }";
      return wkt;
    }

    default:
      QgsDebugMsg( "error: mGeometry type not recognized" );
      return QString::null;
  }
#endif //0
}

QgsGeometry* QgsGeometry::convertToType( QGis::GeometryType destType, bool destMultipart )
{
  return 0; //todo...
#if 0
  switch ( destType )
  {
    case QGis::Point:
      return convertToPoint( destMultipart );

    case QGis::Line:
      return convertToLine( destMultipart );

    case QGis::Polygon:
      return convertToPolygon( destMultipart );

    default:
      return 0;
  }
#endif //0
}

bool QgsGeometry::convertToMultiType()
{
  return false; //todo...

#if 0
  // TODO: implement with GEOS
  if ( mDirtyWkb )
  {
    exportGeosToWkb();
  }

  if ( !mGeometry )
  {
    return false;
  }

  QGis::WkbType geomType = wkbType();

  if ( geomType == QGis::WKBMultiPoint || geomType == QGis::WKBMultiPoint25D ||
       geomType == QGis::WKBMultiLineString || geomType == QGis::WKBMultiLineString25D ||
       geomType == QGis::WKBMultiPolygon || geomType == QGis::WKBMultiPolygon25D || geomType == QGis::WKBUnknown )
  {
    return false; //no need to convert
  }

  size_t newGeomSize = mGeometrySize + 1 + 2 * sizeof( int ); //endian: 1, multitype: sizeof(int), number of geometries: sizeof(int)
  unsigned char* newGeometry = new unsigned char[newGeomSize];

  //copy endian
  char byteOrder = QgsApplication::endian();

  QgsWkbPtr wkbPtr( newGeometry );
  wkbPtr << byteOrder;

  //copy wkbtype
  //todo
  QGis::WkbType newMultiType;
  switch ( geomType )
  {
    case QGis::WKBPoint:
      newMultiType = QGis::WKBMultiPoint;
      break;
    case QGis::WKBPoint25D:
      newMultiType = QGis::WKBMultiPoint25D;
      break;
    case QGis::WKBLineString:
      newMultiType = QGis::WKBMultiLineString;
      break;
    case QGis::WKBLineString25D:
      newMultiType = QGis::WKBMultiLineString25D;
      break;
    case QGis::WKBPolygon:
      newMultiType = QGis::WKBMultiPolygon;
      break;
    case QGis::WKBPolygon25D:
      newMultiType = QGis::WKBMultiPolygon25D;
      break;
    default:
      delete [] newGeometry;
      return false;
  }

  wkbPtr << newMultiType << 1;

  //copy the existing single geometry
  memcpy( wkbPtr, mGeometry, mGeometrySize );

  delete [] mGeometry;
  mGeometry = newGeometry;
  mGeometrySize = newGeomSize;
  mDirtyGeos = true;
  return true;
#endif //0
}

QgsPoint QgsGeometry::asPoint() const
{
  if ( !d || !d->geometry || d->geometry->geometryType() != "Point" )
  {
    return QgsPoint();
  }
  QgsPointV2* pt = dynamic_cast<QgsPointV2*>( d->geometry );
  if ( !pt )
  {
    return QgsPoint();
  }

  return QgsPoint( pt->x(), pt->y() );
}

QgsPolyline QgsGeometry::asPolyline() const
{
  QgsPolyline polyLine;
  if ( !d || !d->geometry )
  {
    return polyLine;
  }

  bool doSegmentation = ( d->geometry->geometryType() == "CompoundCurve" || d->geometry->geometryType() == "CircularString" );
  QgsLineStringV2* line = 0;
  if ( doSegmentation )
  {
    QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( d->geometry );
    if ( !curve )
    {
      return polyLine;
    }
    line = curve->curveToLine();
  }
  else
  {
    line = dynamic_cast<QgsLineStringV2*>( d->geometry );
    if ( !line )
    {
      return polyLine;
    }
  }

  int nVertices = line->numPoints();
  polyLine.resize( nVertices );
  for ( int i = 0; i < nVertices; ++i )
  {
    QgsPointV2 pt = line->pointN( i );
    polyLine[i].setX( pt.x() );
    polyLine[i].setY( pt.y() );
  }

  if ( doSegmentation )
  {
    delete line;
  }

  return polyLine;
}

QgsPolygon QgsGeometry::asPolygon() const
{
  bool doSegmentation = ( d->geometry->geometryType() == "CurvePolygon" );

  QgsPolygonV2* p = 0;
  if ( doSegmentation )
  {
    QgsCurvePolygonV2* curvePoly = dynamic_cast<QgsCurvePolygonV2*>( d->geometry );
    if ( !curvePoly )
    {
      return QgsPolygon();
    }
    p = curvePoly->toPolygon();
  }
  else
  {
    p = dynamic_cast<QgsPolygonV2*>( d->geometry );
  }

  if ( !p )
  {
    return QgsPolygon();
  }

  QgsPolygon polygon;
  convertPolygon( *p, polygon );

  if ( doSegmentation )
  {
    delete p;
  }
  return polygon;
}

QgsMultiPoint QgsGeometry::asMultiPoint() const
{
  if ( !d || !d->geometry || d->geometry->geometryType() != "MultiPoint" )
  {
    return QgsMultiPoint();
  }

  const QgsMultiPointV2* mp = dynamic_cast<QgsMultiPointV2*>( d->geometry );
  if ( !mp )
  {
    return QgsMultiPoint();
  }

  int nPoints = mp->numGeometries();
  QgsMultiPoint multiPoint( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    const QgsPointV2* pt = dynamic_cast<const QgsPointV2*>( mp->geometryN( i ) );
    multiPoint[i].setX( pt->x() );
    multiPoint[i].setY( pt->y() );
  }
  return multiPoint;
}

QgsMultiPolyline QgsGeometry::asMultiPolyline() const
{
  if ( !d || !d->geometry || d->geometry->geometryType() != "MultiCurve" )
  {
    return QgsMultiPolyline();
  }

  QgsMultiCurveV2* multiCurve = dynamic_cast<QgsMultiCurveV2*>( d->geometry );
  if ( !multiCurve )
  {
    return QgsMultiPolyline();
  }

  int nLines = multiCurve->numGeometries();
  if ( nLines < 1 )
  {
    return QgsMultiPolyline();
  }

  QgsMultiPolyline mpl;
  for ( int i = 0; i < nLines; ++i )
  {
    QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( multiCurve->geometryN( i ) );
    if ( !curve )
    {
      continue;
    }
    QgsLineStringV2* linestring = curve->curveToLine();
    if ( linestring )
    {
      QList< QgsPointV2 > lineCoords;
      linestring->points( lineCoords );
      delete linestring;
      QgsPolyline polyLine;
      convertToPolyline( lineCoords, polyLine );
      mpl.append( polyLine );
    }
  }
  return mpl;
}

QgsMultiPolygon QgsGeometry::asMultiPolygon() const
{
  if ( !d || !d->geometry || d->geometry->geometryType() != "MultiSurface" )
  {
    return QgsMultiPolygon();
  }

  QgsMultiSurfaceV2* multiSurface = dynamic_cast<QgsMultiSurfaceV2*>( d->geometry );
  if ( !multiSurface )
  {
    return QgsMultiPolygon();
  }

  int nPolygons = multiSurface->numGeometries();
  if ( nPolygons < 1 )
  {
    return QgsMultiPolygon();
  }

  QgsMultiPolygon mp;
  for ( int i = 0; i < nPolygons; ++i )
  {
    QgsCurvePolygonV2* curvePolygon = dynamic_cast<QgsCurvePolygonV2*>( multiSurface->geometryN( i ) );
    if ( !curvePolygon )
    {
      continue;
    }
    QgsPolygonV2* polygon = curvePolygon->toPolygon();
    QgsPolygon poly;
    convertPolygon( *polygon, poly );
    delete polygon;
    mp.append( poly );
  }
  return mp;
}

double QgsGeometry::area()
{
  return 0; //todo...
#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return -1.0;

  double area;

  try
  {
    if ( GEOSArea( mGeos, &area ) == 0 )
      return -1.0;
  }
  CATCH_GEOS( -1.0 )

  return area;
#endif //0
}

double QgsGeometry::length()
{
  return 0; //todo...
#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return -1.0;

  double length;

  try
  {
    if ( GEOSLength( mGeos, &length ) == 0 )
      return -1.0;
  }
  CATCH_GEOS( -1.0 )

  return length;
#endif //0
}
double QgsGeometry::distance( QgsGeometry& geom )
{
  return 0; //todo...

#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geom.mDirtyGeos )
    geom.exportWkbToGeos();

  if ( !mGeos || !geom.mGeos )
    return -1.0;

  double dist = -1.0;

  try
  {
    GEOSDistance( mGeos, geom.mGeos, &dist );
  }
  CATCH_GEOS( -1.0 )

  return dist;
#endif //0
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments )
{
  return 0; //todo...
#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSBuffer( mGeos, distance, segments ) );
  }
  CATCH_GEOS( 0 )
#endif //0
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit )
{
  return 0; //todo...

#if 0
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
 ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSBufferWithStyle( mGeos, distance, segments, endCapStyle, joinStyle, mitreLimit ) );
  }
  CATCH_GEOS( 0 )
#else
  return 0;
#endif
#endif //0
}

QgsGeometry* QgsGeometry::offsetCurve( double distance, int segments, int joinStyle, double mitreLimit )
{
  return 0;

#if 0
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
 ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos || this->type() != QGis::Line )
    return 0;

  try
  {
    return fromGeosGeom( GEOSOffsetCurve( mGeos, distance, segments, joinStyle, mitreLimit ) );
  }
  CATCH_GEOS( 0 )
#else
  return 0;
#endif
#endif //0
}

QgsGeometry* QgsGeometry::simplify( double tolerance )
{
  return 0; //todo...
#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSTopologyPreserveSimplify( mGeos, tolerance ) );
  }
  CATCH_GEOS( 0 )
#endif //0
}

QgsGeometry* QgsGeometry::centroid()
{
  return 0; //todo...
#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSGetCentroid( mGeos ) );
  }
  CATCH_GEOS( 0 )
#endif //0
}

QgsGeometry* QgsGeometry::pointOnSurface()
{
  return 0; //todo...
#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSPointOnSurface( mGeos ) );
  }
  CATCH_GEOS( 0 )
#endif //0
}

QgsGeometry* QgsGeometry::convexHull()
{
  return 0; //todo...

#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSConvexHull( mGeos ) );
  }
  CATCH_GEOS( 0 )
#endif //0
}

QgsGeometry* QgsGeometry::interpolate( double distance )
{
  return 0; //todo...

#if 0
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
    ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=2)))
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSInterpolate( mGeos, distance ) );
  }
  CATCH_GEOS( 0 )
#else
  QgsMessageLog::logMessage( QObject::tr( "GEOS prior to 3.2 doesn't support GEOSInterpolate" ), QObject::tr( "GEOS" ) );
#endif
#endif //0
}

QgsGeometry* QgsGeometry::intersection( QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry->d || !geometry->d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.intersection( *( geometry->d->geometry ) );
  return new QgsGeometry( resultGeom );
}

QgsGeometry* QgsGeometry::combine( QgsGeometry* geometry )
{
  return 0; //todo...

#if 0
  if ( !geometry )
    return NULL;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geometry->mDirtyGeos )
    geometry->exportWkbToGeos();

  if ( !mGeos || !geometry->mGeos )
    return 0;

  try
  {
    GEOSGeometry* unionGeom = GEOSUnion( mGeos, geometry->mGeos );
    if ( !unionGeom )
      return 0;

    if ( type() == QGis::Line )
    {
      GEOSGeometry* mergedGeom = GEOSLineMerge( unionGeom );
      if ( mergedGeom )
      {
        GEOSGeom_destroy( unionGeom );
        unionGeom = mergedGeom;
      }
    }
    return fromGeosGeom( unionGeom );
  }
  CATCH_GEOS( new QgsGeometry( *this ) ) //return this geometry if union not possible
#endif //0
}

QgsGeometry* QgsGeometry::difference( QgsGeometry* geometry )
{
  return 0; //todo...
#if 0
  if ( !geometry )
    return NULL;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geometry->mDirtyGeos )
    geometry->exportWkbToGeos();

  if ( !mGeos || !geometry->mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSDifference( mGeos, geometry->mGeos ) );
  }
  CATCH_GEOS( 0 )
#endif //0
}

QgsGeometry* QgsGeometry::symDifference( QgsGeometry* geometry )
{
  return 0; //todo...
#if 0
  if ( !geometry )
    return NULL;

  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( geometry->mDirtyGeos )
    geometry->exportWkbToGeos();

  if ( !mGeos || !geometry->mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSSymDifference( mGeos, geometry->mGeos ) );
  }
  CATCH_GEOS( 0 )
#endif //0
}

QList<QgsGeometry*> QgsGeometry::asGeometryCollection() const
{
  return QList<QgsGeometry*>(); //todo...

#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return QList<QgsGeometry*>();

  int type = GEOSGeomTypeId( mGeos );
  QgsDebugMsg( "geom type: " + QString::number( type ) );

  QList<QgsGeometry*> geomCollection;

  if ( type != GEOS_MULTIPOINT &&
       type != GEOS_MULTILINESTRING &&
       type != GEOS_MULTIPOLYGON &&
       type != GEOS_GEOMETRYCOLLECTION )
  {
    // we have a single-part geometry - put there a copy of this one
    geomCollection.append( new QgsGeometry( *this ) );
    return geomCollection;
  }

  int count = GEOSGetNumGeometries( mGeos );
  QgsDebugMsg( "geom count: " + QString::number( count ) );

  for ( int i = 0; i < count; ++i )
  {
    const GEOSGeometry * geometry = GEOSGetGeometryN( mGeos, i );
    geomCollection.append( fromGeosGeom( GEOSGeom_clone( geometry ) ) );
  }

  return geomCollection;
#endif //0
}

bool QgsGeometry::deleteRing( int ringNum, int partNum )
{
  return false; //todo...

#if 0
  if ( ringNum <= 0 || partNum < 0 )
    return false;

  switch ( wkbType() )
  {
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
    {
      if ( partNum != 0 )
        return false;

      QgsPolygon polygon = asPolygon();
      if ( ringNum >= polygon.count() )
        return false;

      polygon.remove( ringNum );

      QgsGeometry* g2 = QgsGeometry::fromPolygon( polygon );
      *this = *g2;
      delete g2;
      return true;
    }

    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      QgsMultiPolygon mpolygon = asMultiPolygon();

      if ( partNum >= mpolygon.count() )
        return false;

      if ( ringNum >= mpolygon[partNum].count() )
        return false;

      mpolygon[partNum].remove( ringNum );

      QgsGeometry* g2 = QgsGeometry::fromMultiPolygon( mpolygon );
      *this = *g2;
      delete g2;
      return true;
    }

    default:
      return false; // only makes sense with polygons and multipolygons
  }
#endif //0
}

bool QgsGeometry::deletePart( int partNum )
{
  return 0; //todo....

#if 0
  if ( partNum < 0 )
    return false;

  switch ( wkbType() )
  {
    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
    {
      QgsMultiPoint mpoint = asMultiPoint();

      if ( partNum >= mpoint.size() || mpoint.size() == 1 )
        return false;

      mpoint.remove( partNum );

      QgsGeometry* g2 = QgsGeometry::fromMultiPoint( mpoint );
      *this = *g2;
      delete g2;
      break;
    }

    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      QgsMultiPolyline mline = asMultiPolyline();

      if ( partNum >= mline.size() || mline.size() == 1 )
        return false;

      mline.remove( partNum );

      QgsGeometry* g2 = QgsGeometry::fromMultiPolyline( mline );
      *this = *g2;
      delete g2;
      break;
    }

    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      QgsMultiPolygon mpolygon = asMultiPolygon();

      if ( partNum >= mpolygon.size() || mpolygon.size() == 1 )
        return false;

      mpolygon.remove( partNum );

      QgsGeometry* g2 = QgsGeometry::fromMultiPolygon( mpolygon );
      *this = *g2;
      delete g2;
      break;
    }

    default:
      // single part geometries are ignored
      return false;
  }

  return true;
#endif //0
}

/** Return union of several geometries - try to use unary union if available (GEOS >= 3.3) otherwise use a cascade of unions.
 *  Takes ownership of passed geometries, returns a new instance */
static GEOSGeometry* _makeUnion( QList<GEOSGeometry*> geoms )
{
  return 0; //todo...

#if 0
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && (((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)) || (GEOS_VERSION_MAJOR>3))
  GEOSGeometry* geomCollection = 0;
  geomCollection = createGeosCollection( GEOS_GEOMETRYCOLLECTION, geoms.toVector() );
  GEOSGeometry* geomUnion = GEOSUnaryUnion( geomCollection );
  GEOSGeom_destroy( geomCollection );
  return geomUnion;
#else
  GEOSGeometry* geomCollection = geoms.takeFirst();

  while ( !geoms.isEmpty() )
  {
    GEOSGeometry* g = geoms.takeFirst();
    GEOSGeometry* geomCollectionNew = GEOSUnion( geomCollection, g );
    GEOSGeom_destroy( geomCollection );
    GEOSGeom_destroy( g );
    geomCollection = geomCollectionNew;
  }

  return geomCollection;
#endif
#endif //0
}

int QgsGeometry::avoidIntersections( QMap<QgsVectorLayer*, QSet< QgsFeatureId > > ignoreFeatures )
{
  return 0; //todo...

#if 0
  int returnValue = 0;

  //check if g has polygon type
  if ( type() != QGis::Polygon )
    return 1;

  QGis::WkbType geomTypeBeforeModification = wkbType();

  //read avoid intersections list from project properties
  bool listReadOk;
  QStringList avoidIntersectionsList = QgsProject::instance()->readListEntry( "Digitizing", "/AvoidIntersectionsList", QStringList(), &listReadOk );
  if ( !listReadOk )
    return true; //no intersections stored in project does not mean error

  QList<GEOSGeometry*> nearGeometries;

  //go through list, convert each layer to vector layer and call QgsVectorLayer::removePolygonIntersections for each
  QgsVectorLayer* currentLayer = 0;
  QStringList::const_iterator aIt = avoidIntersectionsList.constBegin();
  for ( ; aIt != avoidIntersectionsList.constEnd(); ++aIt )
  {
    currentLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( *aIt ) );
    if ( currentLayer )
    {
      QgsFeatureIds ignoreIds;
      QMap<QgsVectorLayer*, QSet<qint64> >::const_iterator ignoreIt = ignoreFeatures.find( currentLayer );
      if ( ignoreIt != ignoreFeatures.constEnd() )
        ignoreIds = ignoreIt.value();

      QgsFeatureIterator fi = currentLayer->getFeatures( QgsFeatureRequest( boundingBox() )
                              .setFlags( QgsFeatureRequest::ExactIntersect )
                              .setSubsetOfAttributes( QgsAttributeList() ) );
      QgsFeature f;
      while ( fi.nextFeature( f ) )
      {
        if ( ignoreIds.contains( f.id() ) )
          continue;

        if ( !f.geometry() )
          continue;

        nearGeometries << GEOSGeom_clone( f.geometry()->asGeos() );
      }
    }
  }

  if ( nearGeometries.isEmpty() )
    return 0;

  GEOSGeometry* nearGeometriesUnion = 0;
  GEOSGeometry* geomWithoutIntersections = 0;
  try
  {
    nearGeometriesUnion = _makeUnion( nearGeometries );
    geomWithoutIntersections = GEOSDifference( asGeos(), nearGeometriesUnion );

    fromGeos( geomWithoutIntersections );

    GEOSGeom_destroy( nearGeometriesUnion );
  }
  catch ( GEOSException &e )
  {
    if ( nearGeometriesUnion )
      GEOSGeom_destroy( nearGeometriesUnion );
    if ( geomWithoutIntersections )
      GEOSGeom_destroy( geomWithoutIntersections );

    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return 3;
  }

  //make sure the geometry still has the same type (e.g. no change from polygon to multipolygon)
  if ( wkbType() != geomTypeBeforeModification )
    return 2;

  return returnValue;
#endif //0
}

void QgsGeometry::validateGeometry( QList<Error> &errors )
{
//todo // QgsGeometryValidator::validateGeometry( this, errors );
}

bool QgsGeometry::isGeosValid()
{
  return false; //todo...
#if 0
  try
  {
    const GEOSGeometry *g = asGeos();

    if ( !g )
      return false;

    return GEOSisValid( g );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }
#endif //0
}

bool QgsGeometry::isGeosEqual( QgsGeometry &g )
{
  return false; //todo... //return geosRelOp( GEOSEquals, this, &g );
}

bool QgsGeometry::isGeosEmpty()
{
  return false; //todo...
#if 0
  try
  {
    const GEOSGeometry *g = asGeos();

    if ( !g )
      return false;

    return GEOSisEmpty( g );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }
#endif //0
}

QgsGeometry *QgsGeometry::unaryUnion( const QList<QgsGeometry*> &geometryList )
{
  return 0; //todo...
#if 0
  QList<GEOSGeometry*> geoms;
  foreach ( QgsGeometry* g, geometryList )
  {
    geoms.append( GEOSGeom_clone( g->asGeos() ) );
  }
  GEOSGeometry *geomUnion = _makeUnion( geoms );
  QgsGeometry *ret = new QgsGeometry();
  ret->fromGeos( geomUnion );
  return ret;
#endif //0
}

void QgsGeometry::convertToStraightSegment()
{
  if ( !d || !d->geometry )
  {
    return;
  }

  detach();
  QGis::WkbType geomType = d->geometry->wkbType();
  if ( geomType == QGis::WKBCompoundCurve || geomType == QGis::WKBCompoundCurveZ ||
       geomType == QGis::WKBCompoundCurveM || geomType == QGis::WKBCompoundCurveZM ||
       geomType == QGis::WKBCircularString || geomType == QGis::WKBCircularStringZ ||
       geomType == QGis::WKBCircularStringM || geomType == QGis::WKBCircularStringZM )
  {
    QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( d->geometry );
    if ( !curve )
    {
      return ;
    }
    d->geometry = curve->curveToLine();
    delete curve;
  }
  else if ( geomType == QGis::WKBCurvePolygon || geomType == QGis::WKBCurvePolygonZ ||
            geomType == QGis::WKBCurvePolygonM || geomType == QGis::WKBCurvePolygonZM )
  {
    QgsCurvePolygonV2* curvePolygon = dynamic_cast<QgsCurvePolygonV2*>( d->geometry );
    if ( !curvePolygon )
    {
      return;
    }
    d->geometry = curvePolygon->toPolygon();
    delete curvePolygon;
  }
  else //no segmentation needed
  {
    return;
  }

  //compoundcurve / circularstring /multicurve ?

  //curve polygon / multisurface?
  delete[] mWkb;
  mWkb = 0;
  mWkbSize = 0;
  GEOSGeom_destroy( mGeos );
  mGeos = 0;
}

int QgsGeometry::transform( const QgsCoordinateTransform& ct )
{
  if ( !d || !d->geometry )
  {
    return 1;
  }

  detach();
  d->geometry->transform( ct );
  return 0;
}

void QgsGeometry::mapToPixel( const QgsMapToPixel& mtp )
{
  if ( d && d->geometry )
  {
    detach();
    d->geometry->mapToPixel( mtp );
  }
}

void QgsGeometry::clip( const QgsRectangle& rect )
{
  if ( d && d->geometry )
  {
    detach();
    d->geometry->clip( rect );
  }
}

void QgsGeometry::draw( QPainter& p ) const
{
  if ( d && d->geometry )
  {
    d->geometry->draw( p );
  }
}

bool QgsGeometry::vertexIdFromVertexNr( int nr, QgsVertexId& id ) const
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QList< QList< QList< QgsPointV2 > > > coords;
  d->geometry->coordinateSequence( coords );

  int vertexCount = 0;
  for ( int feature = 0; feature < coords.size(); ++feature )
  {
    const QList< QList< QgsPointV2 > >& featureCoords = coords.at( feature );
    for ( int ring = 0; ring < featureCoords.size(); ++ring )
    {
      const QList< QgsPointV2 >& ringCoords = featureCoords.at( ring );
      for ( int vertex = 0; vertex < ringCoords.size(); ++vertex )
      {
        if ( vertexCount == nr )
        {
          id.feature = feature;
          id.ring = ring;
          id.vertex = vertex;
          return true;
        }
        ++vertexCount;
      }
    }
  }
  return false;
}

int QgsGeometry::vertexNrFromVertexId( const QgsVertexId& id ) const
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QList< QList< QList< QgsPointV2 > > > coords;
  d->geometry->coordinateSequence( coords );

  int vertexCount = 0;
  for ( int feature = 0; feature < coords.size(); ++feature )
  {
    const QList< QList< QgsPointV2 > >& featureCoords = coords.at( feature );
    for ( int ring = 0; ring < featureCoords.size(); ++ring )
    {
      const QList< QgsPointV2 >& ringCoords = featureCoords.at( ring );
      for ( int vertex = 0; vertex < ringCoords.size(); ++vertex )
      {
        if ( vertex == id.vertex && ring == id.ring && feature == id.feature )
        {
          return vertexCount;
        }
        ++vertexCount;
      }
    }
  }
  return -1;
}

void QgsGeometry::convertPointList( const QList<QgsPoint>& input, QList<QgsPointV2>& output )
{
  output.clear();
  QList<QgsPoint>::const_iterator it = input.constBegin();
  for ( ; it != input.constEnd(); ++it )
  {
    output.append( QgsPointV2( it->x(), it->y() ) );
  }
}

void QgsGeometry::convertPointList( const QList<QgsPointV2>& input, QList<QgsPoint>& output )
{
  output.clear();
  QList<QgsPointV2>::const_iterator it = input.constBegin();
  for ( ; it != input.constEnd(); ++it )
  {
    output.append( QgsPoint( it->x(), it->y() ) );
  }
}

void QgsGeometry::convertToPolyline( const QList<QgsPointV2>& input, QgsPolyline& output )
{
  output.clear();
  output.resize( input.size() );

  for ( int i = 0; i < input.size(); ++i )
  {
    const QgsPointV2& pt = input.at( i );
    output[i].setX( pt.x() );
    output[i].setY( pt.y() );
  }
}

void QgsGeometry::convertPolygon( const QgsPolygonV2& input, QgsPolygon& output )
{
  output.clear();
  QList< QList< QList< QgsPointV2 > > > coord;
  input.coordinateSequence( coord );
  if ( coord.size() < 1 )
  {
    return;
  }

  const QList< QList< QgsPointV2 > >& rings = coord[0];
  output.resize( rings.size() );
  for ( int i = 0; i < rings.size(); ++i )
  {
    convertToPolyline( rings[i], output[i] );
  }
}
