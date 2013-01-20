#ifndef QGSGEOMETRYUTILS_H
#define QGSGEOMETRYUTILS_H

#include <QVector>
#include <geos_c.h>

class QPolygonF;

class QgsGeometryUtils
{
  public:
    /**Create geos linestring. Caller takes ownership*/
    static GEOSGeometry* createGeosLineString( QVector<double>* x, QVector<double>* y, QVector<double>* z = 0, QVector<double>* m = 0 );
    /**Creates geos coordinate sequence. Caller takes ownership*/
    static GEOSCoordSequence* createGeosCoordSequence( QVector<double>* x, QVector<double>* y, QVector<double>* z = 0, QVector<double>* m = 0 );
    /**Create coordinate vectors from sequence. If z and m are not present, the vectors have size 0*/
    static bool createCoordinateVectors( const GEOSCoordSequence* seq, QVector<double>* x, QVector<double>* y, QVector<double>* z, QVector<double>* m );
    /**Creates QPolygon from x- and y- coordinate vectors*/
    static QPolygonF polygonFromCoordinates( const QVector<double>* x, const QVector<double>* y );
};


#endif // QGSGEOMETRYUTILS_H
