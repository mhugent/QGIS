#ifndef QGSGEOMETRYUTILS_H
#define QGSGEOMETRYUTILS_H

#include "qgspoint.h"
#include <QVector>
#include <geos_c.h>

class QPolygonF;

class QgsGeometryUtils
{
  public:
    /**Create geos linestring. Caller takes ownership*/
    static GEOSGeometry* createGeosLineString( const QVector<double>* x, const QVector<double>* y, const QVector<double>* z = 0,
        const QVector<double>* m = 0 );
    /**Create geos linear ring. Caller takes ownership*/
    static GEOSGeometry* createGeosLinearRing( const QVector<double>* x, const QVector<double>* y, const QVector<double>* z = 0,
        const QVector<double>* m = 0 );
    /**Create geos polygon. Caller takes ownership*/
    static GEOSGeometry* createGeosPolygon( const QVector< QVector< double > >* ringsX, const QVector< QVector< double > >* ringsY,
                                            const QVector< QVector< double > >* ringsZ = 0, const QVector< QVector< double > >* ringsM = 0 );
    /**Creates geos coordinate sequence. Caller takes ownership
        @param close add first point at the end if true (necessary e.g. for linear ring)*/
    static GEOSCoordSequence* createGeosCoordSequence( const QVector<double>* x, const QVector<double>* y, const QVector<double>* z = 0,
        const QVector<double>* m = 0, bool close = false );
    /**Create coordinate vectors from sequence. If z and m are not present, the vectors have size 0*/
    static bool createCoordinateVectors( const GEOSCoordSequence* seq, QVector<double>* x, QVector<double>* y, QVector<double>* z, QVector<double>* m );
    /**Creates QPolygon from x- and y- coordinate vectors*/
    static QPolygonF polygonFromCoordinates( const QVector<double>* x, const QVector<double>* y );
    /**Creates QgsPoint vector from polygon*/
    static QVector<QgsPoint> pointVectorFromPolygon( const QPolygonF& polygon );
    static QVector< QVector<QgsPoint> > convertToRings( const QPolygonF& points, QList<QPolygonF>* rings );
};


#endif // QGSGEOMETRYUTILS_H
