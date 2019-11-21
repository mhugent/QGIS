/***************************************************************************
 *  vectoranalysis.cpp                                                               *
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

#include "vectoranalysis.h"
#include "plugin.h"

#include "buffertooldialog.h"
#include "convexhulltooldialog.h"

#if 0
#include "geoprocessing/differencetool.h"
#include "geoprocessing/intersectiontool.h"
#include "geoprocessing/symdifferencetool.h"
#include "geoprocessing/uniontool.h"
#include "geoprocessing/ui/binarytooldialog.h"
#include "geoprocessing/ui/buffertooldialog.h"
#include "geoprocessing/ui/convexhulltooldialog.h"
#include "geoprocessing/ui/dissolvetooldialog.h"
#include "geoprocessing/ui/sliverpolygontooldialog.h"
#endif //0

#include "qgisinterface.h"

#include <QAction>
#include <QMenu>
#include <QSettings>

VectorAnalysis::VectorAnalysis( QgisInterface * theQgisInterface ):
    QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface ), mCurrDialog( 0 )
{
}

void VectorAnalysis::initGui()
{
  // Construct menu
  QAction* bufferAction = new QAction( getPluginIcon( "buffer.png" ), tr( "Buffer" ), this );
  QAction* clipAction = new QAction( getPluginIcon( "clip.png" ), tr( "Clip" ), this );
  QAction* convexhullAction = new QAction( getPluginIcon( "convex_hull.png" ), tr( "Convex Hull" ), this );
  QAction* differenceAction = new QAction( getPluginIcon( "difference.png" ), tr( "Difference" ), this );
  QAction* dissolveAction = new QAction( getPluginIcon( "dissolve.png" ), tr( "Dissolve" ), this );
  QAction* intersectAction = new QAction( getPluginIcon( "intersect.png" ), tr( "Intersect" ), this );
  QAction* sliverPolygonsAction = new QAction( getPluginIcon( "sliverpolygon.png" ), tr( "Eliminate sliver polygons" ), this );
  QAction* symDifferenceAction = new QAction( getPluginIcon( "sym_difference.png" ), tr( "Symmetric difference" ), this );
  QAction* unionAction = new QAction( getPluginIcon( "union.png" ), tr( "Union" ), this );

  mGeoprocessingMenu = new QMenu( tr( "&Geoprocessing Tools" ), mQGisIface->vectorMenu() );
  mGeoprocessingMenu->setIcon( QIcon( ":/vectoranalysis/icons/geoprocessing.png" ) );
  mGeoprocessingMenu->addAction( convexhullAction );
  mGeoprocessingMenu->addAction( bufferAction );
  mGeoprocessingMenu->addAction( intersectAction );
  mGeoprocessingMenu->addAction( unionAction );
  mGeoprocessingMenu->addAction( symDifferenceAction );
  mGeoprocessingMenu->addAction( clipAction );
  mGeoprocessingMenu->addAction( differenceAction );
  mGeoprocessingMenu->addAction( dissolveAction );
  mGeoprocessingMenu->addAction( sliverPolygonsAction );
  mQGisIface->vectorMenu()->addMenu( mGeoprocessingMenu );

  connect( bufferAction, SIGNAL( triggered() ), this, SLOT( runBufferTool() ) );
  connect( clipAction, SIGNAL( triggered() ), this, SLOT( runClipTool() ) );
  connect( convexhullAction, SIGNAL( triggered() ), this, SLOT( runConvexhullTool() ) );
  connect( differenceAction, SIGNAL( triggered() ), this, SLOT( runDifferenceTool() ) );
  connect( dissolveAction, SIGNAL( triggered() ), this, SLOT( runDissolveTool() ) );
  connect( intersectAction, SIGNAL( triggered() ), this, SLOT( runIntersectionTool() ) );
  connect( sliverPolygonsAction, SIGNAL( triggered() ), this, SLOT( runSliverPolygonTool() ) );
  connect( symDifferenceAction, SIGNAL( triggered() ), this, SLOT( runSymDifferenceTool() ) );
  connect( unionAction, SIGNAL( triggered() ), this, SLOT( runUnionTool() ) );
}

void VectorAnalysis::unload()
{
  mQGisIface->vectorMenu()->removeAction( mGeoprocessingMenu->menuAction() );
  delete mGeoprocessingMenu;
  delete mCurrDialog;
  mCurrDialog = 0;
}

void VectorAnalysis::runBufferTool()
{
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::BufferToolDialog( mQGisIface );
  mCurrDialog->show();
}

void VectorAnalysis::runClipTool()
{
#if 0
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::BinaryToolDialog<Geoprocessing::IntersectionTool, Geoprocessing::AbstractTool::FieldsA>( mQGisIface, tr( "Clip" ) );
  mCurrDialog->show();
#endif //0
}

void VectorAnalysis::runConvexhullTool()
{
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::ConvexHullToolDialog( mQGisIface );
  mCurrDialog->show();
}

void VectorAnalysis::runDifferenceTool()
{
#if 0
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::BinaryToolDialog<Geoprocessing::DifferenceTool, Geoprocessing::AbstractTool::FieldsA>( mQGisIface, tr( "Difference" ) );
  mCurrDialog->show();
#endif //0
}

void VectorAnalysis::runDissolveTool()
{
#if 0
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::DissolveToolDialog( mQGisIface );
  mCurrDialog->show();
#endif //0
}

void VectorAnalysis::runIntersectionTool()
{
#if 0
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::BinaryToolDialog<Geoprocessing::IntersectionTool>( mQGisIface, tr( "Intersection" ) );
  mCurrDialog->show();
#endif //0
}

void VectorAnalysis::runSliverPolygonTool()
{
#if 0
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::SliverPolygonToolDialog( mQGisIface );
  mCurrDialog->show();
#endif //0
}

void VectorAnalysis::runSymDifferenceTool()
{
#if 0
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::BinaryToolDialog<Geoprocessing::SymDifferenceTool>( mQGisIface, tr( "Symmetric difference" ) );
  mCurrDialog->show();
#endif //0
}

void VectorAnalysis::runUnionTool()
{
#if 0
  delete mCurrDialog;
  mCurrDialog = new Geoprocessing::BinaryToolDialog<Geoprocessing::UnionTool>( mQGisIface, tr( "Union" ) );
  mCurrDialog->show();
#endif //0
}

QIcon VectorAnalysis::getPluginIcon( const QString &name )
{
  return QIcon( QString( ":/vectoranalysis/icons/%1" ).arg( name ) );
}
