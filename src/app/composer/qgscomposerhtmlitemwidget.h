#ifndef QGSCOMPOSERHTMLITEMWIDGET_H
#define QGSCOMPOSERHTMLITEMWIDGET_H

#include "ui_qgscomposerhtmlitemwidgetbase.h"

class QgsComposerHtmlItem;

class QgsComposerHtmlItemWidget: public QWidget, private Ui::QgsComposerHtmlItemWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerHtmlItemWidget( QgsComposerHtmlItem* item );
    ~QgsComposerHtmlItemWidget();

  private slots:
    void on_mUrlLineEdit_editingFinished();

  private:
    QgsComposerHtmlItemWidget();

    QgsComposerHtmlItem* mHtmlItem;
};

#endif // QGSCOMPOSERHTMLITEMWIDGET_H
