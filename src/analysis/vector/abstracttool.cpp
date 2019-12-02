/***************************************************************************
 *  abstracttool.h                                                      *
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

#include <QApplication>
#include <QtConcurrentRun>
#include <QtConcurrentMap>
#include "abstracttool.h"
#include "qgsfield.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgswkbtypes.h"
#include "qgsgeos.h"

namespace Vectoranalysis
{

  QString AbstractTool::errFeatureDoesNotExist = QApplication::translate( "AbstractTool", "The requested feature does not exist" );
  QString AbstractTool::errFailedToFetchGeometry = QApplication::translate( "AbstractTool", "The feature geometry could not be fetched" );

  void AbstractTool::ProcessFeatureWrapper::operator()( const Job *job )
  {
    try
    {
      instance->processFeature( job );
    }
    catch ( const std::exception &e )
    {
      instance->mExceptions.append( e.what() );
      throw e;
    }
  }

  AbstractTool::AbstractTool( double precision )
  {
    mPrecision = precision;
  }

  AbstractTool::~AbstractTool()
  {
    qDeleteAll( mJobQueue );
  }

  QFuture<void> AbstractTool::init()
  {
    return QtConcurrent::run( this, &AbstractTool::prepare );
  }

  QFuture<void> AbstractTool::execute( int /*task*/ )
  {
    return QtConcurrent::map( mJobQueue, ProcessFeatureWrapper( this ) );
  }

  void AbstractTool::buildSpatialIndex( QgsSpatialIndex &index, QgsVectorLayer *layer, bool selectedOnly ) const
  {
    QgsFeature currentFeature;
    if ( selectedOnly )
    {
      for ( const QgsFeatureId &id : layer->selectedFeatureIds() )
      {
        QgsFeatureRequest request( id );
        request.setFlags( QgsFeatureRequest::SubsetOfAttributes );
        if ( layer->getFeatures( request ).nextFeature( currentFeature ) )
        {
          index.addFeature( currentFeature );
        }
      }
    }
    else
    {
      QgsFeatureRequest request;
      request.setFlags( QgsFeatureRequest::SubsetOfAttributes );
      QgsFeatureIterator it = layer->getFeatures();
      while ( it.nextFeature( currentFeature ) )
      {
        index.addFeature( currentFeature );
      }
    }
  }

  void AbstractTool::appendToJobQueue( QgsVectorLayer *layer, bool selectedOnly, int taskFlag )
  {
    if ( selectedOnly )
    {
      const QgsFeatureIds selection = layer->selectedFeatureIds();
      QgsFeatureIds::const_iterator it = selection.constBegin();
      for ( ; it != selection.constEnd(); ++it )
      {
        mJobQueue.append( new Job( *it, taskFlag ) );
      }
    }
    else
    {
      for ( const QgsFeatureId &id : layer->allFeatureIds() )
      {
        mJobQueue.append( new Job( id, taskFlag ) );
      }
    }
  }

  void AbstractTool::createOutputFileWriter( const QString &fileName, const QgsVectorLayer *layerA, const QgsVectorLayer *layerB, OutputFields outputFields, OutputCrs outputCrs, const QString &outputDriverName )
  {
    mOutWkbType = layerA->wkbType();

    QgsFields fields;
    switch ( outputFields )
    {
      case FieldsA:
        fields = layerA->fields();
        break;
      case FieldsB:
        fields = layerB->fields();
        break;
      case FieldsAandB:
        fields = layerA->fields();

        QList<QString> names;
        for ( const QgsField &field : fields.toList() )
        {
          names.append( field.name() );
        }

        for ( const QgsField &field : layerB->fields().toList() )
        {
          QString name = field.name();
          for ( int count = 0; names.contains( name ); ++count )
          {
            name = QString( "%1_%2" ).arg( field.name() ).arg( count );
          }
          fields.append( QgsField( name, field.type() ) );
        }
        break;
    }

    QgsCoordinateReferenceSystem crs;
    switch ( outputCrs )
    {
      case CrsLayerA:
        crs = layerA->crs();
        break;
      case CrsLayerB:
        crs = layerB->crs();
        break;
    }

    mNumOutFields = fields.size();
    mOutputWriter = new QgsVectorFileWriter( fileName, layerA->dataProvider()->encoding(), fields, layerA->wkbType(), crs, outputDriverName );
  }

  bool AbstractTool::getFeatureAtId( QgsFeature &feature, QgsFeatureId id, QgsVectorLayer *layer )
  {
    QgsFeatureRequest request( id );
    if ( !layer->getFeatures( request ).nextFeature( feature ) )
    {
      reportInvalidFeatureError( layer, id, errFeatureDoesNotExist );
      return false;
    }
    else if ( !feature.hasGeometry() )
    {
      reportInvalidFeatureError( layer, id, errFailedToFetchGeometry );
      return false;
    }
    return true;
  }

  QVector<QgsFeature *> AbstractTool::getIntersects( const QgsRectangle &rect, QgsSpatialIndex &index, QgsVectorLayer *layer )
  {
    mIntersectMutex.lock();
    QList<QgsFeatureId> intersectIds = index.intersects( rect );
    mIntersectMutex.unlock();

    QVector<QgsFeature *> featureList;
    for ( QgsFeatureId id : intersectIds )
    {
      QgsFeature *feature = new QgsFeature();
      if ( !getFeatureAtId( *feature, id, layer ) )
      {
        delete feature;
        continue;
      }
      featureList.append( feature );
    }
    return featureList;
  }

  QgsAttributes AbstractTool::combineAttributes( const QgsAttributes *attribsA, const QgsAttributes *attribsB, OutputFields outputFields ) const
  {
    switch ( outputFields )
    {
      case FieldsA:
        if ( attribsA )
        {
          return *attribsA;
        }
        else
        {
          QgsAttributes result;
          for ( int i = 0; i < mNumOutFields; ++i )
          {
            result.insert( i, 0 );
          }
          return result;
        }
      case FieldsB:
        if ( attribsB )
        {
          return *attribsB;
        }
        else
        {
          QgsAttributes result;
          for ( int i = 0; i < mNumOutFields; ++i )
          {
            result.insert( i, 0 );
          }
          return result;
        }
      case FieldsAandB:
        QgsAttributes result;
        if ( !attribsA && !attribsB )
        {
          for ( int i = 0; i < mNumOutFields; ++i )
          {
            result.insert( i, 0 );
          }
        }
        else
        {
          int mid = attribsA ? attribsA->size() : mNumOutFields - attribsB->size();
          if ( !attribsA )
          {
            for ( int i = 0; i < mid; ++i )
            {
              result.insert( i, 0 );
            }
          }
          else
          {
            result = *attribsA;
          }
          if ( !attribsB )
          {
            for ( int i = mid; i < mNumOutFields; ++i )
            {
              result.insert( i, 0 );
            }
          }
          else
          {
            int i = mid;
            for ( const QVariant &value : *attribsB )
            {
              result.insert( i++, value );
            }
          }
        }
        return result;
    }
    return QgsAttributes();
  }

  void AbstractTool::writeFeatures( const QVector<QgsFeature *> &outFeatures )
  {
    QMutexLocker locker( &mWriteMutex );
    for ( QgsFeature *feature : outFeatures )
    {
      feature->geometry().convertGeometryCollectionToSubclass( QgsWkbTypes::geometryType( mOutWkbType ) );
      // Skip incompatible geometries
      if ( QgsWkbTypes::singleType( feature->geometry().wkbType() ) != QgsWkbTypes::singleType( mOutWkbType ) )
      {
        QgsDebugMsg( QString( "Skipping incompatible geometry: %1 %2" ).arg( feature->geometry().wkbType() ).arg( mOutWkbType ) );
        continue;
      }
      // If output type is a singleType, create features for each single geometry
      else if ( mOutWkbType == QgsWkbTypes::singleType( mOutWkbType ) )
      {
        for ( QgsGeometry geometry : feature->geometry().asGeometryCollection() )
        {
          QgsFeature f;
          f.setGeometry( geometry );
          f.setAttributes( feature->attributes() );
          if ( !mOutputWriter->addFeature( f ) )
          {
            mWriteErrors.append( mOutputWriter->errorMessage() );
          }
        }
      }
      else
      {
        if ( !mOutputWriter->addFeature( *feature ) )
        {
          mWriteErrors.append( mOutputWriter->errorMessage() );
        }
      }
    }
  }

} // Geoprocessing
