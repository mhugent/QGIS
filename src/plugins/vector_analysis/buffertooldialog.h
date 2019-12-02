/***************************************************************************
 *  buffertooldialog.h                                                     *
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

#ifndef VECTORANALYSIS_BUFFERTOOLDIALOG_H
#define VECTORANALYSIS_BUFFERTOOLDIALOG_H

#include "geoprocessingtooldialog.h"

class QDoubleSpinBox;
class QSpinBox;

namespace Vectoranalysis
{

  namespace Utils { class SummarizeUI; }
  namespace Utils { class GroupUI; }

  class BufferToolDialog : public GeoprocessingToolDialog
  {
      Q_OBJECT
    public:
      BufferToolDialog( QgisInterface *iface );

    private:
      QDoubleSpinBox *mDistanceSpin;
      QSpinBox *mSegmentsSpin;
      QComboBox *mDistanceFieldCombo;
      QComboBox *mCapStyleCombo;
      QComboBox *mJoinStyleCombo;
      QDoubleSpinBox *mMitreLimitSpin;
      QCheckBox *mSingleSidedCheckBox;
      QGroupBox *mDissolveGroupbox;
      Utils::GroupUI *mGroupUI;
      Utils::SummarizeUI *mSummarizeUI;
      QCheckBox *mMultiPartCheckbox;

      AbstractTool *setupTool();

    private slots:
      void populateDistanceFields();
  };

} // Geoprocessing

#endif // VECTORANALYSIS_BUFFERTOOLDIALOG_H
