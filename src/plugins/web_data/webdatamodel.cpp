#include "webdatamodel.h"

WebDataModel::WebDataModel(): QStandardItemModel()
{
  QStringList headerLabels;
  headerLabels << tr( "Name" );
  headerLabels << tr( "Favorite" );
  headerLabels << tr( "Type" );
  headerLabels << tr( "In map" );
  headerLabels << tr( "Status" );
  headerLabels << tr( "CRS" );
  headerLabels << tr( "FORMATS" );
  headerLabels << tr( "Styles" );
  setHorizontalHeaderLabels( headerLabels );
}

WebDataModel::~WebDataModel()
{
}

void WebDataModel::addService( const QString& url, const QString& serviceName )
{
}
