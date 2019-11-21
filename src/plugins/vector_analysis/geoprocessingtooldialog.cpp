/***************************************************************************
 *  geoprocessingtooldialog.cpp                                            *
 *  -------------------                                                    *
 *  begin                : Jun 10, 2014                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisinterface.h"
#include "geoprocessingtooldialog.h"
#include "geoprocessingerrordialog.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "abstracttool.h"
#include "vectoranalysisutils.h"
#include <QFutureWatcher>
#include <QEventLoop>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QPlainTextEdit>

namespace Geoprocessing
{

  GeoprocessingToolDialog::GeoprocessingToolDialog( QgisInterface* iface, const QString &title )
      : QDialog( iface->mainWindow() ), mIface( iface ), mInputLayer( 0 )
  {
    ui.setupUi( this );
    setWindowTitle( title );
    ui.buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    ui.buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "Run" ) );
    ui.buttonBox->button( QDialogButtonBox::Cancel )->hide();
    ui.progressBar->hide();
    ui.label_status->setText( tr( "<b>Ready</b>" ) );
    ui.checkBox_addOutput->setChecked( true );
    connect( ui.buttonBox->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( runTool() ) );
    connect( ui.buttonBox->button( QDialogButtonBox::Close ), SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( ui.pushButton_selectOutput, SIGNAL( clicked() ), this, SLOT( selectOutputFile() ) );
    connect( ui.comboBox_inputLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( validateInput() ) );
    connect( ui.comboBox_inputLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setInputLayer() ) );
    connect( ui.lineEdit_outputFileName, SIGNAL( textChanged( QString ) ), this, SLOT( validateInput() ) );
    connect( QgsProject::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( updateLayers() ) );
    connect( QgsProject::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( updateLayers() ) );
    connect( this, SIGNAL( shown() ), this, SLOT( updateLayers() ) );
  }

  void GeoprocessingToolDialog::closeEvent( QCloseEvent* ev )
  {
    ui.buttonBox->button( QDialogButtonBox::Close )->isEnabled() ? ev->accept() : ev->ignore();
  }

  void GeoprocessingToolDialog::updateLayers()
  {
    QString curInput = ui.comboBox_inputLayer->currentText();

    ui.comboBox_inputLayer->blockSignals( true );
    ui.comboBox_inputLayer->clear();

    // Collect layers
    QgsMapLayer* currentLayer = mIface->mapCanvas()->currentLayer();
    int curInputIdx = -1;
    foreach ( QgsMapLayer* layer, QgsProject::instance()->mapLayers() )
    {
      if ( qobject_cast<QgsVectorLayer*>( layer ) )
      {
        ui.comboBox_inputLayer->addItem( layer->name(), layer->id() );
        if ( layer->name() == curInput )
        {
          curInputIdx = ui.comboBox_inputLayer->count() - 1;
        }
        else if ( curInputIdx == -1 && layer == currentLayer )
        {
          curInputIdx = ui.comboBox_inputLayer->count() - 1;
        }
      }
    }
    ui.comboBox_inputLayer->setCurrentIndex( -1 ); // ensure a signal is emitted below
    ui.comboBox_inputLayer->blockSignals( false );
    ui.comboBox_inputLayer->setCurrentIndex( qMax( 0, curInputIdx ) );
  }

  void GeoprocessingToolDialog::runTool()
  {
    AbstractTool* tool = setupTool();
    if ( !tool )
    {
      return;
    }

    QPushButton* okBtn = ui.buttonBox->button( QDialogButtonBox::Ok );
    QPushButton* cancelBtn = ui.buttonBox->button( QDialogButtonBox::Cancel );
    QPushButton* closeBtn = ui.buttonBox->button( QDialogButtonBox::Close );

    QEventLoop evLoop;
    QFutureWatcher<void> futureWatcher;
    connect( &futureWatcher, SIGNAL( progressRangeChanged( int, int ) ), this, SLOT( setProgressRange( int, int ) ) );
    connect( &futureWatcher, SIGNAL( progressValueChanged( int ) ), this, SLOT( setProgressValue( int ) ) );
    connect( &futureWatcher, SIGNAL( finished() ), &evLoop, SLOT( quit() ) );
    connect( cancelBtn, SIGNAL( clicked() ), &futureWatcher, SLOT( cancel() ) );

    hide();
    setModal( true );
    setCursor( Qt::WaitCursor );
    okBtn->hide();
    cancelBtn->show();
    closeBtn->setEnabled( false );
    ui.widget_inputs->setEnabled( false );
    ui.progressBar->show();
    ui.progressBar->setRange( 0, 0 );
    ui.progressBar->setValue( 0 );
    mProgressMin = mProgressMax = 0;
    mProgressNTasks = mProgressCurTask = 0;
    ui.label_status->hide();
    show();

    futureWatcher.setFuture( tool->init() );
    evLoop.exec();

    mProgressNTasks = tool->getTaskCount();
    for ( int i = 0, n = mProgressNTasks; i < n; ++i )
    {
      mProgressCurTask = i;
      futureWatcher.setFuture( tool->execute( i ) );
      evLoop.exec();
      if ( futureWatcher.isCanceled() )
      {
        break;
      }
    }

    tool->finalizeOutput();
    hide();
    setModal( false );
    unsetCursor();
    okBtn->show();
    cancelBtn->setEnabled( true );
    cancelBtn->hide();
    closeBtn->setEnabled( true );
    ui.widget_inputs->setEnabled( true );
    ui.progressBar->hide();
    ui.label_status->show();
    show();

    if ( futureWatcher.isCanceled() )
    {
      ui.label_status->setText( tr( "<b>Canceled</b>" ) );

      QString outputFileName = ui.lineEdit_outputFileName->text();
      QFile( outputFileName ).remove();
      if ( mOutputDriverName == "ESRI Shapefile" )
      {
        QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "dbf" ) ).remove();
        QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "prj" ) ).remove();
        QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "qpj" ) ).remove();
        QFile( QString( outputFileName ).replace( QRegExp( "shp$" ), "shx" ) ).remove();
      }
      if ( !tool->getExceptions().isEmpty() )
      {
        QDialog exceptionDialog( this );
        exceptionDialog.setWindowTitle( tr( "Process aborted" ) );
        exceptionDialog.setLayout( new QVBoxLayout() );
        exceptionDialog.layout()->addWidget( new QLabel( tr( "The process was aborted because unhandled exceptions occurred:" ) ) );
        QPlainTextEdit* exceptionEdit = new QPlainTextEdit();
        exceptionEdit->setPlainText( tool->getExceptions().join( "\n" ) );
        exceptionEdit->setReadOnly( true );
        exceptionDialog.layout()->addWidget( exceptionEdit );
        QDialogButtonBox* bbox = new QDialogButtonBox( QDialogButtonBox::Ok );
        connect( bbox, SIGNAL( accepted() ), &exceptionDialog, SLOT( accept() ) );
        connect( bbox, SIGNAL( rejected() ), &exceptionDialog, SLOT( reject() ) );
        exceptionDialog.layout()->addWidget( bbox );
        exceptionDialog.exec();
      }
    }
    else
    {
      ui.label_status->setText( tr( "<b>Completed</b>" ) );

      if ( ui.checkBox_addOutput->isChecked() )
      {
        QString layerName = ui.lineEdit_outputFileName->text();
        QgsVectorLayer* layer = new QgsVectorLayer( layerName, QFileInfo( layerName ).completeBaseName(), "ogr" );

        // Remove existing layer with same uri
        QStringList toRemove;
        for( QgsMapLayer* maplayer : QgsProject::instance()->mapLayers() )
        {
          if ( dynamic_cast<QgsVectorLayer*>( maplayer ) &&
               static_cast<QgsVectorLayer*>( maplayer )->dataProvider()->dataSourceUri() == layer->dataProvider()->dataSourceUri() )
          {
            toRemove.append( maplayer->id() );
          }
        }
        if ( !toRemove.isEmpty() )
        {
          QgsProject::instance()->removeMapLayers( toRemove );
        }

        QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer );
      }
      if ( tool->errorsOccurred() )
      {
        GeoprocessingErrorDialog dialog( mIface, tool );
        QEventLoop loop;
        connect( &dialog, SIGNAL( finished( int ) ), &loop, SLOT( quit() ) );
        dialog.setWindowModality( Qt::WindowModal );
        dialog.show();
        loop.exec();
      }
    }

    delete tool;
  }

  void GeoprocessingToolDialog::selectOutputFile()
  {
    QString filterString = QgsVectorFileWriter::filterForDriver( "ESRI Shapefile" );
    QList< QgsVectorFileWriter::FilterFormatDetails > filterFormatList = QgsVectorFileWriter::supportedFiltersAndFormats();
    for( const QgsVectorFileWriter::FilterFormatDetails& detail : filterFormatList )
    {
      QString driverName = detail.driverName;
      if ( driverName != "ESRI Shapefile" ) // Default entry, first in list (see above)
      {
        filterString += ";;" + detail.filterString;
      }
    }
    QString initialdir;
    QgsVectorLayer* layer = Utils::getSelectedLayer( ui.comboBox_inputLayer );
    if ( layer )
    {
      QDir dir = QFileInfo( layer->dataProvider()->dataSourceUri() ).dir();
      if ( dir.exists() )
      {
        initialdir = dir.absolutePath();
      }
    }
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName( this, tr( "Select Output File" ), initialdir, filterString, &selectedFilter );
    if ( !filename.isEmpty() )
    {
      mOutputDriverName.clear();
      for( const QgsVectorFileWriter::FilterFormatDetails& detail : filterFormatList )
      {
          if( detail.filterString == selectedFilter )
          {
              mOutputDriverName = detail.driverName;
              break;
          }
      }

      if( !mOutputDriverName.isEmpty() )
      {
        QgsVectorFileWriter::MetaData mdata;
        if ( QgsVectorFileWriter::driverMetadata( mOutputDriverName, mdata ) )
        {
            if ( !filename.endsWith( QString( ".%1" ).arg( mdata.ext ), Qt::CaseInsensitive ) )
            {
                filename += QString( ".%1" ).arg( mdata.ext );
            }
        }
      }
      ui.lineEdit_outputFileName->setText( filename );
    }
  }

  bool GeoprocessingToolDialog::getInputValid() const
  {
    return !ui.lineEdit_outputFileName->text().isEmpty() && ui.comboBox_inputLayer->currentIndex() != -1;
  }

  double GeoprocessingToolDialog::getPrecision() const
  {
    return qPow( 10, -ui.spinBox_precision->value() );
  }

  void GeoprocessingToolDialog::validateInput()
  {
    ui.buttonBox->button( QDialogButtonBox::Ok )->setEnabled( getInputValid() );
  }

  void GeoprocessingToolDialog::setProgressRange( int min, int max )
  {
    mProgressMin = min;
    mProgressMax = max;
    if ( ui.progressBar->value() != 0 )
    {
      ui.progressBar->setRange( min, max );
    }
  }

  void GeoprocessingToolDialog::setProgressValue( int value )
  {
    ui.progressBar->setValue( value );
    if ( value == 0 )
    {
      ui.progressBar->setRange( 0, 0 );
    }
    else
    {
      ui.progressBar->setRange( mProgressMin, mProgressMax );
      if ( mProgressNTasks < 2 )
      {
        ui.progressBar->setFormat( "%p%" );
      }
      else
      {
        ui.progressBar->setFormat( tr( "%p% (Step %1/%2)" ).arg( mProgressCurTask + 1 ).arg( mProgressNTasks ) );
      }
    }
  }

  void GeoprocessingToolDialog::setInputLayer()
  {
    if ( mInputLayer )
    {
      disconnect( mInputLayer, SIGNAL( selectionChanged( const QgsFeatureIds&, const QgsFeatureIds&, bool ) ), this, SLOT( updateInputSelectionCheckBox() ) );
    }
    QgsVectorLayer* layer = Utils::getSelectedLayer( ui.comboBox_inputLayer );
    mInputLayer = layer;

    if ( mInputLayer )
    {
      connect( mInputLayer, SIGNAL( selectionChanged( const QgsFeatureIds&, const QgsFeatureIds&, bool ) ), this, SLOT( updateInputSelectionCheckBox() ) );
    }
    updateInputSelectionCheckBox();
  }

  void GeoprocessingToolDialog::updateInputSelectionCheckBox()
  {
    updateSelectionCheckBox( ui.checkBox_inputLayerSelected, ui.comboBox_inputLayer );
  }

  void GeoprocessingToolDialog::updateSelectionCheckBox( QCheckBox* selectionCheckBox, QComboBox* layerComboBox )
  {
    if ( !selectionCheckBox || !layerComboBox )
    {
      return;
    }

    QgsVectorLayer* layer = Utils::getSelectedLayer( layerComboBox );
    if ( !layer )
    {
      selectionCheckBox->setDisabled( true );
      return;
    }

    int nSelectedFeatures = layer->selectedFeatureCount();
    QString checkboxText = tr( "Only selected features ( %1 features selected )" ).arg( nSelectedFeatures );
    selectionCheckBox->setText( checkboxText );
    selectionCheckBox->setDisabled( nSelectedFeatures < 1 );
    if ( nSelectedFeatures < 1 )
    {
      selectionCheckBox->setCheckState( Qt::Unchecked );
    }
  }

} // Geoprocessing
