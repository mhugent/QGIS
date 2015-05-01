#ifndef QGSADDWFSSENSORLAYERDIALOG_H
#define QGSADDWFSSENSORLAYERDIALOG_H

#include "ui_qgsaddwfssensorlayerdialogbase.h"
#include <QDialog>

class QgsMapLayer;
class QgsVectorLayer;

class QgsAddWFSSensorLayerDialog: public QDialog, private Ui::QgsAddWFSSensorLayerDialogBase
{
    Q_OBJECT
  public:
    QgsAddWFSSensorLayerDialog( const QMap<QString, QgsMapLayer*>& layerMap, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsAddWFSSensorLayerDialog();

    QString wfsUrl() const;
    QString sosUrl() const;
    QString idAttribute() const;
    QString observableAttribute() const;
    QString fromAttribute() const;
    QString toAttribute() const;

  private:
    void addAttributesToComboBox( QComboBox*, const QgsVectorLayer* vl );

  private slots:
    void on_mLayerComboBox_currentIndexChanged( const QString& text );
};

#endif // QGSADDWFSSENSORLAYERDIALOG_H
