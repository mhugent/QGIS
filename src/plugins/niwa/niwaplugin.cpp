#include "niwaplugin.h"
#include "qgis.h"
#include <QObject>

static const QString name_ = QObject::tr( "NIWA plugin" );
static const QString description_ = QObject::tr( "A plugin to access and manage layers from OWS services in a unified way" );
static const QString version_ = QObject::tr( "Version 0.1" );
//static const QString icon_ = ":/raster/dem.png";
static const QString category_ = QObject::tr( "Web" );

NiwaPlugin::NiwaPlugin( QgisInterface* iface ): mIface( iface )
{

}

NiwaPlugin::~NiwaPlugin()
{

}

void NiwaPlugin::initGui()
{
    //soon...
}

void NiwaPlugin::unload()
{
    //soon...
}


//global methods for the plugin manager
QGISEXTERN QgisPlugin* classFactory( QgisInterface * ifacePointer )
{
  return new NiwaPlugin( ifacePointer );
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
