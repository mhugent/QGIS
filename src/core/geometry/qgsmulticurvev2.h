/***************************************************************************
                        qgsmulticurvev2.h
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
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

#ifndef QGSMULTICURVEV2_H
#define QGSMULTICURVEV2_H

#include "qgsgeometrycollectionv2.h"

class QgsMultiCurveV2: public QgsGeometryCollectionV2
{
  public:
    QgsMultiCurveV2();
    ~QgsMultiCurveV2();

    virtual QString geometryType() const { return "MultiCurve"; }

    void fromWkt( const QString& wkt );

    QString asWkt( int precision = 17 ) const;
    QString asGML() const;

    QgsAbstractGeometryV2* clone() const;

    /**Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometryV2* g );
};

#endif // QGSMULTICURVEV2_H
