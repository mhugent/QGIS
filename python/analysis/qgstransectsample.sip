class QgsTransectSample
{
%TypeHeaderCode
#include <qgstransectsample.h>
%End

public:

    enum DistanceUnits
    {
        Meters,
        StrataUnits //units are the same as stratum layer
    };

    QgsTransectSample( QgsVectorLayer* strataLayer, int strataIdAttribute, int minDistanceAttribute,
                       DistanceUnits minDistUnits, int nPointsAttribute, QgsVectorLayer* baselineLayer, bool shareBaseline,
                       int baselineStrataId, const QString& outputPointLayer, const QString& outputLineLayer, const QString& usedBaselineLayer, double minTransectLength = 0.0);
    ~QgsTransectSample();

    int createSample( QProgressDialog* pd );
};
