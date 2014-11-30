/***************************************************************************
                         qgscompoundcurvev2.cpp
                         ----------------------
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

#include "qgscompoundcurvev2.h"
#include "qgsapplication.h"
#include "qgscircularstringv2.h"
#include "qgslinestringv2.h"
#include "qgswkbptr.h"
#include <QPainter>
#include <QPainterPath>


QgsCompoundCurveV2::QgsCompoundCurveV2(): QgsCurveV2()
{

}

QgsCompoundCurveV2::~QgsCompoundCurveV2()
{
  removeCurves();
}

QgsCompoundCurveV2::QgsCompoundCurveV2( const QgsCompoundCurveV2& curve ): QgsCurveV2( curve )
{
  int nC = curve.nCurves();
  for ( int i = 0; i < nC; ++i )
  {
    mCurves.append( dynamic_cast<QgsCurveV2*>( curve.curveAt( i )->clone() ) );
  }
}

QgsCompoundCurveV2& QgsCompoundCurveV2::operator=( const QgsCompoundCurveV2 & curve )
{
  if ( &curve != this )
  {
    QgsCurveV2::operator=( curve );
    removeCurves();
    int nC = curve.nCurves();
    for ( int i = 0; i < nC; ++i )
    {
      mCurves.append( dynamic_cast<QgsCurveV2*>( curve.curveAt( i )->clone() ) );
    }
  }
  return *this;
}

QgsAbstractGeometryV2* QgsCompoundCurveV2::clone() const
{
  return new QgsCompoundCurveV2( *this );
}

void QgsCompoundCurveV2::fromWkb( const unsigned char* wkb )
{
  removeCurves();
  if ( !wkb )
  {
    return;
  }

  QgsConstWkbPtr wkbPtr( wkb + 1 );

  //type
  wkbPtr >> mWkbType;
  int nCurves;
  wkbPtr >> nCurves;

  QgsCurveV2* currentCurve = 0;
  int currentCurveSize = 0;

  for ( int i = 0; i < nCurves; ++i )
  {
    wkbPtr += 1; //skip endian
    QGis::WkbType curveType;
    wkbPtr >> curveType;
    wkbPtr -= ( 1  + sizeof( int ) );

    if ( curveType == QGis::WKBLineString || curveType == QGis::WKBLineStringZ || curveType == QGis::WKBLineStringM ||
         curveType == QGis::WKBLineStringZM || curveType == QGis::WKBLineString25D )
    {
      currentCurve = new QgsLineStringV2();
    }
    else if ( curveType == QGis::WKBCircularString || curveType == QGis::WKBCircularStringZ || curveType == QGis::WKBCircularStringZM ||
              curveType == QGis::WKBCircularStringM )
    {
      currentCurve = new QgsCircularStringV2();
    }
    else
    {
      return;
    }

    currentCurve->fromWkb( wkbPtr );
    currentCurveSize = currentCurve->wkbSize();
    mCurves.append( currentCurve );
    wkbPtr += currentCurveSize;
  }
}

void QgsCompoundCurveV2::fromWkt( const QString& wkt )
{
}

int QgsCompoundCurveV2::wkbSize() const
{
  int size = 0;
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    size += ( *curveIt )->wkbSize();
  }
  size += ( 1 + 2 * sizeof( int ) );
  return size;
}

QString QgsCompoundCurveV2::asText( int precision ) const
{
  return ""; //todo...
}

unsigned char* QgsCompoundCurveV2::asBinary( int& binarySize ) const
{
  QList< QPair< unsigned char*, int > > curveWkb;
  int currentCurveWkbSize = 0;
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    curveWkb.push_back( qMakePair(( *curveIt )->asBinary( currentCurveWkbSize ), currentCurveWkbSize ) );
  }

  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  char byteOrder = QgsApplication::endian();
  QgsWkbPtr wkb( geomPtr );
  wkb << byteOrder;
  wkb << wkbType();
  wkb << mCurves.size();

  QList< QPair< unsigned char*, int > >::const_iterator wkbIt = curveWkb.constBegin();
  for ( ; wkbIt != curveWkb.constEnd(); ++wkbIt )
  {
    memcpy( wkb, wkbIt->first, wkbIt->second );
    wkb += wkbIt->second;
    delete wkbIt->first;
  }

  return geomPtr;
}

QString QgsCompoundCurveV2::asGML() const
{
  return ""; //todo...
}

double QgsCompoundCurveV2::length() const
{
  return 0; //todo...
}

QgsPointV2 QgsCompoundCurveV2::startPoint() const
{
  if ( mCurves.size() < 1 )
  {
    return QgsPointV2();
  }
  return mCurves.at( 0 )->startPoint();
}

QgsPointV2 QgsCompoundCurveV2::endPoint() const
{
  if ( mCurves.size() < 1 )
  {
    return QgsPointV2();
  }
  return mCurves.at( mCurves.size() - 1 )->endPoint();
}

void QgsCompoundCurveV2::points( QList<QgsPointV2>& pts ) const
{
  pts.clear();
  if ( mCurves.size() < 1 )
  {
    return;
  }

  mCurves[0]->points( pts );
  for ( int i = 1; i < mCurves.size(); ++i )
  {
    QList<QgsPointV2> pList;
    mCurves[i]->points( pList );
    pList.removeFirst(); //first vertex already added in previous line
    pts.append( pList );
  }
}

int QgsCompoundCurveV2::numPoints() const
{
  int nPoints = 0;
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    nPoints += ( *curveIt )->numPoints();
  }
  return nPoints;
}

QgsLineStringV2* QgsCompoundCurveV2::curveToLine() const
{
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  QgsLineStringV2* line = 0;
  QgsLineStringV2* currentLine = 0;
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    if ( curveIt == mCurves.constBegin() )
    {
      line = ( *curveIt )->curveToLine();
      if ( !line )
      {
        return 0;
      }
    }
    else
    {
      currentLine = ( *curveIt )->curveToLine();
      line->append( currentLine );
      delete currentLine;
    }
  }
  return line;
}

const QgsCurveV2* QgsCompoundCurveV2::curveAt( int i ) const
{
  if ( i >= mCurves.size() )
  {
    return 0;
  }
  return mCurves.at( i );
}

void QgsCompoundCurveV2::addCurve( QgsCurveV2* c )
{
  if ( c )
  {
    mCurves.append( c );
  }
}

void QgsCompoundCurveV2::removeCurves()
{
  qDeleteAll( mCurves );
  mCurves.clear();
}

QgsRectangle QgsCompoundCurveV2::calculateBoundingBox() const
{
  if ( mCurves.size() < 1 )
  {
    return QgsRectangle();
  }

  QgsRectangle bbox = mCurves.at( 0 )->boundingBox();

  QgsRectangle curveBBox;
  for ( int i = 1; i < mCurves.size(); ++i )
  {
    curveBBox = mCurves.at( i )->boundingBox();
    bbox.combineExtentWith( &curveBBox );
  }
  return bbox;
}

void QgsCompoundCurveV2::draw( QPainter& p ) const
{
  QList< QgsCurveV2* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->draw( p );
  }
}

void QgsCompoundCurveV2::transform( const QgsCoordinateTransform& ct )
{
  QList< QgsCurveV2* >::iterator it = mCurves.begin();
  for ( ; it != mCurves.end(); ++it )
  {
    ( *it )->transform( ct );
  }
}

void QgsCompoundCurveV2::mapToPixel( const QgsMapToPixel& mtp )
{
  QList< QgsCurveV2* >::iterator it = mCurves.begin();
  for ( ; it != mCurves.end(); ++it )
  {
    ( *it )->mapToPixel( mtp );
  }
}



void QgsCompoundCurveV2::addToPainterPath( QPainterPath& path ) const
{
  QPainterPath pp;
  QList< QgsCurveV2* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->addToPainterPath( pp );
  }
  path.addPath( pp );
}

void QgsCompoundCurveV2::drawAsPolygon( QPainter& p ) const
{
  QPainterPath pp;
  QList< QgsCurveV2* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->addToPainterPath( pp );
  }
  p.drawPath( pp );
}

bool QgsCompoundCurveV2::insertVertex( const QgsVertexId& position, const QgsPointV2& vertex )
{
  QList< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
  QList< QPair<int, QgsVertexId> >::const_iterator idIt = curveIds.constBegin();
  for ( ; idIt != curveIds.constEnd(); ++idIt )
  {
    mCurves[idIt->first]->insertVertex( idIt->second, vertex );
  }
  return curveIds.size() > 0;
}

bool QgsCompoundCurveV2::moveVertex( const QgsVertexId& position, const QgsPointV2& newPos )
{
  QList< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
  QList< QPair<int, QgsVertexId> >::const_iterator idIt = curveIds.constBegin();
  for ( ; idIt != curveIds.constEnd(); ++idIt )
  {
    mCurves[idIt->first]->moveVertex( idIt->second, newPos );
  }
  return curveIds.size() > 0;
}

bool QgsCompoundCurveV2::deleteVertex( const QgsVertexId& position )
{
  QList< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
  QList< QPair<int, QgsVertexId> >::const_iterator idIt = curveIds.constBegin();
  for ( ; idIt != curveIds.constEnd(); ++idIt )
  {
    mCurves[idIt->first]->deleteVertex( idIt->second );
  }
  return curveIds.size() > 0;
}

QList< QPair<int, QgsVertexId> > QgsCompoundCurveV2::curveVertexId( const QgsVertexId& id ) const
{
  QList< QPair<int, QgsVertexId> > curveIds;

  int currentVertexIndex = 0;
  for ( int i = 0; i < mCurves.size(); ++i )
  {
    int increment = mCurves.at( i )->numPoints() - 1;
    if ( id.vertex >= currentVertexIndex && id.vertex <= currentVertexIndex + increment )
    {
      int curveVertexId = id.vertex - currentVertexIndex;
      QgsVertexId vid; vid.feature = 0; vid.ring = 0; vid.vertex = curveVertexId;
      curveIds.append( qMakePair( i, vid ) );
      if ( curveVertexId == increment && i < ( mCurves.size() - 1 ) ) //add first vertex of next curve
      {
        vid.vertex = 0;
        curveIds.append( qMakePair( i + 1, vid ) );
      }
    }
    currentVertexIndex += increment;
  }

  return curveIds;
}

