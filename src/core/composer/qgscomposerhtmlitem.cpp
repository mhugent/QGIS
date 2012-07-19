#include "qgscomposerhtmlitem.h"
#include <QApplication>
#include <QWebFrame>

QgsComposerHtmlItem::QgsComposerHtmlItem( QgsComposition* c ): QgsComposerItem( c ), mLoaded( false )
{
  //test
  mHtml = new QWebPage();
  QObject::connect( mHtml, SIGNAL( loadFinished( bool ) ), this, SLOT( frameLoaded( bool ) ) );
  //setUrl( QUrl( "http://www.server.com" ) );
}

QgsComposerHtmlItem::~QgsComposerHtmlItem()
{
  delete mHtml;
}

void QgsComposerHtmlItem::setUrl( const QUrl& url )
{
  mHtml->mainFrame()->load( url );
  while ( !mLoaded )
  {
    qApp->processEvents();
  }
  mHtml->setViewportSize( mHtml->mainFrame()->contentsSize() );
  //mHtml->setViewportSize( QSize( rect().width(), rect().height() ) );
}

void QgsComposerHtmlItem::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  drawBackground( painter );
  mHtml->mainFrame()->render( painter );
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
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


