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

QgsAbstractGeometryV2::QgsAbstractGeometryV2( int coordDimension, bool hasM ): mRefs( 0 ), mCoordDimension( coordDimension ), mHasM( hasM )
{

}

QgsAbstractGeometryV2::~QgsAbstractGeometryV2()
{

}

void QgsAbstractGeometryV2::ref()
{
  ++mRefs;
}

void QgsAbstractGeometryV2::deref()
{
  --mRefs;
  if ( mRefs <= 0 )
  {
    delete this;
  }
}

#if 0
QgsAbstractGeometryV2* QgsAbstractGeometryV2::geomFromWkb( unsigned char * wkb, size_t length )
{
  if ( length < ( 1 + sizeof( int ) ) )
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
        geom->fromWkb( wkb, length );
        return geom;
  default:
  return geom;
  }
}

QgsAbstractGeometryV2* QgsAbstractGeometryV2::geomFromWkt( const QString& wkt )
{
  return 0; //soon
}

QgsAbstractGeometryV2* QgsAbstractGeometryV2::geomFromGeos( const GEOSGeometry* geos )
{
  return 0; //soon
}
#endif //0
