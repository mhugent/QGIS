#ifndef WEBDATAMODEL_H
#define WEBDATAMODEL_H

#include <QStandardItemModel>

class QNetworkReply;

/**A model for storing services (WFS/WMS/WCS in future) and their layers*/
class WebDataModel: public QStandardItemModel
{
    Q_OBJECT
  public:
    WebDataModel();
    ~WebDataModel();

    /**Adds service directory and items for service layers to the model
    @param title service title (usually the service name from the combo box)
    @param url service url
    @param serviceName WMS/WFS/WCS*/
    void addService( const QString& title, const QString& url, const QString& service );

  private slots:
    void wmsCapabilitiesRequestFinished();

  private:
    QNetworkReply *mCapabilitiesReply;
};

#endif //WEBDATAMODEL_H
