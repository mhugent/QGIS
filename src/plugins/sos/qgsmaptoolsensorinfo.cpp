/***************************************************************************
                          qgsmaptoolsensorinfo.cpp  -  description
                          --------------------------------------
    begin                : June 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolsensorinfo.h"

QgsMapToolSensorInfo::QgsMapToolSensorInfo( QgsMapCanvas* canvas ): QgsMapTool( canvas )
{

}

QgsMapToolSensorInfo::~QgsMapToolSensorInfo()
{

}

void QgsMapToolSensorInfo::canvasReleaseEvent( QMouseEvent * e )
{
  qWarning( "***********************QgsMapToolSensorInfo canvas click******************" );

  //get sensor layer (active one or the first sos layer otherwise)

  //get features under the cursor

  //show window with GetDataAvailability info for all features
}
