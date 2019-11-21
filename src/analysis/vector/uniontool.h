/***************************************************************************
 *  unionaytool.h                                                      *
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

#ifndef VECTORANALYSIS_UNION_TOOL_H
#define VECTORANALYSIS_UNION_TOOL_H

#include "abstracttool.h"

namespace Geoprocessing
{

  class UnionTool : public AbstractTool
  {
    public:
      UnionTool( QgsVectorLayer *layerA,
                 QgsVectorLayer *layerB,
                 bool selectedA, bool selectedB,
                 const QString& output,
                 const QString& outputDriverName,
                 OutputFields outputFields,
                 OutputCrs outputCrs,
                 double precision );

      static bool validateInputs( QgsVectorLayer *layerA, QgsVectorLayer *layerB, QString& errorMsgs );

    private:
      enum Task
      {
        ProcessLayerAFeature,
        ProcessLayerBFeature
      };

      QgsSpatialIndex mSpatialIndexA;
      QgsSpatialIndex mSpatialIndexB;
      QgsVectorLayer* mLayerA;
      QgsVectorLayer* mLayerB;
      bool mSelectedA;
      bool mSelectedB;
      OutputFields mOutputFileds;

      void prepare();
      void processFeature( const Job* job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_UNION_TOOL_H
