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
    case LineType:
      return 1;
    case PolygonType:
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
    return UnknownGeometryType;
  }
  return it->mGeometryType;
}

void QgsWKBTypes::registerTypes()
{
  //register the known wkb types
  mEntries.insert( Unknown, wkbEntry( "Unknown", false, Unknown, Unknown, Unknown, UnknownGeometryType ) );
  mEntries.insert( NoGeometry, wkbEntry( "NoGeometry", false, NoGeometry, NoGeometry, NoGeometry, NoGeometryType ) );
  //point
  mEntries.insert( Point, wkbEntry( "Point", false, MultiPoint, Point, Point, PointType ) );
  mEntries.insert( PointZ, wkbEntry( "PointZ", false, MultiPointZ, PointZ, Point, PointType ) );
  mEntries.insert( PointM, wkbEntry( "PointM", false, MultiPointM, PointM, Point, PointType ) );
  mEntries.insert( PointZM, wkbEntry( "PointZM", false, MultiPointZM, PointZM, Point, PointType ) );
  mEntries.insert( Point25D, wkbEntry( "Point25D", false, MultiPoint25D, Point25D, Point, PointType ) );
  //linestring
  mEntries.insert( LineString, wkbEntry( "LineString", false, MultiLineString, LineString, LineString, LineType ) );
  mEntries.insert( LineStringZ, wkbEntry( "LineStringZ", false, MultiLineStringZ, LineStringZ, LineString, LineType ) );
  mEntries.insert( LineStringM, wkbEntry( "LineStringM", false, MultiLineStringM, LineStringM, LineString, LineType ) );
  mEntries.insert( LineStringZM, wkbEntry( "LineStringZM", false, MultiLineStringZM, LineStringZM, LineString, LineType ) );
  mEntries.insert( LineString25D, wkbEntry( "LineString25D", false, MultiLineString25D, LineString25D, LineString, LineType ) );
  //circularstring
  mEntries.insert( CircularString, wkbEntry( "CircularString", false, MultiCurve, CircularString, CircularString, LineType ) );
  mEntries.insert( CircularStringZ, wkbEntry( "CircularStringZ", false, MultiCurveZ, CircularStringZ, CircularString, LineType ) );
  mEntries.insert( CircularStringM, wkbEntry( "CircularStringM", false, MultiCurveM, CircularStringM, CircularString, LineType ) );
  mEntries.insert( CircularStringZM, wkbEntry( "CircularStringZM", false, MultiCurveZM, CircularStringZM, CircularString, LineType ) );
  //compoundcurve
  mEntries.insert( CompoundCurve, wkbEntry( "CompoundCurve", false, MultiCurve, CompoundCurve, CompoundCurve, LineType ) );
  mEntries.insert( CompoundCurveZ, wkbEntry( "CompoundCurveZ", false, MultiCurveZ, CompoundCurveZ, CompoundCurve, LineType ) );
  mEntries.insert( CompoundCurveM, wkbEntry( "CompoundCurveM", false, MultiCurveM, CompoundCurveM, CompoundCurve, LineType ) );
  mEntries.insert( CompoundCurveZM, wkbEntry( "CompoundCurveZM", false, MultiCurveZM, CompoundCurveZM, CompoundCurve, LineType ) );
  //polygon
  mEntries.insert( Polygon, wkbEntry( "Polygon", false, MultiPolygon, Polygon, Polygon, PolygonType ) );
  mEntries.insert( PolygonZ, wkbEntry( "PolygonZ", false, MultiPolygonZ, PolygonZ, Polygon, PolygonType ) );
  mEntries.insert( PolygonM, wkbEntry( "PolygonM", false, MultiPolygonM, PolygonM, Polygon, PolygonType ) );
  mEntries.insert( PolygonZM, wkbEntry( "PolygonZM", false, MultiPolygonZM, PolygonZM, Polygon, PolygonType ) );
  mEntries.insert( PolygonZM, wkbEntry( "Polygon25D", false, MultiPolygon25D, Polygon25D, Polygon, PolygonType ) );
  //curvepolygon
  mEntries.insert( CurvePolygon, wkbEntry( "CurvePolygon", false, MultiSurface, CurvePolygon, CurvePolygon, PolygonType ) );
  mEntries.insert( CurvePolygonZ, wkbEntry( "CurvePolygonZ", false, MultiSurfaceZ, CurvePolygonZ, CurvePolygon, PolygonType ) );
  mEntries.insert( CurvePolygonM, wkbEntry( "CurvePolygonM", false, MultiSurfaceM, CurvePolygonM, CurvePolygon, PolygonType ) );
  mEntries.insert( CurvePolygonZM, wkbEntry( "CurvePolygonZM", false, MultiSurfaceZM, CurvePolygonZM, CurvePolygon, PolygonType ) );
  //multipoint
  mEntries.insert( MultiPoint, wkbEntry( "MultiPoint", true, MultiPoint, Point, MultiPoint, PointType ) );
  mEntries.insert( MultiPointZ, wkbEntry( "MultiPointZ", true, MultiPointZ, PointZ, MultiPoint, PointType ) );
  mEntries.insert( MultiPointM, wkbEntry( "MultiPointM", true, MultiPointM, PointM, MultiPoint, PointType ) );
  mEntries.insert( MultiPointZM, wkbEntry( "MultiPointZM", true, MultiPointZM, PointZM, MultiPoint, PointType ) );
  mEntries.insert( MultiPoint25D, wkbEntry( "MultiPoint25D", true, MultiPoint25D, Point25D, MultiPoint, PointType ) );
  //multiline
  mEntries.insert( MultiLineString, wkbEntry( "MultiLineString", true, MultiLineString, LineString, MultiLineString, LineType ) );
  mEntries.insert( MultiLineStringZ, wkbEntry( "MultiLineStringZ", true, MultiLineStringZ, LineStringZ, MultiLineString, LineType ) );
  mEntries.insert( MultiLineStringM, wkbEntry( "MultiLineStringM", true, MultiLineStringM, LineStringM, MultiLineString, LineType ) );
  mEntries.insert( MultiLineStringZM, wkbEntry( "MultiLineStringZM", true, MultiLineStringZM, LineStringZM, MultiLineString, LineType ) );
  mEntries.insert( MultiLineString25D, wkbEntry( "MultiLineString25D", true, MultiLineString25D, LineString25D, MultiLineString, LineType ) );
  //multicurve
  mEntries.insert( MultiCurve, wkbEntry( "MultiCurve", true, MultiCurve, CompoundCurve, MultiCurve, LineType ) );
  mEntries.insert( MultiCurveZ, wkbEntry( "MultiCurveZ", true, MultiCurveZ, CompoundCurveZ, MultiCurve, LineType ) );
  mEntries.insert( MultiCurveM, wkbEntry( "MultiCurveM", true, MultiCurveM, CompoundCurveM, MultiCurve, LineType ) );
  mEntries.insert( MultiCurveZM, wkbEntry( "MultiCurveZM", true, MultiCurveZM, CompoundCurveZM, MultiCurve, LineType ) );
  //multipolygon
  mEntries.insert( MultiPolygon, wkbEntry( "MultiPolygon", true, MultiPolygon, Polygon, MultiPolygon, PolygonType ) );
  mEntries.insert( MultiPolygonZ, wkbEntry( "MultiPolygonZ", true, MultiPolygonZ, PolygonZ, MultiPolygon, PolygonType ) );
  mEntries.insert( MultiPolygonM, wkbEntry( "MultiPolygonM", true, MultiPolygonM, PolygonM, MultiPolygon, PolygonType ) );
  mEntries.insert( MultiPolygonZM, wkbEntry( "MultiPolygonZM", true, MultiPolygonZM, PolygonZM, MultiPolygon, PolygonType ) );
  mEntries.insert( MultiPolygon25D, wkbEntry( "MultiPolygon25D", true, MultiPolygon25D, Polygon25D, MultiPolygon, PolygonType ) );
  //multisurface
  mEntries.insert( MultiSurface, wkbEntry( "MultiSurface", true, MultiSurface, CurvePolygon, MultiSurface, PolygonType ) );
  mEntries.insert( MultiSurfaceZ, wkbEntry( "MultiSurfaceZ", true, MultiSurfaceZ, CurvePolygonZ, MultiSurface, PolygonType ) );
  mEntries.insert( MultiSurfaceM, wkbEntry( "MultiSurfaceM", true, MultiSurfaceM, CurvePolygonM, MultiSurface, PolygonType ) );
  mEntries.insert( MultiSurfaceZM, wkbEntry( "MultiSurfaceZM", true, MultiSurfaceZM, CurvePolygonZM, MultiSurface, PolygonType ) );
}
