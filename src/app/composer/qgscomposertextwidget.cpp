/***************************************************************************
                         qgscomposertextwidget.cpp
                         -------------------------
    begin                : September, 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

#include "qgscomposertextwidget.h"
#include "qgscomposerframe.h"
#include "qgscomposeritemwidget.h"
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
  connect( mTextEdit, SIGNAL( textChanged() ), this, SLOT( handleTextChange() ) );

  //options for resize mode
  blockAllSignals( true );
  mResizeModeComboBox->addItem( tr( "Use existing frames" ), QgsComposerMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to next page" ), QgsComposerMultiFrame::ExtendToNextPage );
  mResizeModeComboBox->addItem( tr( "Repeat on every page" ), QgsComposerMultiFrame::RepeatOnEveryPage );
  mResizeModeComboBox->addItem( tr( "Repeat until finished" ), QgsComposerMultiFrame::RepeatUntilFinished );
  if( mComposerText )
  {
    mResizeModeComboBox->setCurrentIndex( mResizeModeComboBox->findData( mComposerText->resizeMode() ) );
  }
  blockAllSignals( false );

  //embed widget for general options
  if ( mFrame )
  {
    //add widget for general composer item properties
    QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mFrame );
    mToolBox->addItem( itemPropertiesWidget, tr( "General options" ) );
  }
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

void QgsComposerTextWidget::on_mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mComposerText )
  {
    return;
  }

  QgsComposition* composition = mComposerText->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerText, tr( "Change resize mode" ) );
    mComposerText->setResizeMode(( QgsComposerMultiFrame::ResizeMode )mResizeModeComboBox->itemData( index ).toInt() );
    composition->endMultiFrameCommand();
  }
}

void QgsComposerTextWidget::handleTextChange()
{
  if ( mComposerText )
  {
    mComposerText->recalculateFrameSizes();
    mComposerText->update();
  }
}

void QgsComposerTextWidget::blockAllSignals( bool block )
{
    blockSignals( block );
    mResizeModeComboBox->blockSignals( block );
    mFontComboBox->blockSignals( block );
    mFontSizeSpinBox->blockSignals( block );
    mBoldPushButton->blockSignals( block );
    mItalicsPushButton->blockSignals( block );
    mFontColorButton->blockSignals( block );
    mTextEdit->blockSignals( block );
}


