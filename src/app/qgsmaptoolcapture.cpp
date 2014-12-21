/***************************************************************************
    qgsmaptoolcapture.cpp  -  map tool for capturing points, lines, polygons
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolcapture.h"

#include "qgisapp.h"
#include "qgscompoundcurvev2.h"
#include "qgscursors.h"
#include "qgscurvepolygonv2.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryvalidator.h"
#include "qgslayertreeview.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"

#include <QCursor>
#include <QPixmap>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStatusBar>


QgsMapToolCapture::QgsMapToolCapture( QgsMapCanvas* canvas, enum CaptureMode tool )
    : QgsMapToolEdit( canvas )
    , mCaptureMode( tool )
    , mValidator( 0 )
    , mSnappingMarker( 0 )
    , mGeometry( 0 )
    , mGeometryRubberBand( 0 )
    , mTempGeometryRubberBand( 0 )
    , mCurrentRubberBandVertex( -1 )
{
  mCaptureModeFromLayer = tool == CaptureNone;
  mCapturing = false;

  mGeometry = new QgsCompoundCurveV2();

  QPixmap mySelectQPixmap = QPixmap(( const char ** ) capture_point_cursor );
  mCursor = QCursor( mySelectQPixmap, 8, 8 );

  connect( QgisApp::instance()->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SLOT( currentLayerChanged( QgsMapLayer * ) ) );
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  delete mSnappingMarker;

  stopCapturing();

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = 0;
  }

  delete mGeometry;
  delete mGeometryRubberBand;
  delete mTempGeometryRubberBand;
}

void QgsMapToolCapture::deactivate()
{
  delete mSnappingMarker;
  mSnappingMarker = 0;

  QgsMapToolEdit::deactivate();
}

void QgsMapToolCapture::currentLayerChanged( QgsMapLayer *layer )
{
  if ( !mCaptureModeFromLayer )
    return;

  mCaptureMode = CaptureNone;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
  {
    return;
  }

  switch ( vlayer->geometryType() )
  {
    case QGis::Point:
      mCaptureMode = CapturePoint;
      break;
    case QGis::Line:
      mCaptureMode = CaptureLine;
      break;
    case QGis::Polygon:
      mCaptureMode = CapturePolygon;
      break;
    default:
      mCaptureMode = CaptureNone;
      break;
  }
}

void QgsMapToolCapture::canvasMoveEvent( QMouseEvent * e )
{
  QgsPoint mapPoint;
  QList<QgsSnappingResult> snapResults;
  if ( mSnapper.snapToBackgroundLayers( e->pos(), snapResults ) == 0 )
  {
    if ( snapResults.isEmpty() )
    {
      delete mSnappingMarker;
      mSnappingMarker = 0;
    }
    else
    {
      if ( !mSnappingMarker )
      {
        mSnappingMarker = new QgsVertexMarker( mCanvas );
        mSnappingMarker->setIconType( QgsVertexMarker::ICON_CROSS );
        mSnappingMarker->setColor( Qt::magenta );
        mSnappingMarker->setPenWidth( 3 );
      }
      mSnappingMarker->setCenter( snapResults.constBegin()->snappedVertex );
    }

    if ( mCaptureMode != CapturePoint && mGeometryRubberBand && mCapturing && mCurrentRubberBandVertex >= 0 )
    {
      mapPoint = snapPointFromResults( snapResults, e->pos() );
      QgsVertexId vId; vId.feature = 0; vId.ring = 0; vId.vertex = mCurrentRubberBandVertex;
      mGeometryRubberBand->moveVertex( vId, QgsPointV2( mapPoint.x(), mapPoint.y() ) );
    }
  }
} // mouseMoveEvent


void QgsMapToolCapture::canvasPressEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  // nothing to be done
}


int QgsMapToolCapture::nextPoint( const QPoint &p, QgsPoint &layerPoint, QgsPoint &mapPoint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
  {
    QgsDebugMsg( "no vector layer" );
    return 1;
  }

  QgsPoint digitisedPoint;
  try
  {
    digitisedPoint = toLayerCoordinates( vlayer, p );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( "transformation to layer coordinate failed" );
    return 2;
  }

  QList<QgsSnappingResult> snapResults;
  if ( mSnapper.snapToBackgroundLayers( p, snapResults ) == 0 )
  {
    mapPoint = snapPointFromResults( snapResults, p );
    try
    {
      layerPoint = toLayerCoordinates( vlayer, mapPoint ); //transform snapped point back to layer crs
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      QgsDebugMsg( "transformation to layer coordinate failed" );
      return 2;
    }
  }

  return 0;
}

void QgsMapToolCapture::setCurve( QgsCurveV2* c )
{
  if ( !mGeometry )
  {
    return;
  }

  int nCurves = mGeometry->nCurves();
  if ( nCurves > 0 && mGeometry->curveAt( nCurves - 1 )->geometryType() == "CircularString" )
  {
    mGeometry->removeCurve( nCurves - 1 );
  }

  mGeometry->addCurve( c );
  setGeometryToRubberBand();
}

int QgsMapToolCapture::addVertex( const QPoint &p )
{
  QgsPoint layerPoint;
  QgsPoint mapPoint;
  int res = nextPoint( p, layerPoint, mapPoint );
  if ( res != 0 )
  {
    QgsDebugMsg( "nextPoint failed: " + QString::number( res ) );
    return res;
  }

  if ( !mGeometry )
  {
    return 2;
  }
  mGeometry->addVertex( QgsPointV2( layerPoint.x(), layerPoint.y() ) );

  setGeometryToRubberBand();
  validateGeometry();
  return 0;
}

void QgsMapToolCapture::setGeometryToRubberBand()
{
  if ( !mGeometryRubberBand )
  {
    mGeometryRubberBand = createGeometryRubberBand(( mCaptureMode == CapturePolygon ) ? QGis::Polygon : QGis::Line );
  }
  if ( !mTempGeometryRubberBand )
  {
    mTempGeometryRubberBand = createGeometryRubberBand(( mCaptureMode == CapturePolygon ) ? QGis::Polygon : QGis::Line );
  }

  QgsCompoundCurveV2* rubberBandGeom = dynamic_cast<QgsCompoundCurveV2*>( mGeometry->clone() );
  rubberBandGeom->addVertex( rubberBandGeom->endPoint() );
  mCurrentRubberBandVertex = rubberBandGeom->numPoints() - 1;
  if ( mCaptureMode == CapturePolygon )
  {
    rubberBandGeom->close();
  }
  mGeometryRubberBand->setGeometry( rubberBandGeom );
}


void QgsMapToolCapture::undo()
{
#if 0
  if ( mRubberBand )
  {
    int rubberBandSize = mRubberBand->numberOfVertices();
    int tempRubberBandSize = mTempRubberBand->numberOfVertices();
    int captureListSize = mCaptureList.size();

    if ( rubberBandSize < 1 || captureListSize < 1 )
    {
      return;
    }

    mRubberBand->removePoint( -1 );

    if ( rubberBandSize > 1 )
    {
      if ( tempRubberBandSize > 1 )
      {
        const QgsPoint *point = mRubberBand->getPoint( 0, rubberBandSize - 2 );
        mTempRubberBand->movePoint( tempRubberBandSize - 2, *point );
      }
    }
    else
    {
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? true : false );
    }

    mCaptureList.removeLast();

    validateGeometry();
  }
#endif //0
}

void QgsMapToolCapture::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
  {
    undo();

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}

void QgsMapToolCapture::startCapturing()
{
  mCapturing = true;
}

void QgsMapToolCapture::stopCapturing()
{
  delete mGeometry;
  mGeometry = new QgsCompoundCurveV2();

  delete mGeometryRubberBand; mGeometryRubberBand = 0;

  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  mGeomErrors.clear();

#ifdef Q_OS_WIN
  // hope your wearing your peril sensitive sunglasses.
  QgisApp::instance()->skipNextContextMenuEvent();
#endif

  mCapturing = false;
  mCaptureList.clear();
  mCanvas->refresh();
}

void QgsMapToolCapture::closePolygon()
{
  mCaptureList.append( mCaptureList[0] );
}

void QgsMapToolCapture::validateGeometry()
{
  QSettings settings;
  if ( settings.value( "/qgis/digitizing/validate_geometries", 1 ).toInt() == 0 )
    return;

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = 0;
  }

  mTip = "";
  mGeomErrors.clear();
  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  QgsGeometry *g = 0;

  switch ( mCaptureMode )
  {
    case CaptureNone:
    case CapturePoint:
      return;
    case CaptureLine:
      if ( mCaptureList.size() < 2 )
        return;
      g = QgsGeometry::fromPolyline( mCaptureList.toVector() );
      break;
    case CapturePolygon:
      if ( mCaptureList.size() < 3 )
        return;
      g = QgsGeometry::fromPolygon( QgsPolygon() << ( QgsPolyline() << mCaptureList.toVector() << mCaptureList[0] ) );
      break;
  }

  if ( !g )
    return;

  mValidator = new QgsGeometryValidator( g );
  connect( mValidator, SIGNAL( errorFound( QgsGeometry::Error ) ), this, SLOT( addError( QgsGeometry::Error ) ) );
  connect( mValidator, SIGNAL( finished() ), this, SLOT( validationFinished() ) );
  mValidator->start();

  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation started." ) );
}

void QgsMapToolCapture::addError( QgsGeometry::Error e )
{
  mGeomErrors << e;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
    return;

  if ( !mTip.isEmpty() )
    mTip += "\n";

  mTip += e.what();

  if ( e.hasWhere() )
  {
    QgsVertexMarker *vm =  new QgsVertexMarker( mCanvas );
    vm->setCenter( mCanvas->mapSettings().layerToMapCoordinates( vlayer, e.where() ) );
    vm->setIconType( QgsVertexMarker::ICON_X );
    vm->setPenWidth( 2 );
    vm->setToolTip( e.what() );
    vm->setColor( Qt::green );
    vm->setZValue( vm->zValue() + 1 );
    mGeomErrorMarkers << vm;
  }

  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( e.what() );
  if ( !mTip.isEmpty() )
    sb->setToolTip( mTip );
}

void QgsMapToolCapture::validationFinished()
{
  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation finished." ) );
}
