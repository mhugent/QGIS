#ifndef QGSTRANSECTSAMPLE_H
#define QGSTRANSECTSAMPLE_H

#include "qgsfeature.h"
#include <QMap>
#include <QString>

class QgsDistanceArea;
class QgsGeometry;
class QgsSpatialIndex;
class QgsVectorLayer;
class QgsPoint;
class QProgressDialog;

/**A class for the creation of transect sample lines based on a set of strata polygons and baselines*/
class ANALYSIS_EXPORT QgsTransectSample
{
  public:

    enum DistanceUnits
    {
      Meters,
      StrataUnits //units are the same as stratum layer
    };

    QgsTransectSample( QgsVectorLayer* strataLayer, int strataIdAttribute, int minDistanceAttribute, DistanceUnits minDistUnits,
                       int nPointsAttribute, QgsVectorLayer* baselineLayer, bool shareBaseline,
                       int baselineStrataId, const QString& outputPointLayer, const QString& outputLineLayer, const QString& usedBaselineLayer );
    ~QgsTransectSample();

    int createSample( QProgressDialog* pd );

  private:
    QgsTransectSample(); //default constructor forbidden

    QgsGeometry* findBaselineGeometry( int strataId );

    /**Returns true if another transect is within the specified minimum distance*/
    static bool otherTransectWithinDistance( QgsGeometry* geom, double minDistance, QgsSpatialIndex& sIndex, const QMap< QgsFeatureId, QgsGeometry* >&
        lineFeatureMap, QgsDistanceArea& da );

    QgsVectorLayer* mStrataLayer;
    int mStrataIdAttribute;
    int mMinDistanceAttribute;
    int mNPointsAttribute;

    QgsVectorLayer* mBaselineLayer;
    bool mShareBaseline;
    int mBaselineStrataId;

    QString mOutputPointLayer;
    QString mOutputLineLayer;
    QString mUsedBaselineLayer;

    DistanceUnits mMinDistanceUnits;

    /**Finds the closest points between two line segments
        @param g1 first input geometry. Must be a linestring with two vertices
        @param g2 second input geometry. Must be a linestring with two vertices
        @param dist out: distance between the segments
        @param pt1 out: closest point on first geometry
        @param pt2 out: closest point on secont geometry
        @return true in case of success*/
    static bool closestSegmentPoints( QgsGeometry& g1, QgsGeometry& g2, double& dist, QgsPoint& pt1, QgsPoint& pt2 );
    /**Returns a copy of the multiline element closest to a point (caller takes ownership)*/
    static QgsGeometry* closestMultilineElement( const QgsPoint& pt, QgsGeometry* multiLine );
};

#endif // QGSTRANSECTSAMPLE_H
