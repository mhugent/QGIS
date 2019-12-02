/***************************************************************************
 *  dissolvetool.h                                                     *
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

#ifndef VECTORANALYSIS_DISSOLVE_TOOL_H
#define VECTORANALYSIS_DISSOLVE_TOOL_H

#include "abstracttool.h"
#include "buffertool.h"
#include "utils.h"

#include <QSet>

class QgsExpression;

namespace Vectoranalysis
{

  class ANALYSIS_EXPORT DissolveTool : public AbstractTool
  {
    public:
      DissolveTool( QgsVectorLayer *layer,
                    bool selected,
                    const QString &output,
                    const QString &outputDriverName,
                    Utils::GroupMode groupMode,
                    int groupField,
                    QgsExpression *groupExpression,
                    Utils::SummarizeMode numericSummarizeMode,
                    Utils::SummarizeMode nonNumericSummarizeMode,
                    bool allowMultipart,
                    double precision,
                    double bufferDistance = 0.0,
                    int bufferDistanceField = -1,
                    double mapToLayer = 1.,
                    BufferTool::BufferParams *bufferParams = 0 );
      ~DissolveTool();

    private:
      struct DissolveJob : AbstractTool::Job
      {
        DissolveJob( const QgsFeatureIds &_cluster ) : Job( 0, 0 ), cluster( _cluster ) {}
        QgsFeatureIds cluster;
      };

      QgsVectorLayer *mLayer;
      bool mSelected;
      QSet<int> mProcessedIds;
      Utils::GroupMode mGroupMode;
      int mGroupField;
      QgsExpression *mGroupExpression;
      bool mAllowMultipart;

      double mBufferDistance;
      int mBufferDistanceField;
      double mMapToLayer;
      BufferTool::BufferParams *mBufferParams;

      typedef QVariant( *Summarizer_t )( int, const QVector<const QgsAttributeMap *> & );
      Utils::summarizer_t mNumericSummarizer;
      Utils::summarizer_t mNonNumericSummarizer;

      void prepare();
      void processFeature( const Job *job );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_DISSOLVE_TOOL_H
