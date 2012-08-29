class QgsAttributeDialog: QObject
{
%TypeHeaderCode
#include "qgsattributedialog.h"
%End

  public:
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature, bool featureOwner, QWidget* parent = 0 );
    ~QgsAttributeDialog();

    /** Saves the size and position for the next time
     *  this dialog box was used.
     */

    void saveGeometry();

    /** Restores the size and position from the last time
     *  this dialog box was used.
     */
    void restoreGeometry();

    void setHighlight( QgsHighlight *h );

    QDialog *dialog();

  public slots:
    void accept();

    int exec_();
    void show();

    void dialogDestroyed();
};
