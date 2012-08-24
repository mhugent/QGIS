#ifndef QGSTRANSECTSAMPLE_H
#define QGSTRANSECTSAMPLE_H

#include <QString>

class QgsGeometry;
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
