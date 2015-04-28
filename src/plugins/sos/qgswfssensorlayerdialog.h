#ifndef QGSWFSSENSORLAYERDIALOG_H
#define QGSWFSSENSORLAYERDIALOG_H

#include <QDialog>
#include <ui_qgswfssensorlayerdialogbase.h>

class QgsWFSSensorLayerDialog: public QDialog, private Ui::QgsWFSSensorLayerDialogBase
{
    Q_OBJECT
  public:
    QgsWFSSensorLayerDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsWFSSensorLayerDialog();

  private slots:
    void writeSettings();
    void addEntry();
    void removeEntry();
};

#endif // QGSWFSSENSORLAYERDIALOG_H
