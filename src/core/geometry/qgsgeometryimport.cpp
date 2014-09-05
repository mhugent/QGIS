/***************************************************************************
                           qgsgeometryimport.cpp
                         ------------------------
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

#include "qgsgeometryimport.h"
#include "qgspointv2.h"

QgsAbstractGeometryV2* QgsGeometryImport::geomFromWkb( const unsigned char* wkb, int wkbSize )
{
  if ( wkbSize < ( 1 + sizeof( int ) ) )
  {
    return 0;
  }

  //find out type (bytes 2-5)
  int type;
  memcpy( &type, wkb + 1, sizeof( int ) );
  QgsAbstractGeometryV2* geom = 0;

  switch ( type )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
      geom = new QgsPointV2();
      geom->fromWkb( wkb, wkbSize );
      return geom;
    default:
      return geom;
  }
}

QgsAbstractGeometryV2* QgsGeometryImport::geomFromWkt( const QString& text )
{
  return 0; //soon...
}

QgsAbstractGeometryV2* QgsGeometryImport::geomFromGeos( const GEOSGeometry* geos )
{
  return 0; //soon...
}
