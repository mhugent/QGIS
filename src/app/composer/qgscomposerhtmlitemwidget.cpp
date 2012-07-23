#include "qgscomposerhtmlitemwidget.h"
#include "qgscomposerhtmlitem.h"
#include <QFileDialog>

QgsComposerHtmlItemWidget::QgsComposerHtmlItemWidget( QgsComposerHtmlItem* item ): mHtmlItem( item )
{
  setupUi( this );
}

QgsComposerHtmlItemWidget::QgsComposerHtmlItemWidget()
{
}

QgsComposerHtmlItemWidget::~QgsComposerHtmlItemWidget()
{
}

void QgsComposerHtmlItemWidget::blockSignals( bool block )
{
  mUrlLineEdit->blockSignals( block );
  mFileToolButton->blockSignals( block );
}

void QgsComposerHtmlItemWidget::on_mUrlLineEdit_editingFinished()
{
  if ( mHtmlItem )
  {
    mHtmlItem->beginCommand( tr( "Url changed" ) );
    mHtmlItem->setUrl( QUrl( mUrlLineEdit->text() ) );
    mHtmlItem->update();
    mHtmlItem->endCommand();
  }
}

void QgsComposerHtmlItemWidget::on_mFileToolButton_clicked()
{
  QString file = QFileDialog::getOpenFileName( this, tr( "Select HTML document" ), QString(), "HTML (*.html)" );
  if ( !file.isEmpty() )
  {
    QUrl url = QUrl::fromLocalFile( file );
    mHtmlItem->beginCommand( tr( "Url changed" ) );
    mHtmlItem->setUrl( url );
    mHtmlItem->endCommand();
    mUrlLineEdit->setText( url.toString() );
  }
}

void QgsComposerHtmlItemWidget::on_mResizeToFullContentButton_clicked()
{
  if ( mHtmlItem )
  {
    mHtmlItem->beginCommand( tr( "Set size to html content" ) );
    mHtmlItem->setToFullHtmlContent();
    mHtmlItem->endCommand();
  }
}

void QgsComposerHtmlItemWidget::setGuiElementValues()
{
  if ( !mHtmlItem )
  {
    return;
  }

  blockSignals( true );
  mUrlLineEdit->setText( mHtmlItem->url().toString() );
  blockSignals( false );
}


