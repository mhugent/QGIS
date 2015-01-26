/***************************************************************************
                         qgswkbtypes.cpp
                         ---------------
    begin                : January 2015
    copyright            : (C) 2015 by Marco Hugentobler
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

#include "qgswkbtypes.h"

QgsWKBTypes* QgsWKBTypes::mInstance = 0;

QgsWKBTypes* QgsWKBTypes::instance()
{
  if ( mInstance == 0 )
  {
    mInstance = new QgsWKBTypes();
  }
  return mInstance;
}

QgsWKBTypes::QgsWKBTypes()
{
  registerTypes();
}

QgsWKBTypes::~QgsWKBTypes()
{
}

QgsWKBTypes::Type QgsWKBTypes::singleType( Type type ) const
{
  QMap< Type, wkbEntry >::const_iterator it = mEntries.find( type );
  if ( it == mEntries.constEnd() || it.key() != Unknown )
  {
    return Unknown;
  }
  return ( it->mSingleType );
}

QgsWKBTypes::Type QgsWKBTypes::multiType( Type type ) const
{
  QMap< Type, wkbEntry >::const_iterator it = mEntries.find( type );
  if ( it == mEntries.constEnd() )
  {
    return Unknown;
  }
  return it->mMultiType;
}

QgsWKBTypes::Type QgsWKBTypes::flatType( Type type ) const
{
  QMap< Type, wkbEntry >::const_iterator it = mEntries.find( type );
  if ( it == mEntries.constEnd() )
  {
    return Unknown;
  }
  return it->mFlatType;
}

bool QgsWKBTypes::isSingleType( Type type ) const
{
  return ( type != Unknown && !isMultiType( type ) );
}

bool QgsWKBTypes::isMultiType( Type type ) const
{
  QMap< Type, wkbEntry >::const_iterator it = mEntries.find( type );
  if ( it == mEntries.constEnd() )
  {
    return Unknown;
  }
  return it->mIsMultiType;
}

int QgsWKBTypes::wkbDimensions( Type type ) const
{
  GeometryType gtype = geometryType( type );
  switch ( gtype )
  {
    case LineGeometry:
      return 1;
    case PolygonGeometry:
      return 2;
    default: //point, no geometry, unknown geometry
      return 0;
  }
}

QgsWKBTypes::GeometryType QgsWKBTypes::geometryType( Type type ) const
{
  QMap< Type, wkbEntry >::const_iterator it = mEntries.find( type );
  if ( it == mEntries.constEnd() )
  {
    return UnknownGeometry;
  }
  return it->mGeometryType;
}

void QgsWKBTypes::registerTypes()
{
  //register the known wkb types
  mEntries.insert( Unknown, wkbEntry( "Unknown", false, Unknown, Unknown, Unknown, UnknownGeometry ) );
  mEntries.insert( NoGeometry, wkbEntry( "NoGeometry", false, NoGeometry, NoGeometry, NoGeometry, NullGeometry ) );
  //point
  mEntries.insert( Point, wkbEntry( "Point", false, MultiPoint, Point, Point, PointGeometry ) );
  mEntries.insert( PointZ, wkbEntry( "PointZ", false, MultiPointZ, PointZ, Point, PointGeometry ) );
  mEntries.insert( PointM, wkbEntry( "PointM", false, MultiPointM, PointM, Point, PointGeometry ) );
  mEntries.insert( PointZM, wkbEntry( "PointZM", false, MultiPointZM, PointZM, Point, PointGeometry ) );
  mEntries.insert( Point25D, wkbEntry( "Point25D", false, MultiPoint25D, Point25D, Point, PointGeometry ) );
  //linestring
  mEntries.insert( LineString, wkbEntry( "LineString", false, MultiLineString, LineString, LineString, LineGeometry ) );
  mEntries.insert( LineStringZ, wkbEntry( "LineStringZ", false, MultiLineStringZ, LineStringZ, LineString, LineGeometry ) );
  mEntries.insert( LineStringM, wkbEntry( "LineStringM", false, MultiLineStringM, LineStringM, LineString, LineGeometry ) );
  mEntries.insert( LineStringZM, wkbEntry( "LineStringZM", false, MultiLineStringZM, LineStringZM, LineString, LineGeometry ) );
  mEntries.insert( LineString25D, wkbEntry( "LineString25D", false, MultiLineString25D, LineString25D, LineString, LineGeometry ) );
  //circularstring
  mEntries.insert( CircularString, wkbEntry( "CircularString", false, MultiCurve, CircularString, CircularString, LineGeometry ) );
  mEntries.insert( CircularStringZ, wkbEntry( "CircularStringZ", false, MultiCurveZ, CircularStringZ, CircularString, LineGeometry ) );
  mEntries.insert( CircularStringM, wkbEntry( "CircularStringM", false, MultiCurveM, CircularStringM, CircularString, LineGeometry ) );
  mEntries.insert( CircularStringZM, wkbEntry( "CircularStringZM", false, MultiCurveZM, CircularStringZM, CircularString, LineGeometry ) );
  //compoundcurve
  mEntries.insert( CompoundCurve, wkbEntry( "CompoundCurve", false, MultiCurve, CompoundCurve, CompoundCurve, LineGeometry ) );
  mEntries.insert( CompoundCurveZ, wkbEntry( "CompoundCurveZ", false, MultiCurveZ, CompoundCurveZ, CompoundCurve, LineGeometry ) );
  mEntries.insert( CompoundCurveM, wkbEntry( "CompoundCurveM", false, MultiCurveM, CompoundCurveM, CompoundCurve, LineGeometry ) );
  mEntries.insert( CompoundCurveZM, wkbEntry( "CompoundCurveZM", false, MultiCurveZM, CompoundCurveZM, CompoundCurve, LineGeometry ) );
  //polygon
  mEntries.insert( Polygon, wkbEntry( "Polygon", false, MultiPolygon, Polygon, Polygon, PolygonGeometry ) );
  mEntries.insert( PolygonZ, wkbEntry( "PolygonZ", false, MultiPolygonZ, PolygonZ, Polygon, PolygonGeometry ) );
  mEntries.insert( PolygonM, wkbEntry( "PolygonM", false, MultiPolygonM, PolygonM, Polygon, PolygonGeometry ) );
  mEntries.insert( PolygonZM, wkbEntry( "PolygonZM", false, MultiPolygonZM, PolygonZM, Polygon, PolygonGeometry ) );
  mEntries.insert( PolygonZM, wkbEntry( "Polygon25D", false, MultiPolygon25D, Polygon25D, Polygon, PolygonGeometry ) );
  //curvepolygon
  mEntries.insert( CurvePolygon, wkbEntry( "CurvePolygon", false, MultiSurface, CurvePolygon, CurvePolygon, PolygonGeometry ) );
  mEntries.insert( CurvePolygonZ, wkbEntry( "CurvePolygonZ", false, MultiSurfaceZ, CurvePolygonZ, CurvePolygon, PolygonGeometry ) );
  mEntries.insert( CurvePolygonM, wkbEntry( "CurvePolygonM", false, MultiSurfaceM, CurvePolygonM, CurvePolygon, PolygonGeometry ) );
  mEntries.insert( CurvePolygonZM, wkbEntry( "CurvePolygonZM", false, MultiSurfaceZM, CurvePolygonZM, CurvePolygon, PolygonGeometry ) );
  //multipoint
  mEntries.insert( MultiPoint, wkbEntry( "MultiPoint", true, MultiPoint, Point, MultiPoint, PointGeometry ) );
  mEntries.insert( MultiPointZ, wkbEntry( "MultiPointZ", true, MultiPointZ, PointZ, MultiPoint, PointGeometry ) );
  mEntries.insert( MultiPointM, wkbEntry( "MultiPointM", true, MultiPointM, PointM, MultiPoint, PointGeometry ) );
  mEntries.insert( MultiPointZM, wkbEntry( "MultiPointZM", true, MultiPointZM, PointZM, MultiPoint, PointGeometry ) );
  mEntries.insert( MultiPoint25D, wkbEntry( "MultiPoint25D", true, MultiPoint25D, Point25D, MultiPoint, PointGeometry ) );
  //multiline
  mEntries.insert( MultiLineString, wkbEntry( "MultiLineString", true, MultiLineString, LineString, MultiLineString, LineGeometry ) );
  mEntries.insert( MultiLineStringZ, wkbEntry( "MultiLineStringZ", true, MultiLineStringZ, LineStringZ, MultiLineString, LineGeometry ) );
  mEntries.insert( MultiLineStringM, wkbEntry( "MultiLineStringM", true, MultiLineStringM, LineStringM, MultiLineString, LineGeometry ) );
  mEntries.insert( MultiLineStringZM, wkbEntry( "MultiLineStringZM", true, MultiLineStringZM, LineStringZM, MultiLineString, LineGeometry ) );
  mEntries.insert( MultiLineString25D, wkbEntry( "MultiLineString25D", true, MultiLineString25D, LineString25D, MultiLineString, LineGeometry ) );
  //multicurve
  mEntries.insert( MultiCurve, wkbEntry( "MultiCurve", true, MultiCurve, CompoundCurve, MultiCurve, LineGeometry ) );
  mEntries.insert( MultiCurveZ, wkbEntry( "MultiCurveZ", true, MultiCurveZ, CompoundCurveZ, MultiCurve, LineGeometry ) );
  mEntries.insert( MultiCurveM, wkbEntry( "MultiCurveM", true, MultiCurveM, CompoundCurveM, MultiCurve, LineGeometry ) );
  mEntries.insert( MultiCurveZM, wkbEntry( "MultiCurveZM", true, MultiCurveZM, CompoundCurveZM, MultiCurve, LineGeometry ) );
  //multipolygon
  mEntries.insert( MultiPolygon, wkbEntry( "MultiPolygon", true, MultiPolygon, Polygon, MultiPolygon, PolygonGeometry ) );
  mEntries.insert( MultiPolygonZ, wkbEntry( "MultiPolygonZ", true, MultiPolygonZ, PolygonZ, MultiPolygon, PolygonGeometry ) );
  mEntries.insert( MultiPolygonM, wkbEntry( "MultiPolygonM", true, MultiPolygonM, PolygonM, MultiPolygon, PolygonGeometry ) );
  mEntries.insert( MultiPolygonZM, wkbEntry( "MultiPolygonZM", true, MultiPolygonZM, PolygonZM, MultiPolygon, PolygonGeometry ) );
  mEntries.insert( MultiPolygon25D, wkbEntry( "MultiPolygon25D", true, MultiPolygon25D, Polygon25D, MultiPolygon, PolygonGeometry ) );
  //multisurface
  mEntries.insert( MultiSurface, wkbEntry( "MultiSurface", true, MultiSurface, CurvePolygon, MultiSurface, PolygonGeometry ) );
  mEntries.insert( MultiSurfaceZ, wkbEntry( "MultiSurfaceZ", true, MultiSurfaceZ, CurvePolygonZ, MultiSurface, PolygonGeometry ) );
  mEntries.insert( MultiSurfaceM, wkbEntry( "MultiSurfaceM", true, MultiSurfaceM, CurvePolygonM, MultiSurface, PolygonGeometry ) );
  mEntries.insert( MultiSurfaceZM, wkbEntry( "MultiSurfaceZM", true, MultiSurfaceZM, CurvePolygonZM, MultiSurface, PolygonGeometry ) );
}
