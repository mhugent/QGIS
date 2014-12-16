/***************************************************************************
    qgsmaptooladdcirularstring.h  -  map tool for adding circular strings
    ---------------------
    begin                : December 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLADDCIRCULARSTRING_H
#define QGSMAPTOOLADDCIRCULARSTRING_H

#include "qgsmaptooladdfeature.h"

class QgsGeometryRubberBand;

class QgsMapToolAddCircularString: public QgsMapToolCapture
{
  public:
    QgsMapToolAddCircularString( QgsMapToolAddFeature* parentTool, QgsMapCanvas* canvas );
    ~QgsMapToolAddCircularString();

    void canvasReleaseEvent( QMouseEvent * e );

    void deactivate();

  private:
    QgsMapToolAddCircularString( QgsMapCanvas* canvas = 0 ); //forbidden

    QgsMapToolAddFeature* mParentTool;
    QList< QgsPointV2 > mPoints;
    QgsGeometryRubberBand* mRubberBand;
};

#endif // QGSMAPTOOLADDCIRCULARSTRING_H
