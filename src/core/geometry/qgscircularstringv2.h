/***************************************************************************
                         qgscircularstringv2.h
                         ---------------------
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

#ifndef QGSCIRCULARSTRING_H
#define QGSCIRCULARSTRING_H

#include "qgscurvev2.h"
#include <QVector>

class QgsCircularStringV2: public QgsCurveV2
{
  public:
    QgsCircularStringV2();
    ~QgsCircularStringV2();

    virtual QString geometryType() const { return "CircularString"; }
    virtual int dimension() const { return 1; }
    virtual QgsAbstractGeometryV2* clone() const;

    virtual void fromWkb( const unsigned char * wkb, size_t length );
    virtual void fromGeos( GEOSGeometry* geos );
    virtual void fromWkt( const QString& wkt );
    virtual int wkbSize( const unsigned char* wkb ) const;

    virtual QString asText( int precision = 17 ) const;
    virtual unsigned char* asBinary( int& binarySize ) const;
    virtual QString asGML() const;

    int numPoints() const;
    QgsPointV2 pointN( int i ) const;

    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual bool isClosed() const;
    virtual bool isRing() const;
    virtual QgsLineStringV2* curveToLine() const;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;
};

#endif // QGSCIRCULARSTRING_H
