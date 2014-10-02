/***************************************************************************
                         qgslinestringv2.h
                         -----------------
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

#ifndef QGSLINESTRINGV2_H
#define QGSLINESTRINGV2_H

#include "qgscurvev2.h"
#include "qgswkbptr.h"
#include <QPolygonF>

class QgsLineStringV2: public QgsCurveV2
{
  public:
    QgsLineStringV2();
    ~QgsLineStringV2();

    virtual QString geometryType() const { return "LineString"; }
    virtual int dimension() const { return 1; }
    virtual QgsAbstractGeometryV2* clone() const;

    virtual void fromWkb( const unsigned char* wkb );
    virtual void fromWkt( const QString& wkt );

    virtual QString asText( int precision = 17 ) const;
    virtual unsigned char* asBinary( int& binarySize ) const;
    virtual int wkbSize() const;
    virtual QString asGML() const;

    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual bool isClosed() const;
    virtual bool isRing() const;
    virtual QgsLineStringV2* curveToLine() const;

    int numPoints() const;
    QgsPointV2 pointN( int i ) const;
    QList<QgsPointV2> points() const;

    void setPoints( const QList<QgsPointV2> points );
    void append( const QgsLineStringV2* line );

    virtual QgsRectangle calculateBoundingBox() const;

    void fromWkbPoints( QGis::WkbType type, const QgsConstWkbPtr& wkb );

  private:
    void importVerticesFromWkb( const QgsConstWkbPtr& wkb );

    QPolygonF mCoords;
    QVector<double> mZ;
    QVector<double> mM;
};

#endif // QGSLINESTRINGV2_H
