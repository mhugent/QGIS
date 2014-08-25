/***************************************************************************
                          qgssosplugin.cpp  -  description
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

#include "qgssosplugin.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolsensorinfo.h"
#include "qgssossourceselect.h"
#include "qgis.h"
#include "qgisinterface.h"
#include <QAction>

static const QString name_ = QObject::tr( "SOS plugin" );
static const QString description_ = QObject::tr( "A plugin to access sensor observation services" );
static const QString version_ = QObject::tr( "Version 0.1" );
//static const QString icon_ = ":/raster/dem.png";
static const QString category_ = QObject::tr( "Vector" );

QgsSOSPlugin::QgsSOSPlugin( QgisInterface* iface ): mIface( iface ), mAction( 0 ), mSensorInfoAction( 0 ), mMapToolSensorInfo( 0 )
{

}

QgsSOSPlugin::~QgsSOSPlugin()
{
  unload();
}

void QgsSOSPlugin::initGui()
{
  if ( mIface )
  {
    mAction = new QAction( QIcon( ":/sos.png" ), tr( "Sensor observation service" ), 0 );
    QObject::connect( mAction, SIGNAL( triggered() ), this, SLOT( showSOSDialog() ) );
    mIface->addWebToolBarIcon( mAction );
    mIface->addPluginToWebMenu( tr( "Sensor observation service" ), mAction );

    mSensorInfoAction = new QAction( QIcon( ":/sensor_info.png" ), tr( "Sensor info" ), 0 );
    QObject::connect( mSensorInfoAction, SIGNAL( toggled( bool ) ), this, SLOT( toggleSensorInfo( bool ) ) );
    mSensorInfoAction->setCheckable( true );
    mIface->addWebToolBarIcon( mSensorInfoAction );

    mMapToolSensorInfo = new QgsMapToolSensorInfo( mIface->mapCanvas() );
    mMapToolSensorInfo->setAction( mSensorInfoAction );
  }
}

void QgsSOSPlugin::unload()
{
  if ( mAction && mIface )
  {
    mIface->removeWebToolBarIcon( mAction );
    mIface->removePluginWebMenu( tr( "Sensor ovservation service" ), mAction );
  }
  delete mAction;
  mAction = 0;

  if ( mSensorInfoAction && mIface )
  {
    mIface->removeWebToolBarIcon( mSensorInfoAction );
  }

  delete mMapToolSensorInfo;
  mMapToolSensorInfo = 0;

  delete mSensorInfoAction;
  mSensorInfoAction = 0;
}

void QgsSOSPlugin::showSOSDialog()
{
  QgsSOSSourceSelect dialog( mIface );
  dialog.exec();
}

void QgsSOSPlugin::toggleSensorInfo( bool checked )
{
  if ( !mIface )
  {
    return;
  }

  QgsMapCanvas* canvas = mIface->mapCanvas();
  if ( !canvas )
  {
    return;
  }

  if ( checked )
  {
    canvas->setMapTool( mMapToolSensorInfo );
  }
  else
  {
    canvas->unsetMapTool( mMapToolSensorInfo );
  }
}

//global methods for the plugin manager
QGISEXTERN QgisPlugin* classFactory( QgisInterface * ifacePointer )
{
  return new QgsSOSPlugin( ifacePointer );
}

QGISEXTERN QString name()
{
  return name_;
}

QGISEXTERN QString description()
{
  return description_;
}

QGISEXTERN QString version()
{
  return version_;
}

/*QGISEXTERN QString icon()
{
  return icon_;
}*/

QGISEXTERN int type()
{
  return QgisPlugin::UI;
}

QGISEXTERN void unload( QgisPlugin* pluginPointer )
{
  delete pluginPointer;
}

QGISEXTERN QString category()
{
  return category_;
}

