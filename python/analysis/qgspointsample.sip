class QgsPointSample
{
%TypeHeaderCode
#include <qgspointsample.h>
%End

   public:
        QgsPointSample( QgsVectorLayer* inputLayer, const QString& outputLayer, int nPointsAttribute, int minDistAttribute = -1 );
        ~QgsPointSample();

        /**Starts calculation of random points
            @return 0 in case of success*/
        int createRandomPoints( QProgressDialog* pd );
};