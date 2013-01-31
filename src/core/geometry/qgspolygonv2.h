#ifndef QGSPOLYGONV2_H
#define QGSPOLYGONV2_H

#include "qgsgeometry.h"

/**The name QgsPolygon is already used*/
class QgsPolygonV2: public QgsSurface
{
  public:
    QgsPolygonV2( QVector< QVector< double > >* ringsX, QVector< QVector< double > >* ringsY,
                  QVector< QVector< double > >* ringsZ = 0, QVector< QVector< double > >* ringsM = 0 );
    ~QgsPolygonV2();

    void draw( QPainter* p ) const;
    void coordinateTransform( const QgsCoordinateTransform& t );
    void pixelTransform( const QgsMapToPixel& mtp );
    int translate( double dx, double dy );

    void vertices( QList<QgsPoint>& vertexList ) const;

    //surface
    double area() const;

    //import
    static QgsAbstractGeometry* fromWkb( unsigned char * wkb );
    static QgsAbstractGeometry* fromWkt( const QString& wkt );
    static QgsAbstractGeometry* fromGeos( const GEOSGeometry* geos );

    //export
    unsigned char* asWkb( int& wkbSize ) const;
    GEOSGeometry* asGeos() const;
    QString asWkt() const;
    QDomElement asGML2( QDomDocument& doc ) const;
    QgsGeometry* clone() const;

    bool hasZ() const { return ( mRingsZ && !mRingsZ->isEmpty() ); }
    bool hasM() const { return ( mRingsM && !mRingsM->isEmpty() ); }

    QgsGeometry::GeometryType geometryType() const;
    QGis::WkbType wkbType() const;
    QGis::GeometryType type() const;

    QgsRectangle boundingBox() const;

    bool isMultipart() const { return false; }

    virtual bool convertToMultiType() const;

    //edit
    bool insertVertex( double x, double y, int beforeVertex ); //m and z?
    bool moveVertex( double x, double y, int atVertex ); //keep m and z
    bool deleteVertex( int atVertex );
    bool addRing( const QList<QgsPoint>& ring );
    bool addPart( const QList<QgsPoint> &points );
    bool splitGeometry( const QList<QgsPoint>& splitLine,
                        QList<QgsGeometry*>&newGeometries,
                        bool topological,
                        QList<QgsPoint> &topologyTestPoints );
    bool reshapeGeometry( const QList<QgsPoint>& reshapeWithLine );
    bool makeDifference( QgsGeometry* other );

    //analysis
    QgsPoint vertexAt( int atVertex ) const; //return m and z
    double sqrDistToVertexAt( QgsPoint& point, int atVertex ) const;
    double closestVertexWithContext( const QgsPoint& point, int& atVertex ) const;
    double closestSegmentWithContext( const QgsPoint& point, QgsPoint& minDistPoint, int& afterVertex,
                                      double* leftOf = 0, double epsilon = DEFAULT_SEGMENT_EPSILON ) const;
    void adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex ) const;
    virtual bool deleteRing( int ringNum, int partNum = 0 ) const;
    virtual bool deletePart( int partNum );
    virtual double distance( QgsGeometry& geom ) const;

    virtual int avoidIntersections( QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures = ( QMap<QgsVectorLayer*, QSet<QgsFeatureId> >() ) ) const;

    /**Return number of rings*/
    int ringCount() const;

  protected:
    void addToPainterPath( QPainterPath& path ) const;

  private:
    QVector< QVector< double > >* mRingsX;
    QVector< QVector< double > >* mRingsY;
    QVector< QVector< double > >* mRingsZ;
    QVector< QVector< double > >* mRingsM;
};

#endif // QGSPOLYGONGEOMETRY_H
