class QgsNewVectorLayerDialog: QDialog
{
%TypeHeaderCode
#include <qgsnewvectorlayerdialog.h>
%End

public:

   struct AttributeEntry
    {
        QString name;
        QString type;
        int width;
        int precision;
    };

    // run the dialog, create the layer. Return file name if the creation was successful
    static QString runAndCreateLayer( QWidget* parent = 0, QString* enc = 0, QGis::GeometryType geom = QGis::Point,
const QList< QgsNewVectorLayerDialog::AttributeEntry >& att = QList<QgsNewVectorLayerDialog::AttributeEntry>() );


    QgsNewVectorLayerDialog( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsNewVectorLayerDialog();
    /**Returns the selected geometry type*/
    QGis::WkbType selectedType() const;

    /**Populates the attribute section programatically*/
    void insertAttributeEntry( const QString& name, const QString& type, int width, int precision );

    /**Returns the file format for storage*/
    QString selectedFileFormat() const;
    /**Returns the selected crs id*/
    int selectedCrsId() const;
};