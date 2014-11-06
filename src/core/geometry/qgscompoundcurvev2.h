/***************************************************************************
                         qgscompoundcurvev2.h
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

#ifndef QGSCOMPOUNDCURVEV2_H
#define QGSCOMPOUNDCURVEV2_H

#include "qgscurvev2.h"

class QgsCompoundCurveV2: public QgsCurveV2
{
  public:
    QgsCompoundCurveV2();
    QgsCompoundCurveV2( const QgsCompoundCurveV2& curve );
    QgsCompoundCurveV2& operator=( const QgsCompoundCurveV2& curve );
    ~QgsCompoundCurveV2();

    virtual QString geometryType() const { return "CompoundCurve"; }
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
    virtual void points( QList<QgsPointV2>& pts ) const;
    virtual bool isClosed() const;
    virtual bool isRing() const;
    virtual QgsLineStringV2* curveToLine() const;
    int nCurves() const { return mCurves.size(); }
    const QgsCurveV2* curveAt( int i ) const;

    virtual QgsRectangle calculateBoundingBox() const;

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct );
    void mapToPixel( const QgsMapToPixel& mtp );
    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

  private:
    QList< QgsCurveV2* > mCurves;

    void removeCurves();
};

#endif // QGSCOMPOUNDCURVEV2_H
