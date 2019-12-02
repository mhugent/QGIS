/***************************************************************************
 *  dissolvetooldialog.cpp                                                 *
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

#include "dissolvetooldialog.h"
#include "dissolvetool.h"
#include "vectoranalysisutils.h"
#include "qgsvectorlayer.h"

#include <QMessageBox>

namespace Vectoranalysis
{

  DissolveToolDialog::DissolveToolDialog( QgisInterface *iface )
    : GeoprocessingToolDialog( iface, tr( "Dissolve" ) )
  {
    mGroupUI = new Utils::GroupUI( this );
    mSummarizeUI = new Utils::SummarizeUI( this );

    mMultiPartCheckbox = new QCheckBox( tr( "Allow multi-part features" ) );

    QGridLayout *optionsLayout = static_cast<QGridLayout *>( ui.groupBox_options->layout() );
    mGroupUI->setupUI( ui.comboBox_inputLayer, optionsLayout, tr( "Dissolve" ) );
    optionsLayout->addWidget( Utils::createHLine(), optionsLayout->rowCount(), 0, 1, 2 );
    mSummarizeUI->setupUI( optionsLayout, Utils::SummarizeMean, Utils::SummarizeFirst );
    optionsLayout->addWidget( Utils::createHLine(), optionsLayout->rowCount(), 0, 1, 2 );
    optionsLayout->addWidget( mMultiPartCheckbox, optionsLayout->rowCount(), 0, 1, 2 );
  }

  AbstractTool *DissolveToolDialog::setupTool()
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

    QgsVectorLayer *layer = Utils::getSelectedLayer( ui.comboBox_inputLayer );
    if ( layer->geometryType() != QgsWkbTypes::PolygonGeometry )
    {
      QMessageBox::warning( this, tr( "Invalid input" ), tr( "Input and operator layers must both be polygon layers." ) );
      return 0;
    }

    return new DissolveTool(
             layer, ui.checkBox_inputLayerSelected->isChecked(),
             ui.lineEdit_outputFileName->text(), getOutputDriverName(),
             groupMode, groupField, groupExpression,
             numericSummarizeMode, nonNumericSummarizeMode, mMultiPartCheckbox->isChecked(), getPrecision() );
  }

} // Geoprocessing
