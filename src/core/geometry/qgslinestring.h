#ifndef QGSLINESTRING_H
#define QGSLINESTRING_H

#include "qgsgeometry.h"

class QgsLineString: public QgsCurve
{
  public:
    QgsLineString( QVector<double>* x = 0, QVector<double>* y = 0, QVector<double>* z = 0, QVector<double>* m = 0 ); //constructor takes ownership of vertices
    ~QgsLineString();

    void draw( QPainter* p ) const;
    void drawVertexMarkers( QPainter* p, QgsGeometry::VertexMarkerType type, int size ) const;

    //changes the geometry in place
    void coordinateTransform( const QgsCoordinateTransform& t );
    void pixelTransform( const QgsMapToPixel& mtp );
    int translate( double dx, double dy );

    //curve methods
    double length() const;
    bool isClosed() const;

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

    bool hasZ() const { return ( mZValues && !mZValues->isEmpty() ); }
    bool hasM() const { return ( mMValues && !mMValues->isEmpty() ); }

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

  protected:
    void addToPainterPath( QPainterPath& path ) const;

  private:
    QVector<double>* mXValues;
    QVector<double>* mYValues;
    QVector<double>* mZValues;
    QVector<double>* mMValues;
};

#endif // QGSLINESTRING_H
