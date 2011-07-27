
#ifndef QGSMARKERSYMBOLLAYERV2_H
#define QGSMARKERSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

#define DEFAULT_SIMPLEMARKER_NAME         "circle"
#define DEFAULT_SIMPLEMARKER_COLOR        QColor(255,0,0)
#define DEFAULT_SIMPLEMARKER_BORDERCOLOR  QColor(0,0,0)
#define DEFAULT_SIMPLEMARKER_SIZE         DEFAULT_POINT_SIZE
#define DEFAULT_SIMPLEMARKER_ANGLE        0

#include <QPen>
#include <QBrush>
#include <QPicture>
#include <QPolygonF>
#include <QFont>

class CORE_EXPORT QgsSimpleMarkerSymbolLayerV2 : public QgsMarkerSymbolLayerV2
{
  public:
    QgsSimpleMarkerSymbolLayerV2( QString name = DEFAULT_SIMPLEMARKER_NAME,
                                  QColor color = DEFAULT_SIMPLEMARKER_COLOR,
                                  QColor borderColor = DEFAULT_SIMPLEMARKER_BORDERCOLOR,
                                  double size = DEFAULT_SIMPLEMARKER_SIZE,
                                  double angle = DEFAULT_SIMPLEMARKER_ANGLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context, const QgsFieldMap* fields = 0 );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    QString name() const { return mName; }
    void setName( QString name ) { mName = name; }

    QColor borderColor() const { return mBorderColor; }
    void setBorderColor( QColor color ) { mBorderColor = color; }

    double borderWidth() const { return mBorderWidth; }
    void setBorderWidth( double w ){ mBorderWidth = w; }

    double symbolWidth() const { return mSymbolWidth; }
    void setSymbolWidth( double w ){ mSymbolWidth = w; }
    double symbolHeight() const { return mSymbolHeight; }
    void setSymbolHeight( double h ){ mSymbolHeight = h; }

    QString widthField() const { return mWidthField; }
    void setWidthField( const QString& f ){ mWidthField = f; }
    QString heightField() const { return mHeightField; }
    void setHeightField( const QString& f ){ mHeightField = f; }
    QString rotationField() const { return mRotationField; }
    void setRotationField( const QString& f ){ mRotationField = f; }
    QString outlineWidthField() const { return mOutlineWidthField; }
    void setOutlineWidthField( const QString& f ){ mOutlineWidthField = f; }
    QString fillColorField() const { return mFillColorField; }
    void setFillColorField( const QString& f ){ mFillColorField = f; }
    QString outlineColorField() const { return mOutlineColorField; }
    void setOutlineColorField( const QString& f ){ mOutlineColorField = f; }
    QString symbolNameField() const { return mSymbolNameField; }
    void setSymbolNameField( const QString& f ){ mSymbolNameField = f; }


  protected:

    void drawMarker( QPainter* p, QgsSymbolV2RenderContext& context );

    bool prepareShape();
    bool preparePath();

    void prepareCache( QgsSymbolV2RenderContext& context );
    void resolveFieldIndices( const QgsFieldMap* fields );
    int fieldNameIndex( const QString& fieldName, const QgsFieldMap* fields ) const;

    QColor mBorderColor;
    double mBorderWidth;
    QPen mPen;
    QBrush mBrush;
    QPolygonF mPolygon;
    QPainterPath mPath;
    QString mName;
    QImage mCache;
    QPen mSelPen;
    QBrush mSelBrush;
    QImage mSelCache;
    bool mUsingCache;

    //simple marker symbols can be scaled differently in x- and y
    double mSymbolWidth;
    double mSymbolHeight;

    //data defined attributes (null strings means fixed value)
    QString mWidthField;
    QString mHeightField;
    QString mRotationField;
    QString mOutlineWidthField;
    QString mFillColorField;
    QString mOutlineColorField;
    QString mSymbolNameField;

    //data defined field indices (resolved in startRender method, -1 if not data defined)
    int mWidthFieldIndex;
    int mHeightFieldIndex;
    int mRotationFieldIndex;
    int mOutlineWidthFieldIndex;
    int mFillColorFieldIndex;
    int mOutlineColorFieldIndex;
    int mSymbolNameFieldIndex;
};

//////////

#define DEFAULT_SVGMARKER_NAME         "/symbol/Star1.svg"
#define DEFAULT_SVGMARKER_SIZE         2*DEFAULT_POINT_SIZE
#define DEFAULT_SVGMARKER_ANGLE        0

class CORE_EXPORT QgsSvgMarkerSymbolLayerV2 : public QgsMarkerSymbolLayerV2
{
  public:
    QgsSvgMarkerSymbolLayerV2( QString name = DEFAULT_SVGMARKER_NAME,
                               double size = DEFAULT_SVGMARKER_SIZE,
                               double angle = DEFAULT_SVGMARKER_ANGLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    //! Return a list of all available svg files
    static QStringList listSvgFiles();

    //! Get symbol's path from its name
    static QString symbolNameToPath( QString name );

    //! Get symbols's name from its path
    static QString symbolPathToName( QString path );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context, const QgsFieldMap* fields = 0 );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    QString path() const { return mPath; }
    void setPath( QString path );

    QColor fillColor() const { return mFillColor; }
    void setFillColor( const QColor& c ) { mFillColor = c; }

    QColor outlineColor() const { return mOutlineColor; }
    void setOutlineColor( const QColor& c ) { mOutlineColor = c; }

    double outlineWidth() const { return mOutlineWidth; }
    void setOutlineWidth( double w ) { mOutlineWidth = w; }

  protected:

    void loadSvg();

    QString mPath;

    //param(fill), param(outline), param(outline-width) are going
    //to be replaced in memory
    QColor mFillColor;
    QColor mOutlineColor;
    double mOutlineWidth;
    double mOrigSize;
};


//////////

#define POINT2MM(x) ( (x) * 25.4 / 72 ) // point is 1/72 of inch
#define MM2POINT(x) ( (x) * 72 / 25.4 )

#define DEFAULT_FONTMARKER_FONT   "Dingbats"
#define DEFAULT_FONTMARKER_CHR    QChar('A')
#define DEFAULT_FONTMARKER_SIZE   POINT2MM(12)
#define DEFAULT_FONTMARKER_COLOR  QColor(Qt::black)
#define DEFAULT_FONTMARKER_ANGLE  0

class CORE_EXPORT QgsFontMarkerSymbolLayerV2 : public QgsMarkerSymbolLayerV2
{
  public:
    QgsFontMarkerSymbolLayerV2( QString fontFamily = DEFAULT_FONTMARKER_FONT,
                                QChar chr = DEFAULT_FONTMARKER_CHR,
                                double pointSize = DEFAULT_FONTMARKER_SIZE,
                                QColor color = DEFAULT_FONTMARKER_COLOR,
                                double angle = DEFAULT_FONTMARKER_ANGLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context, const QgsFieldMap* fields = 0 );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    // new methods

    QString fontFamily() const { return mFontFamily; }
    void setFontFamily( QString family ) { mFontFamily = family; }

    QChar character() const { return mChr; }
    void setCharacter( QChar ch ) { mChr = ch; }

  protected:

    QString mFontFamily;
    QChar mChr;

    QPointF mChrOffset;
    QFont mFont;
    double mOrigSize;
};


#endif
