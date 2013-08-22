/***************************************************************************
                          qgssosplugin.h  -  description
                          ------------------------------
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

#ifndef QGSSOSPLUGIN_H
#define QGSSOSPLUGIN_H

#include "qgisplugin.h"
#include <QObject>

class QgisInterface;
class QgsMapToolSensorInfo;
class QAction;

/**A plugin to access Sensor Observation Services (SOS)*/
class QgsSOSPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    QgsSOSPlugin( QgisInterface* iface );
    ~QgsSOSPlugin();

    /**initialize connection to GUI*/
    void initGui();
    /**Unload the plugin and cleanup the GUI*/
    void unload();

  private:
    QgisInterface* mIface;
    QAction* mAction;
    QAction* mSensorInfoAction;
    QgsMapToolSensorInfo* mMapToolSensorInfo;

  private slots:
    void showSOSDialog();
    void toggleSensorInfo( bool checked );
};

#endif // QGSSOSPLUGIN_H
