
/***************************************************************************
                        qgsmultisurfacev2.cpp
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
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

#include "qgsmultisurfacev2.h"
#include "qgssurfacev2.h"

QgsMultiSurfaceV2::QgsMultiSurfaceV2(): QgsGeometryCollectionV2()
{

}

QgsMultiSurfaceV2::~QgsMultiSurfaceV2()
{

}

void QgsMultiSurfaceV2::fromWkt( const QString& wkt )
{

}

QString QgsMultiSurfaceV2::asText( int precision ) const
{
  return QString(); //todo...
}

QString QgsMultiSurfaceV2::asGML() const
{
  return QString(); //todo...
}

QgsAbstractGeometryV2* QgsMultiSurfaceV2::clone() const
{
  int nGeometries = mGeometries.size();
  QgsMultiSurfaceV2* geom = new QgsMultiSurfaceV2();
  geom->mGeometries.resize( nGeometries );

  for ( int i = 0; i < nGeometries; ++i )
  {
    geom->mGeometries[i] = mGeometries[i]->clone();
  }

  return geom;
}

bool QgsMultiSurfaceV2::addGeometry( QgsAbstractGeometryV2* g )
{
  bool isSurface = dynamic_cast<QgsSurfaceV2*>( g );
  if ( !g || !isSurface )
  {
    delete g;
    return false;
  }

  //all geometries polygons?
  bool isPolygon = true;
  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    if (( *it )->geometryType() != "Polygon" )
    {
      isPolygon = false;
      break;
    }
  }

  setZMTypeFromSubGeometry( g, isPolygon ? QgsWKBTypes::MultiPolygon : QgsWKBTypes::MultiSurface );
  return QgsGeometryCollectionV2::addGeometry( g );
}
