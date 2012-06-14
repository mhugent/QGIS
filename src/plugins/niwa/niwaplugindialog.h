#ifndef NIWAPLUGINDIALOG_H
#define NIWAPLUGINDIALOG_H

#include "ui_niwaplugindialogbase.h"
class QgisInterface;
class QgsMapLayer;
class QNetworkReply;

class NiwaPluginDialog: public QDialog, private Ui::NiwaPluginDialogBase
{
    Q_OBJECT
  public:
    NiwaPluginDialog( QgisInterface* iface, QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~NiwaPluginDialog();

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

  private slots:
    void loadFromSettings();
    void saveToSettings();
    void handleDownloadProgress( qint64 progress, qint64 total );
};

#endif // NIWAPLUGINDIALOG_H
