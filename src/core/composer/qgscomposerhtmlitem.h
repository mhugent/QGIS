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
