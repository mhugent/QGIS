#include "niwaplugin.h"
#include "niwaplugindialog.h"
#include "qgis.h"
#include "qgisinterface.h"
#include <QAction>
#include <QObject>

static const QString name_ = QObject::tr( "NIWA plugin" );
static const QString description_ = QObject::tr( "A plugin to access and manage layers from OWS services in a unified way" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QString icon_ = ":/niwa/icons/niwa.jpg";
static const QString category_ = QObject::tr( "Web" );

NiwaPlugin::NiwaPlugin( QgisInterface* iface ): mIface( iface ), mAction( 0 )
{

}

NiwaPlugin::~NiwaPlugin()
{
  delete mAction;
}

void NiwaPlugin::initGui()
{
  if ( mIface )
  {
    mAction = new QAction( QIcon( icon_ ), tr( "NIWA plugin" ), 0 );
    connect( mAction, SIGNAL( triggered() ), this, SLOT( showNiwaDialog() ) );
    mIface->addWebToolBarIcon( mAction );
    mIface->addPluginToMenu( name_, mAction );
  }
}

void NiwaPlugin::unload()
{
  mIface->removePluginMenu( name_, mAction );
  mIface->removeWebToolBarIcon( mAction );
  delete mAction;
  mAction = 0;
}

void NiwaPlugin::showNiwaDialog()
{
  NiwaPluginDialog d;
  d.exec();
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

QGISEXTERN QString icon()
{
  return icon_;
}

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
