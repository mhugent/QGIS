/***************************************************************************
                         qgscurvev2.h
                         ------------
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

#ifndef QGSCURVEV2_H
#define QGSCURVEV2_H

#include "qgsabstractgeometryv2.h"
#include "qgspointv2.h"

class QgsLineStringV2;
class QPainterPath;

class QgsCurveV2: public QgsAbstractGeometryV2
{
  public:
    QgsCurveV2();
    virtual ~QgsCurveV2();
    virtual double length() const = 0;
    virtual QgsPointV2 startPoint() const = 0;
    virtual QgsPointV2 endPoint() const = 0;
    virtual bool isClosed() const = 0;
    virtual bool isRing() const = 0;
    virtual QgsLineStringV2* curveToLine() const = 0;

    virtual void addToPainterPath( QPainterPath& path ) const = 0;
    virtual void drawAsPolygon( QPainter& p ) const = 0;
    virtual void points( QList<QgsPointV2>& pt ) const = 0;

    virtual void coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const;
};

#endif // QGSCURVEV2_H
