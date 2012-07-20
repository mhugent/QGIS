#include "qgscomposerhtmlitemwidget.h"
#include "qgscomposerhtmlitem.h"

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

void QgsComposerHtmlItemWidget::on_mUrlLineEdit_editingFinished()
{
  if ( mHtmlItem )
  {
    mHtmlItem->setUrl( QUrl( mUrlLineEdit->text() ) );
    mHtmlItem->update();
  }
}


