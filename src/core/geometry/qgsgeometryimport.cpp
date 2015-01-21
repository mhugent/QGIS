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
#include "qgscurvepolygonv2.h"
#include "qgspointv2.h"
#include "qgspolygonv2.h"
#include "qgslinestringv2.h"
#include "qgsmulticurvev2.h"
#include "qgsmultipointv2.h"
#include "qgsmultisurfacev2.h"
#include "qgswkbtypes.h"

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

  type = QgsWKBTypes::instance()->flatType( QgsWKBTypes::Type( type ) );
  switch ( type )
  {
    case QgsWKBTypes::Point:
      geom = new QgsPointV2();
      break;
    case QgsWKBTypes::LineString:
      geom = new QgsLineStringV2();
      break;
    case QgsWKBTypes::CompoundCurve:
      geom = new QgsCompoundCurveV2();
      break;
    case QgsWKBTypes::Polygon:
      geom = new QgsPolygonV2();
      break;
    case QgsWKBTypes::CurvePolygon:
      geom = new QgsCurvePolygonV2();
      break;
    case QgsWKBTypes::MultiLineString:
      geom = new QgsMultiCurveV2();
      break;
    case QgsWKBTypes::MultiPolygon:
      geom = new QgsMultiSurfaceV2();
      break;
    case QgsWKBTypes::MultiPoint:
      geom = new QgsMultiPointV2();
      break;
    default:
      geom = 0;
  }

  if ( geom )
  {
    geom->fromWkb( wkb );
  }
  return geom;
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
  QgsMultiPointV2* mp = new QgsMultiPointV2();
  QgsMultiPoint::const_iterator ptIt = multipoint.constBegin();
  for ( ; ptIt != multipoint.constEnd(); ++ptIt )
  {
    QgsPointV2* pt = new QgsPointV2( ptIt->x(), ptIt->y() );
    mp->addGeometry( pt );
  }
  return mp;
}

QgsAbstractGeometryV2* QgsGeometryImport::fromPolyline( const QgsPolyline& polyline )
{
  return linestringFromPolyline( polyline );
}

QgsAbstractGeometryV2* QgsGeometryImport::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  return 0; //todo...
}

QgsAbstractGeometryV2* QgsGeometryImport::fromPolygon( const QgsPolygon& polygon )
{
  QgsPolygonV2* poly = new QgsPolygonV2();

  QList<QgsCurveV2*> holes;
  for ( int i = 0; i < polygon.size(); ++i )
  {
    if ( i == 0 )
    {
      poly->setExteriorRing( linestringFromPolyline( polygon.at( i ) ) );
    }
    else
    {
      holes.push_back( linestringFromPolyline( polygon.at( i ) ) );
    }
  }
  poly->setInteriorRings( holes );
  return poly;
}

QgsAbstractGeometryV2* QgsGeometryImport::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  return 0; //todo...
}

QgsAbstractGeometryV2* QgsGeometryImport::fromRect( const QgsRectangle& rect )
{
  return 0; //todo...
}

QgsLineStringV2* QgsGeometryImport::linestringFromPolyline( const QgsPolyline& polyline )
{
  QgsLineStringV2* line = new QgsLineStringV2();

  QList<QgsPointV2> points;
  QgsPolyline::const_iterator it = polyline.constBegin();
  for ( ; it != polyline.constEnd(); ++it )
  {
    points.append( QgsPointV2( it->x(), it->y() ) );
  }
  line->setPoints( points );
  return line;
}
