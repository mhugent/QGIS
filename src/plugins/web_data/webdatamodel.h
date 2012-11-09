#ifndef WEBDATAMODEL_H
#define WEBDATAMODEL_H

#include <QStandardItemModel>

class QgisInterface;
class QNetworkReply;


/**A model for storing services (WFS/WMS/WCS in future) and their layers*/
class WebDataModel: public QStandardItemModel
{
    Q_OBJECT
  public:
    WebDataModel( QgisInterface* iface );
    ~WebDataModel();

    /**Adds service directory and items for service layers to the model
    @param title service title (usually the service name from the combo box)
    @param url service url
    @param serviceName WMS/WFS/WCS*/
    void addService( const QString& title, const QString& url, const QString& service );

    void addEntryToMap( const QModelIndex& index );
    void removeEntryFromMap( const QModelIndex& index );
    void changeEntryToOffline( const QModelIndex& index );
    void changeEntryToOnline( const QModelIndex& index );

  private slots:
    void wmsCapabilitiesRequestFinished();
    void wfsCapabilitiesRequestFinished();
    void handleItemChange( QStandardItem* item );

  private:
    QNetworkReply *mCapabilitiesReply;
    QgisInterface* mIface;

    QString wfsUrlFromLayerIndex( const QModelIndex& index ) const;
    void wmsParameterFromIndex( const QModelIndex& index, QString& url, QString& format, QString& crs, QStringList& layers, QStringList& styles ) const;
};

#endif //WEBDATAMODEL_H
