/***************************************************************************
    qgsmaptooladdcircularstring.h  -  map tool for adding circular strings
    ---------------------
    begin                : December 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooladdcircularstring.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgscurvepolygonv2.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestringv2.h"
#include "qgsmapcanvas.h"
#include "qgspointv2.h"
#include <QMouseEvent>

QgsMapToolAddCircularString::QgsMapToolAddCircularString( QgsMapToolCapture* parentTool, QgsMapCanvas* canvas, CaptureMode mode ): QgsMapToolCapture( canvas, mode ),
    mParentTool( parentTool ), mRubberBand( 0 ), mShowCenterPointRubberBand( false ), mCenterPointRubberBand( 0 )
{
  if ( mCanvas )
  {
    connect( mCanvas, SIGNAL( mapToolSet( QgsMapTool*, QgsMapTool* ) ), this, SLOT( setParentTool( QgsMapTool*, QgsMapTool* ) ) );
  }
}

QgsMapToolAddCircularString::QgsMapToolAddCircularString( QgsMapCanvas* canvas ): QgsMapToolCapture( canvas ), mParentTool( 0 ),
    mRubberBand( 0 ), mShowCenterPointRubberBand( false ), mCenterPointRubberBand( 0 )
{
  if ( mCanvas )
  {
    connect( mCanvas, SIGNAL( mapToolSet( QgsMapTool*, QgsMapTool* ) ), this, SLOT( setParentTool( QgsMapTool*, QgsMapTool* ) ) );
  }
}

QgsMapToolAddCircularString::~QgsMapToolAddCircularString()
{
  delete mRubberBand;
  removeCenterPointRubberBand();
}

void QgsMapToolAddCircularString::setParentTool( QgsMapTool* newTool, QgsMapTool* oldTool )
{
  QgsMapToolCapture* tool = dynamic_cast<QgsMapToolCapture*>( oldTool );
  QgsMapToolAddCircularString* csTool = dynamic_cast<QgsMapToolAddCircularString*>( oldTool );
  if ( csTool && newTool == this )
  {
    mParentTool = csTool->mParentTool;
  }
  else if ( tool && newTool == this )
  {
    mParentTool = tool;
  }
}

void QgsMapToolAddCircularString::canvasMoveEvent( QMouseEvent * e )
{
  if ( mRubberBand )
  {
    QgsPoint layerPoint;
    QgsPoint mapPoint;
    nextPoint( e->pos(), layerPoint, mapPoint );

    QgsVertexId idx; idx.part = 0; idx.ring = 0; idx.vertex = mPoints.size();
    mRubberBand->moveVertex( idx, QgsPointV2( layerPoint.x(), layerPoint.y() ) );
    updateCenterPointRubberBand( QgsPointV2( layerPoint.x(), layerPoint.y() ) );
  }
}

void QgsMapToolAddCircularString::canvasReleaseEvent( QMouseEvent* e )
{
  QgsPoint layerPoint;
  QgsPoint mapPoint;
  nextPoint( e->pos(), layerPoint, mapPoint );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 1 ) //connection to vertex of previous line segment needed?
    {
      const QgsCompoundCurveV2* compoundCurve = mParentTool->geometry();
      if ( compoundCurve )
      {
        if ( compoundCurve->nCurves() > 0 )
        {
          const QgsCurveV2* curve = compoundCurve->curveAt( compoundCurve->nCurves() - 1 );
          if ( curve )
          {
            mPoints.append( curve->endPoint() );
          }
        }
      }
    }
    mPoints.append( QgsPointV2( layerPoint.x(), layerPoint.y() ) );
    if ( !mCenterPointRubberBand && mShowCenterPointRubberBand )
    {
      createCenterPointRubberBand();
    }

    if ( mPoints.size() > 1 )
    {
      if ( !mRubberBand )
      {
        mRubberBand = createGeometryRubberBand(( mCaptureMode == CapturePolygon ) ? QGis::Polygon : QGis::Line );
        mRubberBand->show();
      }

      QgsCircularStringV2* c = new QgsCircularStringV2();
      QList< QgsPointV2 > rubberBandPoints = mPoints;
      rubberBandPoints.append( QgsPointV2( layerPoint.x(), layerPoint.y() ) );
      c->setPoints( rubberBandPoints );
      mRubberBand->setGeometry( c );
    }
    if (( mPoints.size() ) % 2 == 1 )
    {
      removeCenterPointRubberBand();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolAddCircularString::keyPressEvent( QKeyEvent* e )
{
  if ( e->isAutoRepeat() )
  {
    return;
  }

  if ( e && e->key() == Qt::Key_R )
  {
    mShowCenterPointRubberBand = true;

    createCenterPointRubberBand();
  }
}

void QgsMapToolAddCircularString::keyReleaseEvent( QKeyEvent* e )
{
  if ( e->isAutoRepeat() )
  {
    return;
  }

  if ( e && e->key() == Qt::Key_R )
  {
    removeCenterPointRubberBand();
    mShowCenterPointRubberBand = false;
  }
}

void QgsMapToolAddCircularString::deactivate()
{
  if ( mParentTool && mPoints.size() > 0 )
  {
    QgsCircularStringV2* c = new QgsCircularStringV2();
    c->setPoints( mPoints );
    mParentTool->setCurve( c );
  }
  mPoints.clear();
  delete mRubberBand; mRubberBand = 0;
  removeCenterPointRubberBand();
}

void QgsMapToolAddCircularString::createCenterPointRubberBand()
{
  if ( !mShowCenterPointRubberBand || mPoints.size() < 2 || mPoints.size() % 2 != 0 )
  {
    return;
  }

  mCenterPointRubberBand = createGeometryRubberBand( QGis::Polygon );
  mCenterPointRubberBand->show();

  if ( mRubberBand )
  {
    const QgsAbstractGeometryV2* rubberBandGeom = mRubberBand->geometry();
    if ( rubberBandGeom )
    {
      QgsVertexId idx; idx.part = 0; idx.ring = 0; idx.vertex = mPoints.size();
      QgsPointV2 pt = rubberBandGeom->pointAt( idx );
      updateCenterPointRubberBand( pt );
    }
  }
}

void QgsMapToolAddCircularString::updateCenterPointRubberBand( const QgsPointV2& pt )
{
  if ( !mShowCenterPointRubberBand || !mCenterPointRubberBand || mPoints.size() < 2 )
  {
    return;
  }

  if (( mPoints.size() ) % 2 != 0 )
  {
    return;
  }

  //create circular string
  QgsCircularStringV2* cs = new QgsCircularStringV2();
  QList< QgsPointV2 > csPoints;
  csPoints.append( mPoints.at( mPoints.size() - 2 ) );
  csPoints.append( mPoints.at( mPoints.size() - 1 ) );
  csPoints.append( pt );
  cs->setPoints( csPoints );

  double centerX, centerY;
  double radius;
  QgsGeometryUtils::circleCenterRadius( csPoints.at( 0 ), csPoints.at( 1 ), csPoints.at( 2 ), radius, centerX, centerY );

  QgsLineStringV2* segment1 = new QgsLineStringV2();
  segment1->addVertex( QgsPointV2( centerX, centerY ) );
  segment1->addVertex( csPoints.at( 0 ) );

  QgsLineStringV2* segment2 = new QgsLineStringV2();
  segment2->addVertex( csPoints.at( 2 ) );
  segment2->addVertex( QgsPointV2( centerX, centerY ) );

  QgsCompoundCurveV2* cc = new QgsCompoundCurveV2();
  cc->addCurve( segment1 );
  cc->addCurve( cs );
  cc->addCurve( segment2 );

  QgsCurvePolygonV2* cp = new QgsCurvePolygonV2();
  cp->setExteriorRing( cc );
  mCenterPointRubberBand->setGeometry( cp );
  mCenterPointRubberBand->show();
}

void QgsMapToolAddCircularString::removeCenterPointRubberBand()
{
  delete mCenterPointRubberBand; mCenterPointRubberBand = 0;
}
