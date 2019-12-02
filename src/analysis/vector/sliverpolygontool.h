/***************************************************************************
 *  sliverpolygontool.h                                                    *
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

#ifndef VECTORANALYSIS_SLIVERPOLYGON_TOOL_H
#define VECTORANALYSIS_SLIVERPOLYGON_TOOL_H

#include "abstracttool.h"
#include <QMutex>

typedef struct GEOSGeom_t GEOSGeometry;

namespace Geoprocessing
{

  class ANALYSIS_EXPORT SliverPolygonTool : public AbstractTool
  {
    public:
      enum MergeMethod { MergeLargestArea, MergeLongestEdge };

      SliverPolygonTool( QgsVectorLayer *layer,
                         bool selected,
                         const QString &output,
                         const QString &outputDriverName,
                         MergeMethod mergeMethod,
                         double threshold,
                         double precision );
      QFuture<void> execute( int task );
      int getTaskCount() const { return 2; }

    private:
      typedef QPair<QgsFeatureId, int> GeomIdx;

      struct FeatureInfo
      {
        QgsFeature *feature;
        QList<double> areas;
        QList<GEOSGeometry *> boundaries;
      };

      struct MergeJob
      {
        MergeJob( const QgsFeatureId &_id, const QMap< int, QMap<QgsFeatureId, QSet<int> > > &_mergeIdx )
          : id( _id ), mergeIdx( _mergeIdx ) {}
        QgsFeatureId id;
        QMap< int, QMap<QgsFeatureId, QSet<int> > > mergeIdx;
      };

      struct MergeFeaturesWrapper
      {
        SliverPolygonTool *instance;
        MergeFeaturesWrapper( SliverPolygonTool *_instance ) : instance( _instance ) {}
        void operator()( const MergeJob &job ) { instance->mergeFeatures( job ); }
      };

      QgsVectorLayer *mLayer;
      bool mSelected;
      MergeMethod mMergeMethod;
      double mThreshold;
      QgsSpatialIndex mSpatialIndex;

      class MergeManager
      {
        public:
          typedef QMap< QgsFeatureId, QMap<int, QMap<QgsFeatureId, QSet<int> > > > MergeMap;

          void insert( const GeomIdx &idx );
          void merge( const GeomIdx &src, GeomIdx &dest );
          const MergeMap &get() const { return mMergeMap; }

        private:
          QMutex mLock;
          QMap<GeomIdx, GeomIdx> mTargetMap;
          MergeMap mMergeMap;
      };
      MergeManager mMergeManager;

      QList<MergeJob> mMergeJobs;

      QMutex mSpatialIndexMutex;

      void prepare();
      void processFeature( const Job *job );
      void mergeFeatures( const MergeJob &mergeJob );
  };

} // Geoprocessing

#endif // VECTORANALYSIS_SLIVERPOLYGON_TOOL_H
