/***************************************************************************
                              qgsgeometryfactory.h
                              --------------------
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

#ifndef QGSGEOMETRYFACTORY_H
#define QGSGEOMETRYFACTORY_H

class QgsGeometry;
class QgsLineString;

/**Class to create geometry objects*/
class QgsGeometryFactory
{
  public:
    QgsGeometryFactory();
    ~QgsGeometryFactory();

    /*static QgsGeometry* fromWkb( unsigned char * wkb, size_t length );
    static QgsGeometry* fromWkt( const QString& wkt );
    static QgsGeometry* fromGeos( const GEOSGeometry* geos );*/

    /**Creates linestring by taking ownership of coordinate vector*/
    QgsLineString* createLineString( QVector<QPointF*>* coordinates, QVector<double>* zValues = 0, QVector<double>* mValues = 0 );
};

#endif // QGSGEOMETRYFACTORY_H
