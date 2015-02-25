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

#include "qgsapplication.h"
#include "qgsabstractgeometryv2.h"
#include "qgswkbptr.h"
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
  clear();
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
  return(( mWkbType >= 1001 && mWkbType <= 1012 ) || ( mWkbType > 3000 ) );
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

  QgsVertexId id;
  QgsPointV2 vertex;
  double x, y;
  while ( nextVertex( id, vertex ) )
  {
    x = vertex.x();
    y = vertex.y();
    if ( x < xmin )
      xmin = x;
    if ( x > xmax )
      xmax = x;
    if ( y < ymin )
      ymin = y;
    if ( y > ymax )
      ymax = y;
  }

  return QgsRectangle( xmin, ymin, xmax, ymax );
}

QgsPointV2 QgsAbstractGeometryV2::vertexAt( const QgsVertexId& id ) const
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

QString QgsAbstractGeometryV2::wktTypeStr() const
{
  QString wkt = geometryType();
  if ( is3D() )
    wkt += "Z";
  if ( isMeasure() )
    wkt += "M";
  return wkt;
}

QList<QgsPointV2> QgsAbstractGeometryV2::pointsFromWKB( const QgsConstWkbPtr &wkb, bool is3D, bool isMeasure, bool endianSwap )
{
  quint32 count;
  wkb >> count;
  if ( endianSwap )
    QgsApplication::endian_swap( count );
  QList<QgsPointV2> points;
  points.reserve( count );
  for ( quint32 i = 0; i < count; ++i )
  {
    double x, y, z = 0., m = 0.;
    wkb >> x;
    wkb >> y;
    if ( is3D )
      wkb >> z;
    if ( isMeasure )
      wkb >> m;
    if ( endianSwap )
    {
      QgsApplication::endian_swap( x );
      QgsApplication::endian_swap( y );
      QgsApplication::endian_swap( z );
      QgsApplication::endian_swap( m );
    }
    if ( is3D )
    {
      if ( isMeasure )
        points.append( QgsPointV2( x, y, z, m ) );
      else
        points.append( QgsPointV2( x, y, z, false ) );
    }
    else
    {
      if ( isMeasure )
        points.append( QgsPointV2( x, y, m, true ) );
      else
        points.append( QgsPointV2( x, y ) );
    }
  }
  return points;
}

QList<QgsPointV2> QgsAbstractGeometryV2::pointsFromWKT( const QString &wktCoordinateList, bool is3D, bool isMeasure )
{
  int dim = 2 + is3D + isMeasure;
  QList<QgsPointV2> points;
  foreach ( const QString& pointCoordinates, wktCoordinateList.split( ",", QString::SkipEmptyParts ) )
  {
    QStringList coordinates = pointCoordinates.split( " ", QString::SkipEmptyParts );
    if ( coordinates.size() != dim )
      continue;

    int idx = 0;
    double x = coordinates[idx++].toDouble();
    double y = coordinates[idx++].toDouble();
    double z = is3D ? coordinates[idx++].toDouble() : 0.;
    double m = isMeasure ? coordinates[idx++].toDouble() : 0.;

    if ( is3D )
    {
      if ( isMeasure )
        points.append( QgsPointV2( x, y, z, m ) );
      else
        points.append( QgsPointV2( x, y, z, false ) );
    }
    else
    {
      if ( isMeasure )
        points.append( QgsPointV2( x, y, m, true ) );
      else
        points.append( QgsPointV2( x, y ) );
    }
  }
  return points;
}

void QgsAbstractGeometryV2::pointsToWKB( QgsWkbPtr& wkb, const QList<QgsPointV2> &points, bool is3D, bool isMeasure )
{
  wkb << static_cast<quint32>( points.size() );
  foreach ( const QgsPointV2& point, points )
  {
    wkb << point.x() << point.y();
    if ( is3D )
    {
      wkb << point.z();
    }
    if ( isMeasure )
    {
      wkb << point.m();
    }
  }
}

QString QgsAbstractGeometryV2::pointsToWKT( const QList<QgsPointV2>& points, int precision, bool is3D, bool isMeasure )
{
  QString wkt = "(";
  foreach ( const QgsPointV2& p, points )
  {
    wkt += qgsDoubleToString( p.x(), precision );
    wkt += " " + qgsDoubleToString( p.y(), precision );
    if ( is3D )
      wkt += " " + qgsDoubleToString( p.z(), precision );
    if ( isMeasure )
      wkt += " " + qgsDoubleToString( p.m(), precision );
    wkt += ", ";
  }
  if ( wkt.endsWith( ", " ) )
    wkt.chop( 2 ); // Remove last ", "
  wkt += ")";
  return wkt;
}

QDomElement QgsAbstractGeometryV2::pointsToGML2( const QList<QgsPointV2>& points, QDomDocument& doc, int precision, const QString &ns )
{
  QDomElement elemCoordinates = doc.createElementNS( ns, "coordinates" );

  QString strCoordinates;

  foreach ( const QgsPointV2& p, points )
    strCoordinates += qgsDoubleToString( p.x(), precision ) + "," + qgsDoubleToString( p.y(), precision ) + " ";

  if ( strCoordinates.endsWith( " " ) )
    strCoordinates.chop( 1 ); // Remove trailing space

  elemCoordinates.appendChild( doc.createTextNode( strCoordinates ) );
  return elemCoordinates;
}

QDomElement QgsAbstractGeometryV2::pointsToGML3( const QList<QgsPointV2>& points, QDomDocument& doc, int precision, const QString &ns, bool is3D )
{
  QDomElement elemPosList = doc.createElementNS( ns, "posList" );
  elemPosList.setAttribute( "srsDimension", is3D ? 3 : 2 );

  QString strCoordinates;
  foreach ( const QgsPointV2& p, points )
  {
    strCoordinates += qgsDoubleToString( p.x(), precision ) + " " + qgsDoubleToString( p.y(), precision ) + " ";
    if ( is3D )
      strCoordinates += qgsDoubleToString( p.z(), precision ) + " ";
  }
  if ( strCoordinates.endsWith( " " ) )
    strCoordinates.chop( 1 ); // Remove trailing space

  elemPosList.appendChild( doc.createTextNode( strCoordinates ) );
  return elemPosList;
}

QString QgsAbstractGeometryV2::pointsToJSON( const QList<QgsPointV2>& points, int precision )
{
  QString json = "[ ";
  foreach ( const QgsPointV2& p, points )
  {
    json += "[" + qgsDoubleToString( p.x(), precision ) + ", " + qgsDoubleToString( p.y(), precision ) + "], ";
  }
  if ( json.endsWith( ", " ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += "]";
  return json;
}

QPair<QgsWKBTypes::Type, QString> QgsAbstractGeometryV2::wktReadBlock( const QString &wkt )
{
  QgsWKBTypes::Type wkbType = QgsWKBTypes::parseType( wkt );

  QRegExp cooRegEx( "^[^\\(]*\\((.*)\\)[^\\)]*$" );
  QString contents = cooRegEx.indexIn( wkt ) >= 0 ? cooRegEx.cap( 1 ) : QString();
  return qMakePair( wkbType, contents );
}

QStringList QgsAbstractGeometryV2::wktGetChildBlocks( const QString &wkt, const QString& defaultType )
{
  int level = 0;
  QString block;
  QStringList blocks;
  for ( int i = 0, n = wkt.length(); i < n; ++i )
  {
    if ( wkt[i] == ',' && level == 0 )
    {
      if ( !block.isEmpty() )
      {
        if ( block.startsWith( "(" ) && !defaultType.isEmpty() )
          block.prepend( defaultType + " " );
        blocks.append( block );
      }
      block.clear();
      continue;
    }
    if ( wkt[i] == '(' )
      ++level;
    else if ( wkt[i] == ')' )
      --level;
    block += wkt[i];
  }
  if ( !block.isEmpty() )
  {
    if ( block.startsWith( "(" ) && !defaultType.isEmpty() )
      block.prepend( defaultType + " " );
    blocks.append( block );
  }
  return blocks;
}

bool QgsAbstractGeometryV2::readWkbHeader( QgsConstWkbPtr& wkbPtr, QgsWKBTypes::Type& wkbType, bool& endianSwap, QgsWKBTypes::Type expectedType )
{
  if ( !static_cast<const unsigned char*>( wkbPtr ) )
  {
    return false;
  }

  char wkbEndian;
  wkbPtr >> wkbEndian;
  endianSwap = wkbEndian != QgsApplication::endian();

  wkbPtr >> wkbType;
  if ( endianSwap )
    QgsApplication::endian_swap( wkbType );

  if ( QgsWKBTypes::flatType( wkbType ) != expectedType )
  {
    wkbType = QgsWKBTypes::Unknown;
    return false;
  }
  return true;
}
