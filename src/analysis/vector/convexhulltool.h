/***************************************************************************
 *  convexhulltool.h                                                       *
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

#ifndef VECTORANALYSIS_CONVEXHULL_TOOL_H
#define VECTORANALYSIS_CONVEXHULL_TOOL_H

#include "abstracttool.h"
#include "utils.h"

class QgsExpression;

namespace Vectoranalysis
{

  class ANALYSIS_EXPORT ConvexHullTool : public AbstractTool
  {
    public:
      ConvexHullTool( QgsVectorLayer *layer,
                      bool selected,
                      const QString &output,
                      const QString &outputDriverName,
                      Utils::GroupMode groupMode,
                      int groupField,
                      QgsExpression *groupExpression,
                      Utils::SummarizeMode numericSummarizeMode,
                      Utils::SummarizeMode nonNumericSummarizeMode,
                      double precision );
      ~ConvexHullTool();

    private:
      struct ConvexHullJob : AbstractTool::Job
      {
        ConvexHullJob( const QgsFeatureIds _cluster ) : Job( 0, 0 ), cluster( _cluster ) {}
        QgsFeatureIds cluster;
      };

      QgsVectorLayer *mLayer;
      bool mSelected;
      Utils::GroupMode mGroupMode;
      int mGroupField;
      QgsExpression *mGroupExpression;
      Utils::summarizer_t mNumericSummarizer;
      Utils::summarizer_t mNonNumericSummarizer;

      void prepare();
      void processFeature( const Job *job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_CONVEXHULL_TOOL_H
