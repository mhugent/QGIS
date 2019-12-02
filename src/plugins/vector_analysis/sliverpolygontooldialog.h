/***************************************************************************
 *  sliverpolygontooldialog.h                                              *
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

#ifndef VECTORANALYSIS_SLIVERPOLYGONTOOLDIALOG_H
#define VECTORANALYSIS_SLIVERPOLYGONTOOLDIALOG_H

#include "geoprocessingtooldialog.h"

class QComboBox;
class QDoubleSpinBox;

namespace Geoprocessing
{

  class SliverPolygonToolDialog : public GeoprocessingToolDialog
  {
      Q_OBJECT
    public:
      SliverPolygonToolDialog( QgisInterface *iface );

    private:
      QComboBox *mMethodCombo;
      QDoubleSpinBox *mThresholdSpin;

      AbstractTool *setupTool();

  };

} // Geoprocessing

#endif // VECTORANALYSIS_SLIVERPOLYGONTOOLDIALOG_H
