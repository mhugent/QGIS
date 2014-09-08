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

class QgsCurveV2: public QgsAbstractGeometryV2
{
  public:
    QgsCurveV2(): QgsAbstractGeometryV2() {}
    virtual ~QgsCurveV2() {}
    virtual double length() const = 0;
    virtual QgsPointV2 startPoint() const = 0;
    virtual QgsPointV2 endPoint() const = 0;
    virtual bool isClosed() const = 0;
    virtual bool isRing() const = 0;
    virtual QgsCurveV2* curveToLine() const = 0;
};

#endif // QGSCURVEV2_H
