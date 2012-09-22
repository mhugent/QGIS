/***************************************************************************
                         qgscomposertextwidget.h
                         ------------------------
    begin                : September, 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

#ifndef QGSCOMPOSERTEXTWIDGET_H
#define QGSCOMPOSERTEXTWIDGET_H

#include "ui_qgscomposertextwidgetbase.h"

class QgsComposerFrame;
class QgsComposerText;

class QgsComposerTextWidget: public QWidget, private Ui::QgsComposerTextWidgetBase
{
    Q_OBJECT
  public:
    QgsComposerTextWidget( QgsComposerText* composerText, QgsComposerFrame* frame );
    ~QgsComposerTextWidget();

  private:
    /**Text document (a clone of the annotation items document)*/
    QgsComposerText* mComposerText;
    QgsComposerFrame* mFrame;

    /**Blocks signals from this widget and all the gui elements*/
    void blockAllSignals( bool block );

  private slots:
    void changeCurrentFormat();
    void on_mFontColorButton_clicked();
    void on_mResizeModeComboBox_currentIndexChanged( int index );
    void handleTextChange();
};

#endif // QGSCOMPOSERTEXTWIDGET_H
