class QgsHighlight: QgsMapCanvasItem
{
%TypeHeaderCode
#include "qgshighlight.h"
%End

  public:
    QgsHighlight( QgsMapCanvas *mapCanvas, QgsGeometry *geom, QgsVectorLayer *layer );
    ~QgsHighlight();

    void setColor( const QColor & color );
    void setWidth( int width );
};
