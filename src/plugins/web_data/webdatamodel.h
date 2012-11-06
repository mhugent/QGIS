#ifndef WEBDATAMODEL_H
#define WEBDATAMODEL_H

#include <QStandardItemModel>

/**A model for storing services (WFS/WMS/WCS in future) and their layers*/
class WebDataModel: public QStandardItemModel
{
  public:
    WebDataModel();
    ~WebDataModel();

    /**Adds service directory and items for service layers to the model
    @param url service url
    @param serviceName WMS/WFS/WCS*/
    void addService( const QString& url, const QString& serviceName );
};

#endif //WEBDATAMODEL_H
