/***************************************************************************
                              qgsgeometryfactory.cpp
                              ----------------------
  begin                : November 20th, 2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryfactory.h"
#include "qgslinestring.h"

QgsGeometryFactory::QgsGeometryFactory()
{
}

QgsGeometryFactory::~QgsGeometryFactory()
{
}


QgsLineString* QgsGeometryFactory::createLineString( QPolygonF* vertices, QVector<double>* zValues, QVector<double>* mValues )
{
  return new QgsLineString( vertices, zValues, mValues );
}
