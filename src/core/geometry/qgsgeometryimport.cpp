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
#include "qgscompoundcurvev2.h"
#include "qgspointv2.h"
#include "qgslinestringv2.h"

QgsAbstractGeometryV2* QgsGeometryImport::geomFromWkb( const unsigned char* wkb )
{
  if ( !wkb )
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
    case QGis::WKBPointZ:
    case QGis::WKBPointM:
    case QGis::WKBPointZM:
    case QGis::WKBPoint25D:
      geom = new QgsPointV2();
      geom->fromWkb( wkb );
      return geom;
    case QGis::WKBLineString:
    case QGis::WKBLineStringZ:
    case QGis::WKBLineStringM:
    case QGis::WKBLineStringZM:
    case QGis::WKBLineString25D:
      geom = new QgsLineStringV2();
      geom->fromWkb( wkb );
      return geom;
    case QGis::WKBCompoundCurve:
    case QGis::WKBCompoundCurveZ:
    case QGis::WKBCompoundCurveM:
    case QGis::WKBCompoundCurveZM:
      geom = new QgsCompoundCurveV2();
      geom->fromWkb( wkb );
      return geom;
    default:
      return geom;
  }
}

QgsAbstractGeometryV2* QgsGeometryImport::geomFromWkt( const QString& text )
{
  QgsAbstractGeometryV2* geom = 0;
  if ( text.startsWith( "POINT", Qt::CaseInsensitive ) )
  {
    geom = new QgsPointV2();
  }
  else if ( text.startsWith( "LINESTRING", Qt::CaseInsensitive ) )
  {
    geom = new QgsLineStringV2();
  }

  if ( geom )
  {
    geom->fromWkt( text );
  }
  return geom;
}

QgsAbstractGeometryV2* QgsGeometryImport::fromPoint( const QgsPoint& point )
{
  return new QgsPointV2( point.x(), point.y() );
}

QgsAbstractGeometryV2* QgsGeometryImport::fromMultiPoint( const QgsMultiPoint& multipoint )
{
  return 0; //todo...
}

QgsAbstractGeometryV2* QgsGeometryImport::fromPolyline( const QgsPolyline& polyline )
{
  return 0; //todo...
}

QgsAbstractGeometryV2* QgsGeometryImport::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  return 0; //todo...
}

QgsAbstractGeometryV2* fromPolygon( const QgsPolygon& polygon )
{
  return 0; //todo...
}

QgsAbstractGeometryV2* QgsGeometryImport::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  return 0; //todo...
}

QgsAbstractGeometryV2* QgsGeometryImport::fromRect( const QgsRectangle& rect )
{
  return 0; //todo...
}
