/***************************************************************************
                         qgspolygonv2.h
                         -------------------
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

#ifndef QGSPOLYGONV2_H
#define QGSPOLYGONV2_H

#include "qgscurvepolygonv2.h"

class QgsPolygonV2: public QgsCurvePolygonV2
{
  public:
    QgsPolygonV2();
    ~QgsPolygonV2();

    virtual void fromWkb( const unsigned char* wkb );
    virtual unsigned char* asBinary( int& binarySize ) const;

    virtual int wkbSize() const;

    virtual QString geometryType() const { return "Polygon"; }

  private:
    static int ringWkbSize( const QgsCurveV2* ring );
    static void ringWkb( unsigned char** wkb, QgsCurveV2* ring );
};
#endif // QGSPOLYGONV2_H
