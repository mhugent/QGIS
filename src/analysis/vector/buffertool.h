/***************************************************************************
 *  buffertool.h                                                           *
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

#ifndef VECTORANALYSIS_BUFFER_TOOL_H
#define VECTORANALYSIS_BUFFER_TOOL_H

#include "abstracttool.h"


namespace Vectoranalysis
{
  class ANALYSIS_EXPORT BufferTool : public AbstractTool
  {
    public:
      struct BufferParams
      {
        int segments;
        QgsGeometry::EndCapStyle endCapStyle;
        QgsGeometry::JoinStyle joinStyle;
        double mitreLimit;
        bool singleSided;
        QgsGeometry::BufferSide bufferSide;
      };

      BufferTool( QgsVectorLayer *layer,
                  bool selected,
                  const QString &output,
                  const QString &outputDriverName,
                  double distance,
                  int distanceField,
                  double mapToLayer,
                  BufferParams *params,
                  double precision );
      ~BufferTool();

    private:
      QgsVectorLayer *mLayer;
      bool mSelected;
      double mDistance;
      int mDistanceField;
      double mMapToLayer;
      BufferParams *mParams;

      void prepare();
      void processFeature( const Job *job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_BUFFER_TOOL_H
