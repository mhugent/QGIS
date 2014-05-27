#ifndef WEBDATADIALOG_H
#define WEBDATADIALOG_H

#include "ui_webdatadialogbase.h"
#include "webdatafiltermodel.h"
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
    void on_mAddNIWAServicesButton_clicked();
    void on_mAddLINZServicesButton_clicked();
    void on_mAddLRISButton_clicked();
    void NIWAServicesRequestFinished();
    void handleDownloadProgress( qint64 progress, qint64 total );
    void on_mOnlyFavouritesCheckBox_stateChanged( int state );
    void on_mSearchTableEdit_textChanged( const QString&  text );
    void on_mLayersTreeView_clicked( const QModelIndex& index );
    void keyPressEvent( QKeyEvent* event );
    void resetStateAndCursor(); //set status text to ready and restore cursor
    void deleteEntry();
    void updateEntry();
    void showContextMenu( const QPoint& point );

  private:
    QgisInterface* mIface;
    WebDataModel mModel;
    WebDataFilterModel mFilterModel;
    bool mNIWAServicesRequestFinished; //flag to make network request blocking
    QMenu* mContextMenu;

    QString serviceURLFromComboBox();
    void insertServices();
    void setServiceSetting( const QString& name, const QString& serviceType, const QString& url );
    /**Insert services into combo box
        @param service ("WMS","WFS","WCS")*/
    void insertServices( const QString& service );

    /**Adds services to the combo box from an html page (e.g. https://www.niwa.co.nz/ei/feeds/report)*/
    void addServicesFromHtml( const QString& url );

    /**Adds services from a CSW (e.g. http://dc.niwa.co.nz/niwa_dc_cogs/srv/eng/csw) */
    void addServicesFromCSW( const QString& url );

    /**Returns true if main canvas is drawing*/
    bool mapCanvasDrawing() const;

    QModelIndex selectedModelIndex() const;
};

#endif // WEBDATADIALOG_H
