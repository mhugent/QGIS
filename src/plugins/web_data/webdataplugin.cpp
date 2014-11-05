#include "webdataplugin.h"
#include "webdatadialog.h"
#include "qgis.h"
#include "qgisinterface.h"
#include <QAction>
#include <QObject>

static const QString name_ = QObject::tr( "Web data plugin" );
static const QString description_ = QObject::tr( "A plugin to access and manage layers from OWS services in a unified way" );
static const QString version_ = QObject::tr( "Version 1.0" );
static const QString icon_ = ":/niwa/icons/nqmap.png";
static const QString category_ = QObject::tr( "Web" );

WebDataPlugin::WebDataPlugin( QgisInterface* iface ): mIface( iface ), mAction( 0 ), mDialog( 0 )
{

}

WebDataPlugin::~WebDataPlugin()
{
  delete mAction;
  delete mDialog;
}

void WebDataPlugin::initGui()
{
  if ( mIface )
  {
    mAction = new QAction( QIcon( icon_ ), tr( "Web data plugin" ), 0 );
    connect( mAction, SIGNAL( triggered() ), this, SLOT( showWebDataDialog() ) );
    mIface->addWebToolBarIcon( mAction );
    mIface->addPluginToMenu( name_, mAction );
  }
}

void WebDataPlugin::unload()
{
  mIface->removePluginMenu( name_, mAction );
  mIface->removeWebToolBarIcon( mAction );
  delete mAction;
  mAction = 0;
}

void WebDataPlugin::showWebDataDialog()
{
  if ( !mDialog && mIface )
  {
    mDialog = new WebDataDialog( mIface, mIface->mainWindow() );
  }
  mDialog->show();
}


//global methods for the plugin manager
QGISEXTERN QgisPlugin* classFactory( QgisInterface * ifacePointer )
{
  return new WebDataPlugin( ifacePointer );
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
