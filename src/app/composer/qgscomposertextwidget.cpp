/***************************************************************************
                         qgscomposertextwidget.cpp
                         -------------------------
    begin                : September, 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

#include "qgscomposertextwidget.h"
#include "qgscomposertext.h"

QgsComposerTextWidget::QgsComposerTextWidget( QgsComposerText* composerText, QgsComposerFrame* frame ):
    mComposerText( composerText ), mFrame( frame )
{
  setupUi( this );
  mTextEdit->setDocument( composerText->document() );
}

QgsComposerTextWidget::~QgsComposerTextWidget()
{
}


