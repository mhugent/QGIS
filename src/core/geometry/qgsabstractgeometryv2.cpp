/***************************************************************************
                        qgsabstractgeometryv2.cpp
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
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

#include "qgsabstractgeometryv2.h"
#include "qgsgeos.h"
#include "qgsmaptopixel.h"
#include <limits>
#include <QTransform>

QgsAbstractGeometryV2::QgsAbstractGeometryV2(): mWkbType( QgsWKBTypes::Unknown )
{
}

QgsAbstractGeometryV2::~QgsAbstractGeometryV2()
{
}

QgsAbstractGeometryV2::QgsAbstractGeometryV2( const QgsAbstractGeometryV2& geom )
{
  mWkbType = geom.mWkbType;
}

QgsAbstractGeometryV2& QgsAbstractGeometryV2::operator=( const QgsAbstractGeometryV2 & geom )
{
  mWkbType = geom.mWkbType;
  return *this;
}

QgsRectangle QgsAbstractGeometryV2::boundingBox() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox();
  }
  return mBoundingBox;
}

bool QgsAbstractGeometryV2::is3D() const
{
  return(( mWkbType >= 1001 && mWkbType >= 1012 ) || ( mWkbType > 3000 ) );
}

bool QgsAbstractGeometryV2::isMeasure() const
{
  return ( mWkbType >= 2001 && mWkbType <= 3012 );
}

void QgsAbstractGeometryV2::setZMTypeFromSubGeometry( const QgsAbstractGeometryV2* subgeom, QgsWKBTypes::Type baseGeomType )
{
  if ( !subgeom )
  {
    return;
  }

  bool hasZ = subgeom->is3D();
  bool hasM = subgeom->isMeasure();

  if ( hasZ && hasM )
  {
    mWkbType = ( QgsWKBTypes::Type )( baseGeomType + 3000 );
  }
  else if ( hasZ )
  {
    mWkbType = ( QgsWKBTypes::Type )( baseGeomType + 1000 );
  }
  else if ( hasM )
  {
    mWkbType = ( QgsWKBTypes::Type )( baseGeomType + 2000 );
  }
  else
  {
    mWkbType = baseGeomType;
  }
}

QgsRectangle QgsAbstractGeometryV2::calculateBoundingBox() const
{
  double xmin = std::numeric_limits<double>::max();
  double ymin = std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  QList< QList< QList< QgsPointV2 > > > coordinates;
  coordinateSequence( coordinates );
  double x, y;

  QList< QList< QList< QgsPointV2 > > >::const_iterator fIt = coordinates.constBegin();
  for ( ; fIt != coordinates.constEnd(); ++fIt )
  {
    const QList< QList< QgsPointV2 > >& feature = *fIt;
    QList< QList< QgsPointV2 > >::const_iterator ringIt = feature.constBegin();
    for ( ; ringIt != feature.constEnd(); ++ringIt )
    {
      const QList< QgsPointV2 >& ring = *ringIt;
      QList< QgsPointV2 >::const_iterator ringIt = ring.constBegin();
      for ( ; ringIt != ring.constEnd(); ++ringIt )
      {
        x = ringIt->x();
        y = ringIt->y();

        if ( x < xmin )
          xmin = x;
        if ( x > xmax )
          xmax = x;
        if ( y < ymin )
          ymin = y;
        if ( y > ymax )
          ymax = y;
      }
    }
  }

  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QgsPointV2 QgsAbstractGeometryV2::pointAt( const QgsVertexId& id ) const
{
  QList< QList< QList< QgsPointV2 > > > coordinates;
  coordinateSequence( coordinates );

  if ( id.part >= coordinates.size() )
  {
    return QgsPointV2();
  }
  const QList< QList< QgsPointV2 > >& part = coordinates.at( id.part );
  if ( id.ring >= part.size() )
  {
    return QgsPointV2();
  }
  const QList< QgsPointV2 >& ring = part.at( id.ring );
  if ( id.vertex >= ring.size() )
  {
    return QgsPointV2();
  }
  return ring.at( id.vertex );
}

int QgsAbstractGeometryV2::nCoordinates() const
{
  QList< QList< QList< QgsPointV2 > > > coordinates;
  coordinateSequence( coordinates );
  int nCoords = 0;

  QList< QList< QList< QgsPointV2 > > >::const_iterator partIt = coordinates.constBegin();
  for ( ; partIt != coordinates.constEnd(); ++partIt )
  {
    const QList< QList< QgsPointV2 > >& part = *partIt;
    QList< QList< QgsPointV2 > >::const_iterator ringIt = part.constBegin();
    for ( ; ringIt != part.constEnd(); ++ringIt )
    {
      nCoords += ringIt->size();
    }
  }

  return nCoords;
}

/*void QgsAbstractGeometryV2::mapToPixel( const QgsMapToPixel& mtp )
{
    transform( mtp.transform() );
}*/
