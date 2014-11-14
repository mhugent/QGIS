/***************************************************************************
                        qgsmultipointv2.cpp
  -------------------------------------------------------------------
Date                 : 29 Oct 2014
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

#include "qgsmultipointv2.h"

QgsMultiPointV2::QgsMultiPointV2(): QgsGeometryCollectionV2()
{

}

QgsMultiPointV2::~QgsMultiPointV2()
{

}

void QgsMultiPointV2::fromWkt( const QString& wkt )
{
  //todo...
}

QString QgsMultiPointV2::asText( int precision ) const
{
  return QString();
}

QString QgsMultiPointV2::asGML() const
{
  return QString();
}

QgsAbstractGeometryV2* QgsMultiPointV2::clone() const
{
  int nGeometries = mGeometries.size();
  QgsMultiPointV2* geom = new QgsMultiPointV2();
  geom->mGeometries.resize( nGeometries );

  for ( int i = 0; i < nGeometries; ++i )
  {
    geom->mGeometries[i] = mGeometries[i]->clone();
  }

  geom->mWkbType = mWkbType;
  return geom;
}

bool QgsMultiPointV2::addGeometry( QgsAbstractGeometryV2* g )
{
  if ( !g || g->geometryType() != "Point" )
  {
    delete g;
    return false;
  }
  return QgsGeometryCollectionV2::addGeometry( g );
}
