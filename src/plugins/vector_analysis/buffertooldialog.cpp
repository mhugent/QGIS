/***************************************************************************
 *  buffertooldialog.cpp                                                   *
 *  -------------------                                                    *
 *  begin                : Jul 10, 2014                                    *
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

#include "buffertooldialog.h"
#include "buffertool.h"
#include "dissolvetool.h"
#include "vectoranalysisutils.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QRadioButton>
#include <QSpinBox>

namespace Vectoranalysis
{

  BufferToolDialog::BufferToolDialog( QgisInterface *iface )
    : GeoprocessingToolDialog( iface, tr( "Buffer" ) )
  {
    mDistanceSpin = new QDoubleSpinBox();
    mDistanceSpin->setRange( -1.E8, 1.E8 );
    mDistanceSpin->setDecimals( 7 );

    mDistanceFieldCombo = new QComboBox();
    mDistanceFieldCombo->setEnabled( false );

    QRadioButton *distanceValueRadio = new QRadioButton( tr( "Buffer distance value:" ) );
    distanceValueRadio->setChecked( true );
    QRadioButton *distanceFieldRadio = new QRadioButton( tr( "Buffer distance field:" ) );

    mSegmentsSpin = new QSpinBox();
    mSegmentsSpin->setRange( 0, 100 );
    mSegmentsSpin->setValue( 8 );

    mCapStyleCombo = new QComboBox();
    mCapStyleCombo->addItem( tr( "Round" ), 1 );
    mCapStyleCombo->addItem( tr( "Flat" ), 2 );
    mCapStyleCombo->addItem( tr( "Square" ), 3 );

    mJoinStyleCombo = new QComboBox();
    mJoinStyleCombo->addItem( tr( "Round" ), 1 );
    mJoinStyleCombo->addItem( tr( "Mitre" ), 2 );
    mJoinStyleCombo->addItem( tr( "Bevel" ), 3 );

    mMitreLimitSpin = new QDoubleSpinBox();
    mMitreLimitSpin->setRange( 0, 100 );
    mMitreLimitSpin->setValue( 4 );
    mMitreLimitSpin->setDecimals( 2 );

    mSingleSidedCheckBox = new QCheckBox( tr( "Single sided" ) );

    mDissolveGroupbox = new QGroupBox( tr( "Dissolve buffer results" ) );
    mDissolveGroupbox->setCheckable( true );
    mDissolveGroupbox->setChecked( false );
    mDissolveGroupbox->setFlat( true );

    mGroupUI = new Utils::GroupUI( this );
    mSummarizeUI = new Utils::SummarizeUI( this );

    mMultiPartCheckbox = new QCheckBox( tr( "Allow multi-part features" ) );

    QGridLayout *optionsLayout = static_cast<QGridLayout *>( ui.groupBox_options->layout() );
    int row = optionsLayout->rowCount();
    optionsLayout->addWidget( distanceValueRadio, row + 0, 0, 1, 1 );
    optionsLayout->addWidget( mDistanceSpin, row + 0, 1, 1, 1 );
    optionsLayout->addWidget( distanceFieldRadio, row + 1, 0, 1, 1 );
    optionsLayout->addWidget( mDistanceFieldCombo, row + 1, 1, 1, 1 );
    optionsLayout->addWidget( Utils::createHLine(), row + 2, 0, 1, 2 );
    optionsLayout->addWidget( new QLabel( tr( "Curve Segments:" ) ), row + 3, 0, 1, 1 );
    optionsLayout->addWidget( mSegmentsSpin, row + 3, 1, 1, 1 );
    optionsLayout->addWidget( new QLabel( tr( "Cap style:" ) ), row + 4, 0, 1, 1 );
    optionsLayout->addWidget( mCapStyleCombo, row + 4, 1, 1, 1 );
    optionsLayout->addWidget( new QLabel( tr( "Join style:" ) ), row + 5, 0, 1, 1 );
    optionsLayout->addWidget( mJoinStyleCombo, row + 5, 1, 1, 1 );
    optionsLayout->addWidget( new QLabel( tr( "Mitre limit:" ) ), row + 6, 0, 1, 1 );
    optionsLayout->addWidget( mMitreLimitSpin, row + 6, 1, 1, 1 );
    optionsLayout->addWidget( mSingleSidedCheckBox, row + 7, 0, 1, 2 );
    optionsLayout->addWidget( Utils::createHLine(), row + 8, 0, 1, 2 );
    optionsLayout->addWidget( mDissolveGroupbox, row + 9, 0, 1, 2 );

    QGridLayout *dissolveLayout = new QGridLayout( mDissolveGroupbox );
    mGroupUI->setupUI( ui.comboBox_inputLayer, dissolveLayout, tr( "Dissolve" ) );
    dissolveLayout->addWidget( Utils::createHLine(), dissolveLayout->rowCount(), 0, 1, 2 );
    mSummarizeUI->setupUI( dissolveLayout, Utils::SummarizeMean, Utils::SummarizeFirst );
    dissolveLayout->addWidget( Utils::createHLine(), dissolveLayout->rowCount(), 0, 1, 2 );
    dissolveLayout->addWidget( mMultiPartCheckbox, dissolveLayout->rowCount(), 0, 1, 2 );

    connect( distanceValueRadio, SIGNAL( toggled( bool ) ), mDistanceSpin, SLOT( setEnabled( bool ) ) );
    connect( distanceFieldRadio, SIGNAL( toggled( bool ) ), mDistanceFieldCombo, SLOT( setEnabled( bool ) ) );
    connect( ui.comboBox_inputLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( populateDistanceFields() ) );
  }

  AbstractTool *BufferToolDialog::setupTool()
  {
    QgsVectorLayer *layer = Utils::getSelectedLayer( ui.comboBox_inputLayer );
    if ( !mDistanceFieldCombo->isEnabled() )
    {
      if ( layer->geometryType() == QgsWkbTypes::PolygonGeometry && mDistanceSpin->value() == 0. )
      {
        QMessageBox::warning( this, tr( "Invalid input" ), tr( "Distance must be non-zero for polygon layers." ) );
        return 0;
      }
      else if ( layer->geometryType() != QgsWkbTypes::PolygonGeometry && mDistanceSpin->value() <= 0. )
      {
        QMessageBox::warning( this, tr( "Invalid input" ), tr( "Distance must be positive for point and line layers." ) );
        return 0;
      }
    }

    int distanceField = -1;
    if ( mDistanceFieldCombo->isEnabled() && mDistanceFieldCombo->currentIndex() > 0 )
    {
      distanceField = mDistanceFieldCombo->itemData( mDistanceFieldCombo->currentIndex() ).toInt();
    }

    BufferTool::BufferParams *params = new BufferTool::BufferParams;
    params->endCapStyle = ( QgsGeometry::EndCapStyle )( mCapStyleCombo->itemData( mCapStyleCombo->currentIndex() ).toInt() );
    params->joinStyle = ( QgsGeometry::JoinStyle )( mJoinStyleCombo->itemData( mJoinStyleCombo->currentIndex() ).toInt() );
    params->mitreLimit = mMitreLimitSpin->value();
    params->segments = mSegmentsSpin->value();
    params->singleSided = mSingleSidedCheckBox->isChecked();

    double mapToLayer = 1. / mIface->mapCanvas()->mapSettings().layerToMapUnits( layer, mIface->mapCanvas()->extent() );

    if ( mDissolveGroupbox->isChecked() )
    {
      Utils::SummarizeMode numericSummarizeMode, nonNumericSummarizeMode;
      mSummarizeUI->getSettings( numericSummarizeMode, nonNumericSummarizeMode );

      Utils::GroupMode groupMode;
      int groupField;
      QgsExpression *groupExpression;
      if ( !mGroupUI->getSettings( groupMode, groupField, groupExpression ) )
      {
        return 0;
      }

      return new DissolveTool(
               layer, ui.checkBox_inputLayerSelected->isChecked(),
               ui.lineEdit_outputFileName->text(), getOutputDriverName(),
               groupMode, groupField, groupExpression,
               numericSummarizeMode, nonNumericSummarizeMode, mMultiPartCheckbox->isChecked(),
               getPrecision(), mDistanceSpin->value(), distanceField, mapToLayer, params );
    }
    else
    {
      return new BufferTool(
               layer, ui.checkBox_inputLayerSelected->isChecked(),
               ui.lineEdit_outputFileName->text(), getOutputDriverName(),
               mDistanceSpin->value(), distanceField, mapToLayer, params, getPrecision() );
    }
  }

  void BufferToolDialog::populateDistanceFields()
  {
    mDistanceFieldCombo->clear();
    QgsVectorLayer *layer = Utils::getSelectedLayer( ui.comboBox_inputLayer );
    const QgsFields &fields = layer->fields();
    for ( int key = 0, nKeys = fields.count(); key < nKeys; ++key )
    {
      // Don't list primary keys...
      if ( !layer->dataProvider()->pkAttributeIndexes().contains( key ) )
      {
        mDistanceFieldCombo->addItem( fields[key].name(), key );
      }
    }
    if ( mDistanceFieldCombo->count() > 0 )
    {
      mDistanceFieldCombo->setCurrentIndex( 0 );
    }
  }

} // Geoprocessing
