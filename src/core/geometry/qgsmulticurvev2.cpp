/***************************************************************************
                        qgsmulticurvev2.cpp
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

#include "qgsmulticurvev2.h"
#include "qgscurvev2.h"

QgsMultiCurveV2::QgsMultiCurveV2(): QgsGeometryCollectionV2()
{

}

QgsMultiCurveV2::~QgsMultiCurveV2()
{

}

void QgsMultiCurveV2::fromWkt( const QString& wkt )
{

}

QString QgsMultiCurveV2::asText( int precision ) const
{
  return QString(); //todo...
}

QString QgsMultiCurveV2::asGML() const
{
  return QString(); //todo...
}

QgsAbstractGeometryV2* QgsMultiCurveV2::clone() const
{
  int nGeometries = mGeometries.size();
  QgsMultiCurveV2* geom = new QgsMultiCurveV2();
  geom->mGeometries.resize( nGeometries );

  for ( int i = 0; i < nGeometries; ++i )
  {
    geom->mGeometries[i] = mGeometries[i]->clone();
  }

  geom->mWkbType = mWkbType;
  return geom;
}

bool QgsMultiCurveV2::addGeometry( QgsAbstractGeometryV2* g )
{
  bool isCurve = dynamic_cast<QgsCurveV2*>( g );
  if ( !g || !isCurve )
  {
    delete g;
    return false;
  }

  //all geometries linestrings?
  bool isMultiLineString = true;
  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    if (( *it )->geometryType() != "LineString" )
    {
      isMultiLineString = false;
      break;
    }
  }

  setZMTypeFromSubGeometry( g, isMultiLineString ? QGis::WKBMultiLineString : QGis::WKBMultiCurve );
  return QgsGeometryCollectionV2::addGeometry( g );
}
