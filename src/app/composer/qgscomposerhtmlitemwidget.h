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
    void on_mFileToolButton_clicked();
    /**Sets the GUI elements to the values of mHtmlItem*/
    void setGuiElementValues();

  private:
    QgsComposerHtmlItemWidget();
    void blockSignals( bool block );

    QgsComposerHtmlItem* mHtmlItem;
};

#endif // QGSCOMPOSERHTMLITEMWIDGET_H
