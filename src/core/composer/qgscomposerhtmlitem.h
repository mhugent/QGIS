/***************************************************************************
                           qgscomposerhtmlitem.h
                             -------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco.hugentobler@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERHTMLITEM_H
#define QGSCOMPOSERHTMLITEM_H

#include "qgscomposeritem.h"
#include <QWebPage>

class QImage;

class QgsComposerHtmlItem: public QgsComposerItem
{
    Q_OBJECT
  public:
    QgsComposerHtmlItem( QgsComposition* c );
    ~QgsComposerHtmlItem();

    void setUrl( const QUrl& url );
    QUrl url() const;

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    virtual void setSceneRect( const QRectF& rectangle );

    /**Sets size of this item to include all the content of the html page*/
    void setToFullHtmlContent();

  private slots:
    void frameLoaded( bool ok );
    void renderHtmlToImage();

  private:
    QWebPage* mHtml;
    bool mLoaded;
    QImage* mImage; //backbuffering image
    bool mRendering; //true if rendering to backbuffer image is in progress

    void createImage();
};

#endif // QGSCOMPOSERHTMLITEM_H
