/***************************************************************************
                          qgsmaptoolsensorinfo.h  -  description
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

#ifndef QGSMAPTOOLSENSORINFO_H
#define QGSMAPTOOLSENSORINFO_H

#include "qgsmaptool.h"

class QgsSensorInfoDialog;
class QgsVectorLayer;
class QDateTime;

class QgsMapToolSensorInfo: public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolSensorInfo( QgsMapCanvas* canvas );
    ~QgsMapToolSensorInfo();

    /**Get sensor information at point (observed properties and available time intervals)*/
    void canvasReleaseEvent( QMouseEvent * e );

  private:
    /**Returns sensor layers in the project*/
    QList< QgsMapLayer* > sensorLayers();
    /**Does GetDataAvailability request for station id*/
    int getDataAvailability( const QString& serviceUrl, const QString& station_id,
                             QStringList& observedPropertyList, QList< QDateTime >& begin,
                             QList< QDateTime >& end );

    bool mDataAvailabilityRequestFinished;

    QgsSensorInfoDialog* mSensorInfoDialog;

  private slots:
    void dataAvailabilityRequestFinished();
    void showSensorInfoDialog();
};

#endif // QGSMAPTOOLSENSORINFO_H
