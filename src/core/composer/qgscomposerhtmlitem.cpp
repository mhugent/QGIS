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

QgsComposerHtmlItem::QgsComposerHtmlItem( QgsComposition* c ): QgsComposerItem( c ), mLoaded( false ), mImage( 0 ), mRendering( false )
{
  //test
  mHtml = new QWebPage();
  QObject::connect( mHtml, SIGNAL( loadFinished( bool ) ), this, SLOT( frameLoaded( bool ) ) );
  QObject::connect( mHtml, SIGNAL( repaintRequested( const QRect& ) ), this, SLOT( renderHtmlToImage() ) );
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
  renderHtmlToImage();
}

void QgsComposerHtmlItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  if ( mRendering )
  {
    return;
  }

  drawBackground( painter );
  painter->drawImage( rect(), *mImage, QRectF( 0, 0, mImage->width(), mImage->height() ) );
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerHtmlItem::createImage()
{
  if ( !mComposition )
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
  if ( !mImage )
  {
    return;
  }

  if ( mRendering )
  {
    return;
  }

  mRendering = true;
  QPainter p( mImage );
  double pixelPerMM = mComposition->printResolution() / 25.4;
  double sFactor = scaleFactor();
  p.scale( sFactor, sFactor );
  mHtml->mainFrame()->render( &p, QRegion( 0, 0, rect().width() * pixelPerMM, rect().height() * pixelPerMM ) );
  mRendering = false;
  update();
}

void QgsComposerHtmlItem::setSceneRect( const QRectF& rectangle )
{
  QgsComposerItem::setSceneRect( rectangle );
  createImage();
  renderHtmlToImage();
  update();
  emit itemChanged();
}

void QgsComposerHtmlItem::setToFullHtmlContent()
{
  if ( mHtml && mHtml->mainFrame() )
  {
    QSize size = mHtml->mainFrame()->contentsSize();
    double sFactor = scaleFactor();
    double width = size.width() / sFactor;
    double height = size.height() / sFactor;
    setSceneRect( QRectF( transform().dx(), transform().dy(), width, height ) );
  }
}

double QgsComposerHtmlItem::scaleFactor()
{
  if ( !mComposition || !mImage )
  {
    return 1.0;
  }

  double pixelPerMM = mComposition->printResolution() / 25.4;
  return pixelPerMM / (( double )mImage->dotsPerMeterX() / 1000.0 );
}

QUrl QgsComposerHtmlItem::url() const
{
  if ( !mHtml )
  {
    return QUrl();
  }

  return mHtml->mainFrame()->requestedUrl();
}

void QgsComposerHtmlItem::frameLoaded( bool ok )
{
  mLoaded = true;
}

bool QgsComposerHtmlItem::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerHtmlElem = doc.createElement( "ComposerHtml" );
  composerHtmlElem.setAttribute( "url" , url().toString() );
  elem.appendChild( composerHtmlElem );
  return _writeXML( composerHtmlElem, doc );
}

bool QgsComposerHtmlItem::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  QString urlString = itemElem.attribute( "url", "" );
  if ( !urlString.isEmpty() )
  {
    setUrl( QUrl( urlString ) );
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }
  return true;
}


