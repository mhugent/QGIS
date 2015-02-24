/***************************************************************************
    qgsmaptoolcircularstringthreepoints.h  -  map tool for adding circular
    strings with three points per segment
    ---------------------
    begin                : Feb 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLCIRCULARSTRINGTHREEPOINTS_H
#define QGSMAPTOOLCIRCULARSTRINGTHREEPOINTS_H

#include "qgsmaptooladdcircularstring.h"

class QgsMapToolCircularStringThreePoints: public QgsMapToolAddCircularString
{
    public:
        QgsMapToolCircularStringThreePoints( QgsMapToolCapture* parentTool, QgsMapCanvas* canvas, CaptureMode mode = CaptureLine );
        ~QgsMapToolCircularStringThreePoints();

        void canvasReleaseEvent( QMouseEvent * e );
        void canvasMoveEvent( QMouseEvent * e );
};

#endif // QGSMAPTOOLCIRCULARSTRINGTHREEPOINTS_H
