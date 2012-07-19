#ifndef QGSCOMPOSERHTMLITEM_H
#define QGSCOMPOSERHTMLITEM_H

#include "qgscomposeritem.h"
#include <QWebPage>

class QgsComposerHtmlItem: public QgsComposerItem
{
    Q_OBJECT
  public:
    QgsComposerHtmlItem( QgsComposition* c );
    ~QgsComposerHtmlItem();

    void setUrl( const QUrl& url );

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

  private slots:
    void frameLoaded( bool ok );

  private:
    QWebPage* mHtml;
    bool mLoaded;
};

#endif // QGSCOMPOSERHTMLITEM_H
