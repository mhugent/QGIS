/***************************************************************************
 *  sliverpolygontool.cpp                                                  *
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

#include <QApplication>
#include <QtConcurrentMap>
#include <qmath.h>
#include <geos_c.h>
#include <QTextStream>

#include "sliverpolygontool.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

//#define DEBUG(x) QTextStream(stdout) << x << endl;
#define DEBUG(x)

namespace Vectoranalysis
{

  SliverPolygonTool::SliverPolygonTool( QgsVectorLayer *layer,
                                        bool selected,
                                        const QString &output,
                                        const QString &outputDriverName,
                                        MergeMethod mergeMethod, double threshold,
                                        double precision )
    : AbstractTool( precision )
  {
    mOutWkbType = QgsWkbTypes::MultiPolygon;
    mOutputWriter = new QgsVectorFileWriter( output, layer->dataProvider()->encoding(), layer->fields(), mOutWkbType, layer->crs(), outputDriverName );
    mLayer = layer;
    mSelected = selected;
    mMergeMethod = mergeMethod;
    mThreshold = threshold;
  }

  void SliverPolygonTool::prepare()
  {
    appendToJobQueue( mLayer, mSelected );
    buildSpatialIndex( mSpatialIndex, mLayer, mSelected );
  }

  QFuture<void> SliverPolygonTool::execute( int task )
  {
//    QThreadPool::globalInstance()->setMaxThreadCount(1);
    if ( task == 0 )
    {
      return AbstractTool::execute( 1 );
    }
    else /*if(task == 1)*/
    {
      const MergeManager::MergeMap &mergeMap = mMergeManager.get();
      for ( const QgsFeatureId &id : mergeMap.keys() )
      {
        mMergeJobs.append( MergeJob( id, mergeMap[id] ) );
      }
      return QtConcurrent::map( mMergeJobs, MergeFeaturesWrapper( this ) );
    }
  }

  static inline double perpDot( const QgsPoint &q, const QgsPoint &p1, const QgsPoint &p2 )
  {
    return ( ( p1.x() - q.x() ) * ( p2.y() - q.y() ) ) - ( ( p2.x() - q.x() ) * ( p1.y() - q.y() ) );
  }

  static double sharedBoundaryLenght( const GEOSCoordSequence *coo1, const GEOSCoordSequence *coo2 )
  {
    double len = 0;

    // Test every pair of segments for shared edges
    unsigned int n; GEOSCoordSeq_getSize_r( QgsGeos::getGEOSHandler(), coo1, &n );
    unsigned int nt; GEOSCoordSeq_getSize_r( QgsGeos::getGEOSHandler(), coo2, &nt );
    for ( unsigned int i = 0, j = 1; j < n; i = j++ )
    {
      double x, y;

      GEOSCoordSeq_getX_r( QgsGeos::getGEOSHandler(), coo1, i, &x );
      GEOSCoordSeq_getY_r( QgsGeos::getGEOSHandler(), coo1, i, &y );
      QgsPoint p1( x, y );

      GEOSCoordSeq_getX_r( QgsGeos::getGEOSHandler(), coo1, j, &x );
      GEOSCoordSeq_getY_r( QgsGeos::getGEOSHandler(), coo1, j, &y );
      QgsPoint p2( x, y );

      QgsVector d = QgsVector( p2.x() - p1.x(), p2.y() - p1.y() );
      double lambdap1 = 0.;
      double lambdap2 = d.length();
      d = d / lambdap2;

      for ( unsigned int it = 0, jt = 1; jt < nt; it = jt++ )
      {
        GEOSCoordSeq_getX_r( QgsGeos::getGEOSHandler(), coo2, it, &x );
        GEOSCoordSeq_getY_r( QgsGeos::getGEOSHandler(), coo2, it, &y );
        QgsPoint q1( x, y );

        GEOSCoordSeq_getX_r( QgsGeos::getGEOSHandler(), coo2, jt, &x );
        GEOSCoordSeq_getY_r( QgsGeos::getGEOSHandler(), coo2, jt, &y );
        QgsPoint q2( x, y );

        // Check whether q1 and q2 are on the line p1, p2
        if ( qAbs( perpDot( q1, p1, p2 ) ) < 1E-8 && qAbs( perpDot( q2, p1, p2 ) ) < 1E-8 )
        {
          // Get length common edge
          double lambdaq1 = QgsVector( q1.x() - p1.x(), q1.y() - p1.y() ) * d;
          double lambdaq2 = QgsVector( q2.x() - p1.x(), q2.y() - p1.y() ) * d;
          if ( lambdaq1 > lambdaq2 )
          {
            qSwap( lambdaq1, lambdaq2 );
          }
          double lambda1 = qMax( lambdaq1, lambdap1 );
          double lambda2 = qMin( lambdaq2, lambdap2 );
          len += qMax( 0., lambda2 - lambda1 );
        }
      }
    }
    return len;
  }

  void SliverPolygonTool::MergeManager::insert( const GeomIdx &idx )
  {
    QMutexLocker locker( &mLock );
    if ( !mMergeMap[idx.first].contains( idx.second ) )
    {
      mMergeMap[idx.first][idx.second] = QMap<QgsFeatureId, QSet<int> >();
    }
  }

  void SliverPolygonTool::MergeManager::merge( const GeomIdx &src, GeomIdx &dest )
  {
    QMutexLocker locker( &mLock );
    QMap<GeomIdx, GeomIdx>::Iterator it = mTargetMap.find( dest );
    GeomIdx realDest = it == mTargetMap.end() ? dest : it.value();
    if ( src == realDest )
    {
      // src is already listed to be merged with another feature
      return;
    }
    QMap<QgsFeatureId, QSet<int> > &srcMap = mMergeMap[src.first][src.second];
    QMap<QgsFeatureId, QSet<int> > &destMap = mMergeMap[realDest.first][realDest.second];
    for ( const QgsFeatureId &id : srcMap.keys() )
    {
      destMap[id].unite( srcMap[id] );
    }
    destMap[src.first].insert( src.second );
    mMergeMap[src.first].remove( src.second );
    if ( mMergeMap[src.first].isEmpty() )
    {
      mMergeMap.remove( src.first );
    }
    mTargetMap[src] = realDest;
  }

  void SliverPolygonTool::processFeature( const Job *job )
  {
    // Get currently processed feature
    QgsFeature f;
    if ( !getFeatureAtId( f, job->featureid, mLayer ) )
    {
      return;
    }
    QVector<QgsGeometry> geoms = f.geometry().asGeometryCollection();
    DEBUG( "Feature " << f.id() << " has " << geoms.size() << " geometries" );

    // Prepare struct for neighbors
    QMap<QgsFeatureId, FeatureInfo> neighbors;

    // Iterate over geometry parts
    for ( int i = 0, n = geoms.size(); i < n; ++i )
    {
      GeomIdx subjIdx( f.id(), i );
      mMergeManager.insert( subjIdx );
      DEBUG( "Looking at (" << subjIdx.first << ", " << subjIdx.second << ")" );

      // Check if geometry is a sliver
      double area = geoms[i].area();
      if ( area <= 0. || area >= mThreshold )
      {
        DEBUG( "(" << subjIdx.first << ", " << subjIdx.second << ") is not a sliver" );
        continue;
      }

      QgsGeometry &geom = geoms[i];
      geos::unique_ptr geosGeom = QgsGeos::asGeos( geom, mPrecision );

      // Geometry is a sliver: determine with which neighboring geometry to merge
      GEOSGeometry *subjBoundary = GEOSBoundary_r( QgsGeos::getGEOSHandler(), geosGeom.get() );

      mSpatialIndexMutex.lock();
      QList<QgsFeatureId> candidates = mSpatialIndex.intersects( geom.boundingBox() );
      mSpatialIndexMutex.unlock();

      double maxVal = 0;
      GeomIdx maxIdx( 0, 0 );
      for ( QgsFeatureId candId : candidates )
      {
        // Get feature and boundaries of candidate
        FeatureInfo *candInfo = 0;
        QMap<QgsFeatureId, FeatureInfo>::iterator candIt = neighbors.find( candId );
        if ( candIt != neighbors.end() )
        {
          candInfo = &candIt.value();
        }
        else
        {
          FeatureInfo info;
          info.feature = new QgsFeature;
          if ( !getFeatureAtId( *info.feature, candId, mLayer ) )
          {
            delete info.feature;
            continue;
          }
          for ( const QgsGeometry &candGeom : info.feature->geometry().asGeometryCollection() )
          {
            info.areas.append( candGeom.area() );
            geos::unique_ptr candGeosGeom = QgsGeos::asGeos( candGeom, mPrecision );
            info.boundaries.append( GEOSBoundary_r( QgsGeos::getGEOSHandler(), candGeosGeom.get() ) );

          }
          candInfo = &neighbors.insert( candId, info ).value();
        }

        // Iterate over geometries of candidate, get geometry which which to merge
        for ( int j = 0, m = candInfo->boundaries.size(); j < m; ++j )
        {
          GeomIdx candIdx( candId, j );
          if ( candIdx == subjIdx )
          {
            continue;
          }
          double len = sharedBoundaryLenght( GEOSGeom_getCoordSeq_r( QgsGeos::getGEOSHandler(), GEOSGetGeometryN_r( QgsGeos::getGEOSHandler(), subjBoundary, 0 ) ), GEOSGeom_getCoordSeq_r( QgsGeos::getGEOSHandler(), GEOSGetGeometryN_r( QgsGeos::getGEOSHandler(), candInfo->boundaries[j], 0 ) ) );
          if ( len > mPrecision )
          {
            double val = mMergeMethod == MergeLargestArea ? candInfo->areas[j] : len;
            if ( val > maxVal )
            {
              maxVal = val;
              maxIdx = candIdx;
            }
          }
        }
      }
      // Insert into merge struct if merge target found
      if ( maxVal > 0. )
      {
        DEBUG( "Merging (" << subjIdx.first << ", " << subjIdx.second << ") into (" << maxIdx.first << ", " << maxIdx.second << ")" );
        mMergeManager.merge( subjIdx, maxIdx );
      }
    }

    // Cleanup
    for ( const FeatureInfo &finfo : neighbors )
    {
      delete finfo.feature;
      for ( GEOSGeometry *boundary : finfo.boundaries )
      {
        GEOSGeom_destroy_r( QgsGeos::getGEOSHandler(), boundary );
      }
    }
  }

  void SliverPolygonTool::mergeFeatures( const MergeJob &mergeJob )
  {
    // Get feature and geometries which which slivers will be merged
    QgsFeature f;
    if ( !getFeatureAtId( f, mergeJob.id, mLayer ) )
    {
      return;
    }
    QVector<QgsGeometry> geoms = f.geometry().asGeometryCollection();
    QList<QgsGeometry> outGeoms;
    DEBUG( "Processing feature " << f.id() << " with " << geoms.size() << " geometries" );

    // For each geometry part of the feature, gather which slivers to merge which the geometry part
    for ( int i : mergeJob.mergeIdx.keys() )
    {
      QVector<QgsGeometry> unionGeometries;
      const QMap<QgsFeatureId, QSet<int> > &mergeGroup = mergeJob.mergeIdx[i];

      QList<ErrorFeature> errorFeatures;
      for ( QgsFeatureId mergeId : mergeGroup.keys() )
      {
        QgsFeature mergeFeature;
        if ( !getFeatureAtId( mergeFeature, mergeId, mLayer ) )
        {
          errorFeatures.append( ErrorFeature( mLayer, mergeId ) );
          continue;
        }
        QVector<QgsGeometry> mergeFeatureGeometries = mergeFeature.geometry().asGeometryCollection();
        for ( int j : mergeGroup[mergeId] )
        {
          DEBUG( "Adding (" << mergeFeature.id() << ", " << j << ") [" << GEOSGeomType_r( QgsGeometry::getGEOSHandler(), mergeFeatureGeometries[j]->asGeos() ) << "] to group" );
          unionGeometries.append( mergeFeatureGeometries[j] );
          mergeFeatureGeometries[j] = QgsGeometry();
        }
      }

      // If there are geometries to merge, do so
      if ( !unionGeometries.isEmpty() )
      {
        unionGeometries.append( geoms[i] );
        DEBUG( "Merging " << unionGeometries.size() << " geometries with (" << f.id() << ", " << i << ")" );
        geoms[i] = QgsGeometry::unaryUnion( unionGeometries );
        geoms[i].convertGeometryCollectionToSubclass( QgsWkbTypes::PolygonGeometry );
        if ( geoms[i].isNull() )
        {
          reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayer, f.id() ) << errorFeatures, QApplication::translate( "sliverpolygontool", "Merge combine" ) );
        }
      }
      outGeoms.append( geoms[i] );
      geoms[i] = QgsGeometry();
    }

    // If all geometries of the feature were slivers and merged with other geometries, no geometries remain...
    if ( outGeoms.isEmpty() )
    {
      return;
    }

    // Create output feature
    QgsFeature outputFeature;
    outputFeature.setAttributes( f.attributes() );

    // Set output feature geometry; if necessary, convert back to multipolygon
    if ( outGeoms.size() == 1 )
    {
      if ( outGeoms[0].isEmpty() )
      {
        return;
      }
      DEBUG( "outGeoms has 1 geometry" );
      outputFeature.setGeometry( outGeoms[0] );
    }
    else
    {
      DEBUG( "outGeoms has " << outGeoms.size() << " geometries" );
      QVector<QgsGeometry> polygons;
      for ( const QgsGeometry &geom : outGeoms )
      {
        //DEBUG( "Adding " << GEOSGeomType_r( QgsGeometry::getGEOSHandler(), geom->asGeos() ) << " to output polygons (GEOSGetNumGeometries = " << GEOSGetNumGeometries_r( QgsGeometry::getGEOSHandler(), geom->asGeos() ) << ")" );
        polygons.append( geom.asGeometryCollection() );
      }

      // Can apparently happen that a multi polygon contains zero parts...
      if ( polygons.isEmpty() )
      {
        return;
      }

      DEBUG( "Output feature has " << polygons.size() << " geometries" );
      QgsGeometry outGeom = QgsGeometry::collectGeometry( polygons );
      if ( outGeom.isNull() )
      {
        reportGeometryError( QList<ErrorFeature>() << ErrorFeature( mLayer, f.id() ), QApplication::translate( "sliverpolygontool", "Failed to create multipolygon from merge results: %1" ).arg( outGeom.lastError() ) );
        return;
      }
      outputFeature.setGeometry( outGeom );
    }

    // Write output feature
    writeFeatures( QVector<QgsFeature *>() << &outputFeature );
  }

} // Geoprocessing
