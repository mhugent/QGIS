/***************************************************************************
 *  differencetool.h                                                   *
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

#ifndef VECTORANALYSIS_DIFFERENCE_TOOL_H
#define VECTORANALYSIS_DIFFERENCE_TOOL_H

#include "abstracttool.h"

namespace Geoprocessing
{

  class DifferenceTool : public AbstractTool
  {
    public:
      DifferenceTool( QgsVectorLayer *layerA,
                      QgsVectorLayer *layerB,
                      bool selectedA, bool selectedB,
                      const QString& output,
                      const QString& outputDriverName,
                      OutputFields /*outputFields*/, OutputCrs outputCrs,
                      double precision );
      ~DifferenceTool() {}
      static bool validateInputs( QgsVectorLayer *layerA, QgsVectorLayer *layerB, QString& errorMsgs );

    private:
      QgsSpatialIndex mSpatialIndex;
      QgsVectorLayer* mLayerA;
      QgsVectorLayer* mLayerB;
      bool mSelectedA;
      bool mSelectedB;

      void prepare();
      void processFeature( const Job* job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_DIFFERENCE_TOOL_H
