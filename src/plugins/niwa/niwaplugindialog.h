#ifndef NIWAPLUGINDIALOG_H
#define NIWAPLUGINDIALOG_H

#include "ui_niwaplugindialogbase.h"
class QgisInterface;
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
    void on_mChangeOfflineButton_clicked();

    void wfsCapabilitiesRequestFinished();
    void wmsCapabilitiesRequestFinished();

  private:
    QNetworkReply *mCapabilitiesReply;
    QgisInterface* mIface;

    void insertServices();
    /**Insert services into combo box
        @param service ("WMS","WFS","WCS")*/
    void insertServices( const QString& service );
    void insertWFSServices();
    QString serviceURLFromComboBox();
    QString wfsUrlFromLayerItem( QTreeWidgetItem* item );
};

#endif // NIWAPLUGINDIALOG_H
