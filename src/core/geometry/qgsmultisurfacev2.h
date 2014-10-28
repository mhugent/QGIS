/***************************************************************************
                        qgsmultisurfacev2.h
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

#ifndef QGSMULTISURFACEV2_H
#define QGSMULTISURFACEV2_H

#include "qgsgeometrycollectionv2.h"

class QgsMultiSurfaceV2: public QgsGeometryCollectionV2
{
  public:
    QgsMultiSurfaceV2();
    virtual ~QgsMultiSurfaceV2();

    virtual QString geometryType() const { return "MultiSurface"; }
    void fromWkt( const QString& wkt );

    QString asText( int precision = 17 ) const;
    QString asGML() const;

    QgsAbstractGeometryV2* clone() const;
};

#endif // QGSMULTISURFACEV2_H
