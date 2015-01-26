/***************************************************************************
                        qgsmultipointv2.h
  -------------------------------------------------------------------
Date                 : 29 Oct 2014
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

#ifndef QGSMULTIPOINTV2_H
#define QGSMULTIPOINTV2_H

#include "qgsgeometrycollectionv2.h"

class QgsMultiPointV2: public QgsGeometryCollectionV2
{
  public:
    QgsMultiPointV2();
    ~QgsMultiPointV2();

    virtual QString geometryType() const { return "MultiPoint"; }
    void fromWkt( const QString& wkt );

    QString asWkt( int precision = 17 ) const;
    QString asGML() const;

    QgsAbstractGeometryV2* clone() const;

    /**Adds a geometry and takes ownership. Returns true in case of success*/
    virtual bool addGeometry( QgsAbstractGeometryV2* g );
};

#endif // QGSMULTIPOINTV2_H
