/***************************************************************************
 *  geoprocessingerrordialog.cpp                                           *
 *  -------------------                                                    *
 *  begin                : Jun 30, 2014                                    *
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

#include "geoprocessingerrordialog.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsgeometryvalidator.h"
#include "qgsvectorlayer.h"
#include <QPushButton>

namespace Vectoranalysis
{

  int GeoprocessingErrorDialog::sFeatureErrorRole = Qt::UserRole + 1;
  int GeoprocessingErrorDialog::sGeometryErrorRole = Qt::UserRole + 2;

  GeoprocessingErrorDialog::GeoprocessingErrorDialog( QgisInterface *iface, AbstractTool *tool )
    : mIface( iface ), mTool( tool )
  {
    ui.setupUi( this );

    QPushButton *selectFaultyButton = ui.buttonBox->addButton( tr( "Select faulty features" ), QDialogButtonBox::ActionRole );

    if ( !tool->getFeatureErrorList().isEmpty() )
    {
      populateErrorList( tr( "Missing/incomplete features" ), tool->getFeatureErrorList() );
    }

    if ( !tool->getGeometryErrorList().isEmpty() )
    {
      populateErrorList( tr( "Geometry errors" ), tool->getGeometryErrorList() );
    }

    if ( !tool->getWriteErrors().isEmpty() )
    {
      QTreeWidgetItem *parent = new QTreeWidgetItem(
        QStringList() << tr( "%1 features could not be written" ).arg( tool->getWriteErrors().size() ) );
      foreach ( const QString &msg, tool->getWriteErrors() )
      {
        QTreeWidgetItem *msgitem = new QTreeWidgetItem( QStringList() << msg );
        parent->addChild( msgitem );
      }

      ui.treeWidget->addTopLevelItem( parent );
    }

    connect( ui.treeWidget, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ), this, SLOT( highlightFeature( QTreeWidgetItem *, int ) ) );
    connect( selectFaultyButton, SIGNAL( clicked() ), this, SLOT( selectFaulty() ) );
  }

  GeoprocessingErrorDialog::~GeoprocessingErrorDialog()
  {
    qDeleteAll( mRubberBands );
  }

  void GeoprocessingErrorDialog::populateErrorList( const QString &section, const QList<AbstractTool::Error> &errors )
  {
    QTreeWidgetItem *parent = new QTreeWidgetItem( QStringList() << section );
    ui.treeWidget->addTopLevelItem( parent );
    for ( int i = 0, n = errors.size(); i < n; ++i )
    {
      const AbstractTool::Error &err = errors[i];
      QString featureStr;
      foreach ( const AbstractTool::ErrorFeature &errFeature, err.features )
      {
        featureStr += QString( "%1(%2) " ).arg( errFeature.first->name() ).arg( errFeature.second );
      }

      QTreeWidgetItem *iditem = new QTreeWidgetItem( QStringList() << featureStr );
      parent->addChild( iditem );
      iditem->setData( 0, sGeometryErrorRole, i );

      QTreeWidgetItem *msgitem = new QTreeWidgetItem( QStringList() << err.errorMsg );
      iditem->addChild( msgitem );
    }
  }

  void GeoprocessingErrorDialog::highlightFeature( QTreeWidgetItem *item, int column )
  {
    qDeleteAll( mRubberBands );
    mRubberBands.clear();

    const AbstractTool::Error *error = 0;
    if ( !item->data( column, sFeatureErrorRole ).isNull() )
    {
      error = &mTool->getFeatureErrorList()[item->data( column, sFeatureErrorRole ).toInt()];
    }
    else if ( !item->data( column, sGeometryErrorRole ).isNull() )
    {
      error = &mTool->getGeometryErrorList()[item->data( column, sGeometryErrorRole ).toInt()];
    }
    if ( !error )
    {
      return;
    }

    QMap<QgsVectorLayer *, QgsFeatureIds> layerMap;
    foreach ( const AbstractTool::ErrorFeature &errorfeature, error->features )
    {
      layerMap[errorfeature.first].insert( errorfeature.second );
    }
    QgsRectangle bbox;
    bool unite = false;
    foreach ( QgsVectorLayer *layer, layerMap.keys() )
    {
      foreach ( const QgsFeatureId &id, layerMap[layer] )
      {
        QgsFeature f;
        QgsFeatureRequest req( id );
        req.setFlags( QgsFeatureRequest::SubsetOfAttributes );
        layer->getFeatures( req ).nextFeature( f );


        QgsRubberBand *r = new QgsRubberBand( mIface->mapCanvas() );
        r->setToGeometry( f.geometry(), layer );
        r->setColor( Qt::yellow );
        r->setWidth( 2 );
        mRubberBands.append( r );

        QVector<QgsGeometry::Error> geometryErrors;
        QgsGeometryValidator::validateGeometry( f.geometry(), geometryErrors );
        for ( const QgsGeometry::Error &error : geometryErrors )
        {
          if ( error.hasWhere() )
          {
            QgsRubberBand *r = new QgsRubberBand( mIface->mapCanvas(), QgsWkbTypes::PointGeometry );
            r->addPoint( mIface->mapCanvas()->mapSettings().layerToMapCoordinates( layer, error.where() ) );
            r->setWidth( 10 );
            r->setColor( Qt::red );
            mRubberBands.append( r );
          }
        }

        if ( unite )
        {
          bbox.combineExtentWith( mIface->mapCanvas()->mapSettings().layerExtentToOutputExtent( layer, f.geometry().boundingBox() ) );
        }
        else
        {
          bbox = mIface->mapCanvas()->mapSettings().layerExtentToOutputExtent( layer, f.geometry().boundingBox() );
          unite = true;
        }
      }
    }
    mIface->mapCanvas()->setExtent( bbox );
    mIface->mapCanvas()->refresh();
  }

  void GeoprocessingErrorDialog::selectFaulty()
  {
    QMap<QgsVectorLayer *, QgsFeatureIds> layerMap;

    foreach ( const AbstractTool::Error &error, mTool->getFeatureErrorList() )
    {
      foreach ( const AbstractTool::ErrorFeature &errorfeature, error.features )
      {
        layerMap[errorfeature.first].insert( errorfeature.second );
      }
    }

    foreach ( const AbstractTool::Error &error, mTool->getGeometryErrorList() )
    {
      foreach ( const AbstractTool::ErrorFeature &errorfeature, error.features )
      {
        layerMap[errorfeature.first].insert( errorfeature.second );
      }
    }

    foreach ( QgsVectorLayer *layer, layerMap.keys() )
    {
      layer->selectByIds( layerMap[layer ] );
    }
    mIface->mapCanvas()->refresh();
  }

} // Geoprocessing
