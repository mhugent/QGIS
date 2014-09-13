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


QgsCompoundCurveV2::QgsCompoundCurveV2()
{

}

QgsCompoundCurveV2::~QgsCompoundCurveV2()
{
  removeCurves();
}

QgsAbstractGeometryV2* QgsCompoundCurveV2::clone() const
{
  return 0;
}

void QgsCompoundCurveV2::fromWkb( const unsigned char * wkb, size_t length )
{
  removeCurves();
  if ( length < ( 1 + sizeof( int ) ) )
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
    currentCurveSize = currentCurve->wkbSize( wkbPtr );
    currentCurve->fromWkb( wkbPtr, currentCurveSize );
    mCurves.append( currentCurve );
    wkbPtr += currentCurveSize;
  }
}

void QgsCompoundCurveV2::fromGeos( GEOSGeometry* geos )
{

}

void QgsCompoundCurveV2::fromWkt( const QString& wkt )
{

}

QString QgsCompoundCurveV2::asText( int precision ) const
{
  return ""; //todo...
}

unsigned char* QgsCompoundCurveV2::asBinary( int& binarySize ) const
{
  QList< QPair< unsigned char*, int > > curveWkb;
  int curveWkbSize = 0;
  int currentCurveWkbSize = 0;
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    curveWkb.push_back( qMakePair(( *curveIt )->asBinary( currentCurveWkbSize ), currentCurveWkbSize ) );
    curveWkbSize += currentCurveWkbSize;
  }

  binarySize = curveWkbSize + 1 + 2 * sizeof( int );
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
  return QgsPointV2(); //todo...
}

QgsPointV2 QgsCompoundCurveV2::endPoint() const
{
  return QgsPointV2(); //todo...
}

bool QgsCompoundCurveV2::isClosed() const
{
  return false;
}

bool QgsCompoundCurveV2::isRing() const
{
  return false;
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

QgsCurveV2* QgsCompoundCurveV2::curveAt( int i ) const
{
  return 0;
}

void QgsCompoundCurveV2::removeCurves()
{
  qDeleteAll( mCurves );
  mCurves.clear();
}
