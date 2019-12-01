/***************************************************************************
 *  binarytooldialog.cpp                                                   *
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

#include "binarytooldialog.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QComboBox>

namespace Geoprocessing
{

  AbstractBinaryToolDialog::AbstractBinaryToolDialog( QgisInterface *iface, const QString &title, int outputFields, int outputCRS )
    : GeoprocessingToolDialog( iface, title ), mOperatorLayer( 0 )
  {
    mComboOperatorLayer = new QComboBox( this );
    mCheckboxOperatorLayerSelected = new QCheckBox( tr( "Only selected features" ), this );

    QGroupBox *groupBoxOperatorLayer = new QGroupBox( tr( "Operator layer" ), this );
    QVBoxLayout *layoutOperatorLayer = new QVBoxLayout( groupBoxOperatorLayer );
    layoutOperatorLayer->addWidget( mComboOperatorLayer );
    layoutOperatorLayer->addWidget( mCheckboxOperatorLayerSelected );

    static_cast<QVBoxLayout *>( ui.widget_inputs->layout() )->insertWidget( 1, groupBoxOperatorLayer );

    mComboOutputFields = new QComboBox( this );
    mComboOutputFields->addItem( tr( "Input layer" ), AbstractTool::FieldsA );
    mComboOutputFields->addItem( tr( "Operator layer" ), AbstractTool::FieldsB );
    mComboOutputFields->addItem( tr( "Combine" ), AbstractTool::FieldsAandB );

    mComboOutputCRS = new QComboBox( this );
    mComboOutputCRS->addItem( tr( "Input layer" ), AbstractTool::CrsLayerA );
    mComboOutputCRS->addItem( tr( "Operator layer" ), AbstractTool::CrsLayerB );
    mComboOutputCRS->setCurrentIndex( 0 );

    QGridLayout *optionsLayout = static_cast<QGridLayout *>( ui.groupBox_options->layout() );

    if ( outputFields >= 0 )
    {
      mComboOutputFields->setCurrentIndex( outputFields );
      mComboOutputFields->hide();
    }
    else
    {
      mComboOutputFields->setCurrentIndex( 0 );
      int row = optionsLayout->rowCount();
      optionsLayout->addWidget( new QLabel( tr( "Output layer fields:" ) ), row, 0, 1, 1 );
      optionsLayout->addWidget( mComboOutputFields, row, 1, 1, 1 );
    }

    if ( outputCRS >= 0 )
    {
      mComboOutputCRS->setCurrentIndex( outputFields );
      mComboOutputCRS->hide();
    }
    else
    {
      mComboOutputCRS->setCurrentIndex( 0 );
      int row = optionsLayout->rowCount();
      optionsLayout->addWidget( new QLabel( tr( "Output layer CRS:" ) ), row, 0, 1, 1 );
      optionsLayout->addWidget( mComboOutputCRS, row, 1, 1, 1 );
    }

    if ( outputCRS >= 0 && outputFields >= 0 )
    {
      ui.groupBox_options->hide();
    }

    connect( mComboOperatorLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( validateInput() ) );
    connect( mComboOperatorLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setOperatorLayer() ) );
  }

  bool AbstractBinaryToolDialog::getInputValid() const
  {
    return GeoprocessingToolDialog::getInputValid() &&
           mComboOperatorLayer->currentIndex() != -1 &&
           ui.comboBox_inputLayer->currentIndex() != mComboOperatorLayer->currentIndex();
  }

  void AbstractBinaryToolDialog::updateLayers()
  {
    GeoprocessingToolDialog::updateLayers();
    QString curOperator = mComboOperatorLayer->currentText();

    mComboOperatorLayer->blockSignals( true );
    mComboOperatorLayer->clear();

    // Collect layers
    int curOperatorIdx = -1;
    foreach ( QgsMapLayer *layer, QgsProject::instance()->mapLayers() )
    {
      if ( qobject_cast<QgsVectorLayer *>( layer ) )
      {
        mComboOperatorLayer->addItem( layer->name(), layer->id() );
        if ( layer->name() == curOperator )
        {
          curOperatorIdx = mComboOperatorLayer->count() - 1;
        }
      }
    }
    curOperatorIdx = qMax( 0, curOperatorIdx );
    int curInputIdx = ui.comboBox_inputLayer->currentIndex();
    if ( curOperatorIdx == curInputIdx )
    {
      curOperatorIdx = curInputIdx + 1 >= mComboOperatorLayer->count() ? curInputIdx - 1 : curInputIdx + 1;
    }
    mComboOperatorLayer->setCurrentIndex( -1 ); // ensure a signal is emitted below
    mComboOperatorLayer->blockSignals( false );
    mComboOperatorLayer->setCurrentIndex( curOperatorIdx );
  }

  void AbstractBinaryToolDialog::setOperatorLayer()
  {
    if ( mOperatorLayer )
    {
      disconnect( mOperatorLayer, SIGNAL( selectionChanged( const QgsFeatureIds &, const QgsFeatureIds &, bool ) ), this, SLOT( updateOperatorSelectionCheckBox() ) );
    }
    QgsVectorLayer *layer = Utils::getSelectedLayer( mComboOperatorLayer );
    mOperatorLayer = layer;
    if ( mOperatorLayer )
    {
      connect( mOperatorLayer, SIGNAL( selectionChanged( const QgsFeatureIds &, const QgsFeatureIds &, bool ) ), this, SLOT( updateOperatorSelectionCheckBox() ) );
    }
    updateOperatorSelectionCheckBox();
  }

  void AbstractBinaryToolDialog::updateOperatorSelectionCheckBox()
  {
    updateSelectionCheckBox( mCheckboxOperatorLayerSelected, mComboOperatorLayer );
  }

} // Geoprocessing
