/***************************************************************************
                         qgscomposertextwidget.cpp
                         -------------------------
    begin                : September, 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

#include "qgscomposertextwidget.h"
#include "qgscomposertext.h"
#include <QColorDialog>

QgsComposerTextWidget::QgsComposerTextWidget( QgsComposerText* composerText, QgsComposerFrame* frame ):
    mComposerText( composerText ), mFrame( frame )
{
  setupUi( this );
  mTextEdit->setDocument( composerText->document() );

  connect( mBoldPushButton, SIGNAL( toggled( bool ) ), this, SLOT( changeCurrentFormat() ) );
  connect( mItalicsPushButton, SIGNAL( toggled( bool ) ), this, SLOT( changeCurrentFormat() ) );
  connect( mFontSizeSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( changeCurrentFormat() ) );
  connect( mFontComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( changeCurrentFormat() ) );
}

QgsComposerTextWidget::~QgsComposerTextWidget()
{
}

void QgsComposerTextWidget::changeCurrentFormat()
{
  QFont newFont;
  newFont.setFamily( mFontComboBox->currentFont().family() );

  //bold
  if ( mBoldPushButton->isChecked() )
  {
    newFont.setBold( true );
  }
  else
  {
    newFont.setBold( false );
  }

  //italic
  if ( mItalicsPushButton->isChecked() )
  {
    newFont.setItalic( true );
  }
  else
  {
    newFont.setItalic( false );
  }

  //size
  newFont.setPointSize( mFontSizeSpinBox->value() );
  mTextEdit->setCurrentFont( newFont );

  //color
  mTextEdit->setTextColor( mFontColorButton->color() );
}

void QgsComposerTextWidget::on_mFontColorButton_clicked()
{
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( mFontColorButton->color(), 0, tr( "Select font color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor newColor = QColorDialog::getColor( mFontColorButton->color() );
#endif
  if ( !newColor.isValid() )
  {
    return; //dialog canceled
  }
  mFontColorButton->setColor( newColor );
  changeCurrentFormat();
}


