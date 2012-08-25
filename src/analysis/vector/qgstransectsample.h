#ifndef QGSTRANSECTSAMPLE_H
#define QGSTRANSECTSAMPLE_H

#include "qgsfeature.h"
#include <QMap>
#include <QString>

class QgsGeometry;
class QgsSpatialIndex;
class QgsVectorLayer;
class QProgressDialog;

/**A class for the creation of transect sample lines based on a set of strata polygons and baselines*/
class QgsTransectSample
{
  public:
    QgsTransectSample( QgsVectorLayer* strataLayer, int strataIdAttribute, int minDistanceAttribute,
                       int nPointsAttribute, QgsVectorLayer* baselineLayer, bool shareBaseline,
                       int baselineStrataId, const QString& outputPointLayer, const QString& outputLineLayer );
    ~QgsTransectSample();

    int createSample( QProgressDialog* pd );

  private:
    QgsTransectSample(); //default constructor forbidden

    QgsGeometry* findBaselineGeometry( int strataId );

    /**Returns true if another transect is within the specified minimum distance*/
    static bool otherTransectWithinDistance( QgsGeometry* geom, double minDistance, QgsSpatialIndex& sIndex, const QMap< QgsFeatureId, QgsGeometry* >&
        lineFeatureMap );

    QgsVectorLayer* mStrataLayer;
    int mStrataIdAttribute;
    int mMinDistanceAttribute;
    int mNPointsAttribute;

    QgsVectorLayer* mBaselineLayer;
    bool mShareBaseline;
    int mBaselineStrataId;

    QString mOutputPointLayer;
    QString mOutputLineLayer;
};

#endif // QGSTRANSECTSAMPLE_H
