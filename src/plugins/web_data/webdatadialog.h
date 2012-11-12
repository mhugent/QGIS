#ifndef WEBDATADIALOG_H
#define WEBDATADIALOG_H

#include "ui_webdatadialogbase.h"
#include "webdatamodel.h"

class QgisInterface;

class WebDataDialog: public QDialog, private Ui::WebDataDialogBase
{
    Q_OBJECT
  public:
    WebDataDialog( QgisInterface* iface, QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~WebDataDialog();

  private slots:
    void on_mConnectPushButton_clicked();
    void on_mAddPushButton_clicked();
    void on_mRemovePushButton_clicked();
    void on_mEditPushButton_clicked();
    void on_mAddToMapButton_clicked();
    void on_mRemoveFromMapButton_clicked();
    void on_mAddNIWAServicesButton_clicked();
    void on_mAddLINZServicesButton_clicked();
    void NIWAServicesRequestFinished();
    void handleDownloadProgress( qint64 progress, qint64 total );
    void on_mChangeOfflineButton_clicked();
    void on_mChangeOnlineButton_clicked();
    /**Enable / disable layer buttons depending if selected item changes*/
    void adaptLayerButtonStates();

  private:
    QgisInterface* mIface;
    WebDataModel mModel;
    bool mNIWAServicesRequestFinished; //flag to make network request blocking

    QString serviceURLFromComboBox();
    void insertServices();
    void setServiceSetting( const QString& name, const QString& serviceType, const QString& url );
    /**Insert services into combo box
        @param service ("WMS","WFS","WCS")*/
    void insertServices( const QString& service );

    /**Adds services to the combo box from an html page (e.g. https://www.niwa.co.nz/ei/feeds/report)*/
    void addServicesFromHtml( const QString& url );
};

#if 0
class QgisInterface;
class QgsMapLayer;
class QNetworkReply;

class WebDataDialog: public QDialog, private Ui::WebDataDialogBase
{
    Q_OBJECT
  public:
    WebDataDialog( QgisInterface* iface, QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~WebDataDialog();

  private slots:
    void on_mAddPushButton_clicked();
    void on_mRemovePushButton_clicked();
    void on_mEditPushButton_clicked();
    void on_mConnectPushButton_clicked();
    void on_mAddLayerToListButton_clicked();
    void on_mAddToMapButton_clicked();
    void on_mRemoveFromMapButton_clicked();
    void on_mChangeOfflineButton_clicked();
    void on_mChangeOnlineButton_clicked();
    void on_mLayerTreeWidget_currentItemChanged( QTreeWidgetItem* current, QTreeWidgetItem* previous );
    void on_mAddNIWAServicesButton_clicked();
    void on_mAddLINZServicesButton_clicked();
    void on_mRemoveFromListButton_clicked();
    void on_mReloadButton_clicked();
    void NIWAServicesRequestFinished();

    void wfsCapabilitiesRequestFinished();
    void wmsCapabilitiesRequestFinished();

  private:
    QNetworkReply *mCapabilitiesReply;
    QgisInterface* mIface;
    bool mNIWAServicesRequestFinished; //flag to make network request blocking

    void insertServices();
    /**Insert services into combo box
        @param service ("WMS","WFS","WCS")*/
    void insertServices( const QString& service );
    void insertWFSServices();
    QString serviceURLFromComboBox();
    QString wfsUrlFromLayerItem( QTreeWidgetItem* item ) const;
    void wmsParameterFromItem( QTreeWidgetItem* item, QString& url, QString& format, QString& crs, QStringList& layers, QStringList& styles ) const;
    /**Exchanges a layer in the map canvas (and copies the style of the new layer to the old one)*/
    bool exchangeLayer( const QString& layerId, QgsMapLayer* newLayer );
    void setServiceSetting( const QString& name, const QString& serviceType, const QString& url );

    /**Adds services to the combo box from an html page (e.g. https://www.niwa.co.nz/ei/feeds/report)*/
    void addServicesFromHtml( const QString& url );

    void deleteOfflineDatasource( const QString& serviceType, const QString& offlinePath );

    /**Returns id of layer in current map with given url (or empty string if no such layer)*/
    static QString layerIdFromUrl( const QString& url, const QString& serviceType, bool online, QString layerName );

  private slots:
    void loadFromSettings();
    void saveToSettings();
    void handleDownloadProgress( qint64 progress, qint64 total );
};
#endif //0

#endif // WEBDATADIALOG_H
