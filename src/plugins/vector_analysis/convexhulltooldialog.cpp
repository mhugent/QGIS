/***************************************************************************
 *  convexhulltooldialog.cpp                                               *
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

#include "convexhulltooldialog.h"
#include "convexhulltool.h"
#include "vectoranalysisutils.h"

namespace Vectoranalysis
{

  ConvexHullToolDialog::ConvexHullToolDialog( QgisInterface *iface )
    : GeoprocessingToolDialog( iface, tr( "Convex Hull" ) )
  {
    mGroupUI = new Utils::GroupUI( this );
    mSummarizeUI = new Utils::SummarizeUI( this );

    QGridLayout *optionsLayout = static_cast<QGridLayout *>( ui.groupBox_options->layout() );
    mGroupUI->setupUI( ui.comboBox_inputLayer, optionsLayout );
    optionsLayout->addWidget( Utils::createHLine( this ), optionsLayout->rowCount(), 0, 1, 2 );
    mSummarizeUI->setupUI( optionsLayout, Utils::SummarizeNull, Utils::SummarizeNull );
  }

  AbstractTool *ConvexHullToolDialog::setupTool()
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

    return new ConvexHullTool(
             layer, ui.checkBox_inputLayerSelected->isChecked(),
             ui.lineEdit_outputFileName->text(), getOutputDriverName(),
             groupMode, groupField, groupExpression,
             numericSummarizeMode, nonNumericSummarizeMode, getPrecision() );
  }

} // Geoprocessing
