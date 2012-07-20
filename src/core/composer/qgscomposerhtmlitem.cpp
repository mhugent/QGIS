/***************************************************************************
                           qgscomposerhtmlitem.cpp
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

#include "qgscomposerhtmlitem.h"
#include <QApplication>
#include <QPainter>
#include <QWebFrame>

QgsComposerHtmlItem::QgsComposerHtmlItem( QgsComposition* c ): QgsComposerItem( c ), mLoaded( false ), mImage( 0 )
{
  //test
  mHtml = new QWebPage();
  QObject::connect( mHtml, SIGNAL( loadFinished( bool ) ), this, SLOT( frameLoaded( bool ) ) );
}

QgsComposerHtmlItem::~QgsComposerHtmlItem()
{
  delete mHtml;
  delete mImage;
}

void QgsComposerHtmlItem::setUrl( const QUrl& url )
{
  mHtml->mainFrame()->load( url );
  while ( !mLoaded )
  {
    qApp->processEvents();
  }

  mHtml->setViewportSize( mHtml->mainFrame()->contentsSize() );
}

void QgsComposerHtmlItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  drawBackground( painter );

  painter->save();
  double pixelPerMM = mComposition->printResolution() / 25.4;
  double screenPixelPerMM = 96.0 / 25.4;
  painter->scale( 1.0 / ( pixelPerMM / screenPixelPerMM ), 1.0 / ( pixelPerMM / screenPixelPerMM ) );
  mHtml->mainFrame()->render( painter, QRegion( 0, 0, rect().width() * ( pixelPerMM / screenPixelPerMM ), rect().height() * ( pixelPerMM / screenPixelPerMM ) ) );
  painter->restore();

  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerHtmlItem::createImage()
{
    if( !mComposition )
    {
        return;
    }

    delete mImage;
    double pixelPerMM = mComposition->printResolution() / 25.4;
    int pixelWidth = rect().width() * pixelPerMM;
    int pixelHeight = rect().height() * pixelPerMM;
    mImage = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied );
}

void QgsComposerHtmlItem::renderHtmlToImage()
{

}

void QgsComposerHtmlItem::frameLoaded( bool ok )
{
  mLoaded = true;
}

bool QgsComposerHtmlItem::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  return false; //soon...
}

bool QgsComposerHtmlItem::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  return false; //soon...
}


