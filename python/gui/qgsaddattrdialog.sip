class QgsAddAttrDialog: QDialog
{
%TypeHeaderCode
#include "qgsaddattrdialog.h"
%End

public:
    QgsAddAttrDialog( QgsVectorLayer *vlayer,
                      QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );

    QgsField field() const;

    /**Sets a predefined field name programatically*/
    void setFieldName( const QString& name );

    /**Sets a predefined type*/
    void setType( int i );

    void setWidth( int w );

    void setPrecision( int p );

  public slots:
    void on_mTypeBox_currentIndexChanged( int idx );
    void accept();
};