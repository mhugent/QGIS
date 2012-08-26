class QgsTransectSample
{
%TypeHeaderCode
#include <qgstransectsample.h>
%End

public:
    QgsTransectSample( QgsVectorLayer* strataLayer, int strataIdAttribute, int minDistanceAttribute,
                       int nPointsAttribute, QgsVectorLayer* baselineLayer, bool shareBaseline,
                       int baselineStrataId, const QString& outputPointLayer, const QString& outputLineLayer );
    ~QgsTransectSample();

    int createSample( QProgressDialog* pd );
};