/***************************************************************************
                         qgscurvepolygonv2.cpp
                         ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscurvepolygonv2.h"
#include "qgsapplication.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgsgeometryutils.h"
#include "qgslinestringv2.h"
#include "qgspolygonv2.h"
#include "qgswkbptr.h"
#include "QPainter"
#include "QPainterPath"

QgsCurvePolygonV2::QgsCurvePolygonV2(): QgsSurfaceV2(), mExteriorRing( 0 )
{

}

QgsCurvePolygonV2::~QgsCurvePolygonV2()
{
  removeRings();
}

QgsCurvePolygonV2::QgsCurvePolygonV2( const QgsCurvePolygonV2& p ): mExteriorRing( 0 )
{
  *this = p;
}

QgsCurvePolygonV2& QgsCurvePolygonV2::operator=( const QgsCurvePolygonV2 & p )
{
  QgsSurfaceV2::operator=( p );
  removeRings();
  if ( p.mExteriorRing )
  {
    mExteriorRing = dynamic_cast<QgsCurveV2*>( p.mExteriorRing->clone() );
  }

  int nInteriorRings = p.mInteriorRings.size();
  for ( int i = 0; i < nInteriorRings; ++i )
  {
    mInteriorRings.push_back( dynamic_cast<QgsCurveV2*>( p.mInteriorRings[i]->clone() ) );
  }
  return *this;
}

QgsAbstractGeometryV2* QgsCurvePolygonV2::clone() const
{
  return new QgsCurvePolygonV2( *this );
}

void QgsCurvePolygonV2::fromWkb( const unsigned char* wkb )
{
  removeRings();
  if ( !wkb )
  {
    return;
  }

  QgsConstWkbPtr wkbPtr( wkb + 1 );

  //type
  wkbPtr >> mWkbType;
  int nRings;
  wkbPtr >> nRings;

  QgsCurveV2* currentCurve = 0;
  int currentCurveSize = 0;

  for ( int i = 0; i < nRings; ++i )
  {
    wkbPtr += 1; //skip endian
    QgsWKBTypes::Type curveType;
    wkbPtr >> curveType;
    wkbPtr -= ( 1  + sizeof( int ) );

    if ( curveType == QgsWKBTypes::LineString || curveType == QgsWKBTypes::LineStringZ || curveType == QgsWKBTypes::LineStringM ||
         curveType == QgsWKBTypes::LineStringZM || curveType == QgsWKBTypes::LineString25D )
    {
      currentCurve = new QgsLineStringV2();
    }
    else if ( curveType == QgsWKBTypes::CircularString || curveType == QgsWKBTypes::CircularStringZ || curveType == QgsWKBTypes::CircularStringZM ||
              curveType == QgsWKBTypes::CircularStringM )
    {
      currentCurve = new QgsCircularStringV2();
    }
    else if ( curveType == QgsWKBTypes::CompoundCurve || curveType == QgsWKBTypes::CompoundCurveZ || curveType == QgsWKBTypes::CompoundCurveZM )
    {
      currentCurve = new QgsCompoundCurveV2();
    }

    currentCurve->fromWkb( wkbPtr );
    currentCurveSize = currentCurve->wkbSize();
    if ( i == 0 )
    {
      mExteriorRing = currentCurve;
    }
    else
    {
      mInteriorRings.append( currentCurve );
    }
    wkbPtr += currentCurveSize;
  }
}

void QgsCurvePolygonV2::fromWkt( const QString& wkt )
{

}

QgsRectangle QgsCurvePolygonV2::calculateBoundingBox() const
{
  if ( mExteriorRing )
  {
    return mExteriorRing->calculateBoundingBox();
  }
  return QgsRectangle();
}

QString QgsCurvePolygonV2::asText( int precision ) const
{
  QString wkt( "CURVEPOLYGON(" );
  if ( mExteriorRing )
  {
    wkt.append( mExteriorRing->asText( precision ) );
  }
  for ( int i = 0; i < mInteriorRings.size(); ++i )
  {
    wkt.append( "," );
    wkt.append( mInteriorRings[i]->asText( precision ) );
  }
  wkt.append( ")" );
  return wkt;
}

unsigned char* QgsCurvePolygonV2::asBinary( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  char byteOrder = QgsApplication::endian();
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();
  unsigned char* currentWkbPtr = wkb;

  if ( mExteriorRing )
  {
    addRingWkb( &currentWkbPtr, mExteriorRing );
  }

  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    addRingWkb( &currentWkbPtr, ( *ringIt ) );
  }

  return geomPtr;
}

void QgsCurvePolygonV2::addRingWkb( unsigned char** wkb, const QgsCurveV2* ring ) const
{
  if ( !ring )
  {
    return;
  }

  int ringWkbSize = 0;
  unsigned char* ringWkb = ring->asBinary( ringWkbSize );
  memcpy( *wkb, ringWkb, ringWkbSize );
  *wkb += ringWkbSize;
  delete[] ringWkb;
}

int QgsCurvePolygonV2::wkbSize() const
{
  int size = 0;
  if ( mExteriorRing )
  {
    size += mExteriorRing->wkbSize();
  }
  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    size += ( *ringIt )->wkbSize();
  }

  size += ( 1 + 2 * sizeof( int ) );
  return size;
}

QString QgsCurvePolygonV2::asGML() const
{
  return QString();
}

double QgsCurvePolygonV2::area() const
{
  return 0.0;
}

double QgsCurvePolygonV2::perimeter() const
{
  return 0.0;
}

QgsPointV2 QgsCurvePolygonV2::centroid() const
{
  return QgsPointV2( 0, 0 );
}

QgsPointV2 QgsCurvePolygonV2::pointOnSurface() const
{
  return QgsPointV2( 0, 0 );
}

void QgsCurvePolygonV2::removeRings()
{
  delete mExteriorRing;
  mExteriorRing = 0;
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();
}

QgsPolygonV2* QgsCurvePolygonV2::toPolygon() const
{
  if ( !mExteriorRing )
  {
    return 0;
  }

  QgsPolygonV2* poly = new QgsPolygonV2();
  poly->setExteriorRing( mExteriorRing->curveToLine() );

  QList<QgsCurveV2*> rings;
  QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    rings.push_back(( *it )->curveToLine() );
  }
  poly->setInteriorRings( rings );
  return poly;
}

int QgsCurvePolygonV2::numInteriorRings() const
{
  return mInteriorRings.size();
}

const QgsCurveV2* QgsCurvePolygonV2::exteriorRing() const
{
  return mExteriorRing;
}

const QgsCurveV2* QgsCurvePolygonV2::interiorRing( int i ) const
{
  if ( i >= mInteriorRings.size() )
  {
    return 0;
  }
  return mInteriorRings.at( i );
}

void QgsCurvePolygonV2::setExteriorRing( QgsCurveV2* ring )
{
  if ( !ring )
  {
    return;
  }
  delete mExteriorRing;
  mExteriorRing = ring;

  //set proper wkb type
  if ( geometryType() == "Polygon" )
  {
    setZMTypeFromSubGeometry( ring, QgsWKBTypes::Polygon );
  }
  else if ( geometryType() == "CurvePolygon" )
  {
    setZMTypeFromSubGeometry( ring, QgsWKBTypes::CurvePolygon );
  }
}

void QgsCurvePolygonV2::setInteriorRings( QList<QgsCurveV2*> rings )
{
  qDeleteAll( mInteriorRings );
  mInteriorRings = rings;
}

void QgsCurvePolygonV2::addInteriorRing( QgsCurveV2* ring )
{
  mInteriorRings.append( ring );
}

bool QgsCurvePolygonV2::removeInteriorRing( int nr )
{
  if ( nr >= mInteriorRings.size() )
  {
    return false;
  }
  mInteriorRings.removeAt( nr );
  return true;
}

void QgsCurvePolygonV2::draw( QPainter& p ) const
{
  if ( mInteriorRings.size() < 1 )
  {
    mExteriorRing->drawAsPolygon( p );
  }
  else
  {
    QPainterPath path;
    mExteriorRing->addToPainterPath( path );

    QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
    for ( ; it != mInteriorRings.constEnd(); ++it )
    {
      ( *it )->addToPainterPath( path );
    }
    p.drawPath( path );
  }
}

void QgsCurvePolygonV2::mapToPixel( const QgsMapToPixel& mtp )
{
  if ( mExteriorRing )
  {
    mExteriorRing->mapToPixel( mtp );
  }

  QList<QgsCurveV2*>::iterator it = mInteriorRings.begin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    ( *it )->mapToPixel( mtp );
  }
}

void QgsCurvePolygonV2::transform( const QgsCoordinateTransform& ct )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( ct );
  }

  QList<QgsCurveV2*>::iterator it = mInteriorRings.begin();
  for ( ; it != mInteriorRings.end(); ++it )
  {
    ( *it )->transform( ct );
  }
}

void QgsCurvePolygonV2::translate( double dx, double dy, double dz, double dm )
{
  if ( mExteriorRing )
  {
    mExteriorRing->translate( dx, dy, dz, dm );
  }
  QList<QgsCurveV2*>::iterator it = mInteriorRings.begin();
  for ( ; it != mInteriorRings.end(); ++it )
  {
    ( *it )->translate( dx, dy, dz, dm );
  }
}

void QgsCurvePolygonV2::coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const
{
  coord.clear();

  QList< QList< QgsPointV2 > > coordinates;
  QList< QgsPointV2 > ringCoords;
  if ( mExteriorRing )
  {
    mExteriorRing->points( ringCoords );
    coordinates.append( ringCoords );
  }

  QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    ( *it )->points( ringCoords );
    coordinates.append( ringCoords );
  }
  coord.append( coordinates );
}

double QgsCurvePolygonV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  if ( !mExteriorRing )
  {
    return 0.0;
  }
  QList<QgsCurveV2*> segmentList;
  segmentList.append( mExteriorRing );
  segmentList.append( mInteriorRings );
  return QgsGeometryUtils::closestSegmentFromComponents( segmentList, QgsGeometryUtils::RING, pt, segmentPt,  vertexAfter, leftOf, epsilon );
}

bool QgsCurvePolygonV2::nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const
{
  if ( !mExteriorRing || id.ring >= ( mInteriorRings.size() + 1 ) )
  {
    return false;
  }

  if ( id.ring < 0 )
  {
    id.ring = 0; id.vertex = -1;
    if ( id.part < 0 )
    {
      id.part = 0;
    }
    return mExteriorRing->nextVertex( id, vertex );
  }
  else
  {
    QgsCurveV2* ring = 0;
    if ( id.ring == 0 )
    {
      ring = mExteriorRing;
    }
    else
    {
      ring = mInteriorRings.at( id.ring - 1 );
    }

    if ( ring->nextVertex( id, vertex ) )
    {
      return true;
    }
    ++id.ring; id.vertex = -1;
    if ( id.ring >= ( mInteriorRings.size() + 1 ) )
    {
      return false;
    }
    ring = mInteriorRings.at( id.ring - 1 );
    return ring->nextVertex( id, vertex );
  }
}

bool QgsCurvePolygonV2::insertVertex( const QgsVertexId& position, const QgsPointV2& vertex )
{
  if ( !mExteriorRing || position.ring < 0 || position.vertex < 0 )
  {
    return false;
  }

  QgsVertexId ringId = position;
  if ( position.ring == 0 )
  {
    ringId.ring = 0;
    mExteriorRing->insertVertex( ringId, vertex );
  }
  else
  {
    ringId.ring = position.ring - 1;
    mInteriorRings[ position.ring - 1 ]->insertVertex( ringId, vertex );
  }

  return false;
}

bool QgsCurvePolygonV2::moveVertex( const QgsVertexId& position, const QgsPointV2& newPos )
{
  QList< QgsVertexId > vertexIds = ringVertexIds( position );
  QList< QgsVertexId >::const_iterator vIt = vertexIds.constBegin();
  for ( ; vIt != vertexIds.constEnd(); ++vIt )
  {
    if ( vIt->ring == 0 )
    {
      mExteriorRing->moveVertex( *vIt, newPos );
    }
    else
    {
      mInteriorRings[vIt->ring - 1]->moveVertex( *vIt, newPos );
    }
  }
  mBoundingBox = QgsRectangle();
  return ( vertexIds.size() > 0 );
}

bool QgsCurvePolygonV2::deleteVertex( const QgsVertexId& position )
{
  QList< QgsVertexId > vertexIds = ringVertexIds( position );
  QList< QgsVertexId >::const_iterator vIt = vertexIds.constBegin();
  for ( ; vIt != vertexIds.constEnd(); ++vIt )
  {
    if ( vIt->ring == 0 )
    {
      mExteriorRing->deleteVertex( *vIt );
    }
    else
    {
      mInteriorRings[vIt->ring - 1]->deleteVertex( *vIt );
    }
  }
  return ( vertexIds.size() > 0 );
}

QList< QgsVertexId > QgsCurvePolygonV2::ringVertexIds( const QgsVertexId& id ) const
{
  QList< QgsVertexId > vertexList;

  int numRingPoints;
  if ( mExteriorRing && id.ring == 0 )
  {
    numRingPoints = mExteriorRing->numPoints();
  }
  else if ( id.ring > 0 && id.ring <= mInteriorRings.size() )
  {
    numRingPoints = mInteriorRings[id.ring - 1]->numPoints();
  }

  if ( numRingPoints < 1 )
  {
    return vertexList;
  }

  vertexList.append( id );
  if ( id.vertex == 0 )
  {
    QgsVertexId vId = id;
    vId.vertex = numRingPoints - 1;
    vertexList.append( vId );
  }
  else if ( id.vertex == ( numRingPoints - 1 ) )
  {
    QgsVertexId vId = id;
    vId.vertex = 0;
    vertexList.append( vId );
  }
  return vertexList;
}


