/***************************************************************************
 *  dissolvetooldialog.h                                                   *
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

#ifndef VECTORANALYSIS_DISSOLVETOOLDIALOG_H
#define VECTORANALYSIS_DISSOLVETOOLDIALOG_H

#include "geoprocessingtooldialog.h"

namespace Geoprocessing
{

  namespace Utils { class SummarizeUI; }
  namespace Utils { class GroupUI; }

  class DissolveToolDialog : public GeoprocessingToolDialog
  {
      Q_OBJECT
    public:
      DissolveToolDialog( QgisInterface *iface );

    private:
      Utils::GroupUI *mGroupUI;
      Utils::SummarizeUI *mSummarizeUI;
      QCheckBox *mMultiPartCheckbox;

      AbstractTool *setupTool();
  };

} // Geoprocessing

#endif // VECTORANALYSIS_DISSOLVETOOLDIALOG_H
