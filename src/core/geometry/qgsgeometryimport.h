/***************************************************************************
                           qgsgeometryimport.h
                         -----------------------
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

#ifndef QGSGEOMETRYIMPORT_H
#define QGSGEOMETRYIMPORT_H

#include <QString>

class QgsAbstractGeometryV2;
typedef struct GEOSGeom_t GEOSGeometry;

class QgsGeometryImport
{
  public:
    static QgsAbstractGeometryV2* geomFromWkb( const unsigned char* wkb, int wkbSize );
    static QgsAbstractGeometryV2* geomFromWkt( const QString& text );
    static QgsAbstractGeometryV2* geomFromGeos( const GEOSGeometry* geos );
};

#endif // QGSGEOMETRYIMPORT_H
