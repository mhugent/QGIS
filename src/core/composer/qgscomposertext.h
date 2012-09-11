/***************************************************************************
                              qgscomposertext.h
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

#ifndef QGSCOMPOSERTEXT_H
#define QGSCOMPOSERTEXT_H

#include "qgscomposermultiframe.h"

class QTextDocument;

class QgsComposerText: public QgsComposerMultiFrame
{
  public:
    QgsComposerText( QgsComposition* c, bool createUndoCommands );
    ~QgsComposerText();

    QSizeF totalSize() const;
    void render( QPainter* p, const QRectF& renderExtent );

    void addFrame( QgsComposerFrame* frame, bool recalcFrameSizes = true );

    bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const;

    bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false );

    /**Returns document (caller takes ownership)*/
    QTextDocument* document() { return mTextDocument; }

    /**Sets document (takes ownership)*/
    void setDocument( QTextDocument* doc );

  private:
    QTextDocument* mTextDocument;

};

#endif // QGSCOMPOSERTEXT_H
