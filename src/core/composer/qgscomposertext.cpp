/***************************************************************************
                              qgscomposertext.cpp
    ------------------------------------------------------------
    begin                : September 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposertext.h"
#include "qgscomposerframe.h"
#include "qtextdocument.h"
#include <QPainter>

QgsComposerText::QgsComposerText( QgsComposition* c, bool createUndoCommands ): QgsComposerMultiFrame( c, createUndoCommands ),
    mTextDocument( new QTextDocument() )
{
  mTextDocument->setUseDesignMetrics( true );
}

QgsComposerText::~QgsComposerText()
{
  delete mTextDocument;
}

QSizeF QgsComposerText::totalSize() const
{
  return mTextDocument->size();
}

void QgsComposerText::render( QPainter* p, const QRectF& renderExtent )
{
  if ( mFrameItems.size() > 0 )
  {
    mTextDocument->setTextWidth( mFrameItems.at( 0 )->rect().width() );
  }

  p->save();
  double dpMM = mComposition->printResolution() / 96.0;
  p->scale( 1.0 / dpMM, 1.0 / dpMM );
  mTextDocument->drawContents( p, renderExtent );
  p->restore();
}

void QgsComposerText::addFrame( QgsComposerFrame* frame, bool recalcFrameSizes )
{
  //todo: bring to superclass level
  mFrameItems.push_back( frame );
  QObject::connect( frame, SIGNAL( sizeChanged() ), this, SLOT( recalculateFrameSizes() ) );
  if ( mComposition )
  {
    mComposition->addComposerTextFrame( this, frame );
  }

  if ( recalcFrameSizes )
  {
    recalculateFrameSizes();
  }
}

void QgsComposerText::setDocument( QTextDocument* doc )
{
  delete mTextDocument;
  mTextDocument = doc;
}

bool QgsComposerText::writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames ) const
{
  return false; //soon...
}

bool QgsComposerText::readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames )
{
  return false; //soon...
}
