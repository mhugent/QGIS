/***************************************************************************
 *  sliverpolygontooldialog.cpp                                            *
 *  -------------------                                                    *
 *  begin                : Oct 15, 2014                                    *
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

#include "sliverpolygontooldialog.h"
#include "sliverpolygontool.h"
#include "vectoranalysisutils.h"
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"

#include <QDoubleSpinBox>
#include <QMessageBox>

namespace Geoprocessing
{

  SliverPolygonToolDialog::SliverPolygonToolDialog( QgisInterface *iface )
    : GeoprocessingToolDialog( iface, tr( "Eliminate sliver polygons" ) )
  {
    mMethodCombo = new QComboBox();
    mMethodCombo->addItems( QStringList() << tr( "Largest area" ) << tr( "Longest shared boundary" ) );

    mThresholdSpin = new QDoubleSpinBox();
    mThresholdSpin->setRange( 0, 1000 );
    mThresholdSpin->setDecimals( 4 );
    mThresholdSpin->setValue( 0.2 );

    QGridLayout *optionsLayout = static_cast<QGridLayout *>( ui.groupBox_options->layout() );
    int row = optionsLayout->rowCount();
    optionsLayout->addWidget( new QLabel( tr( "Merge method:" ) ), row + 0, 0, 1, 1 );
    optionsLayout->addWidget( mMethodCombo, row + 0, 1, 1, 1 );
    optionsLayout->addWidget( new QLabel( tr( "Threshold (map units sqr.):" ) ), row + 1, 0, 1, 1 );
    optionsLayout->addWidget( mThresholdSpin, row + 1, 1, 1, 1 );

  }

  AbstractTool *SliverPolygonToolDialog::setupTool()
  {
    QgsVectorLayer *layer = Utils::getSelectedLayer( ui.comboBox_inputLayer );

    SliverPolygonTool::MergeMethod method = static_cast<SliverPolygonTool::MergeMethod>( mMethodCombo->currentIndex() );

    if ( layer->geometryType() != QgsWkbTypes::PolygonGeometry )
    {
      QMessageBox::warning( this, tr( "Invalid input" ), tr( "Layer must be a polygon layer." ) );
      return 0;
    }

    double threshMapUnits = mThresholdSpin->value();
    double layerToMapUnits = mIface->mapCanvas()->mapSettings().layerToMapUnits( layer );
    double threshold = threshMapUnits / ( layerToMapUnits * layerToMapUnits );

    return new SliverPolygonTool( layer, ui.checkBox_inputLayerSelected->isChecked(),
                                  ui.lineEdit_outputFileName->text(), getOutputDriverName(),
                                  method, threshold, getPrecision() );
  }

} // Geoprocessing
