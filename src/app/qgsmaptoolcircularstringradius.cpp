/***************************************************************************
    qgsmaptoolcircularstringradius.h  -  map tool for adding circular strings
    ---------------------
    begin                : Feb 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolcircularstringradius.h"
#include "qgisapp.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgsgeometryutils.h"
#include "qgsgeometryrubberband.h"
#include "qgspointv2.h"
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <cmath>

QgsMapToolCircularStringRadius::QgsMapToolCircularStringRadius( QgsMapToolCapture* parentTool, QgsMapCanvas* canvas, CaptureMode mode ) :
    QgsMapToolAddCircularString( parentTool, canvas, mode ), mTemporaryEndPointX( 0.0 ), mTemporaryEndPointY( 0.0 ), mRadiusMode( false ), mRadius( 0.0 ), mLeft( true ),
    mRadiusSpinBox( 0 )
{

}

QgsMapToolCircularStringRadius::~QgsMapToolCircularStringRadius()
{

}

void QgsMapToolCircularStringRadius::canvasReleaseEvent( QMouseEvent * e )
{
  QgsPoint layerPoint;
  QgsPoint mapPoint;
  nextPoint( e->pos(), layerPoint, mapPoint );
  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() == 0 )
    {
      //get first point from parent tool if there. Todo: move to upper class
      const QgsCompoundCurveV2* compoundCurve = mParentTool->geometry();
      if ( compoundCurve && compoundCurve->nCurves() > 0 )
      {
        const QgsCurveV2* curve = compoundCurve->curveAt( compoundCurve->nCurves() - 1 );
        if ( curve )
        {
          mPoints.append( curve->endPoint() );
        }
      }
      else
      {
        mPoints.append( QgsPointV2( layerPoint.x(), layerPoint.y() ) );
        return;
      }
    }

    if ( mPoints.size() % 2 == 1 )
    {
      if ( !mRadiusMode )
      {
        delete mRubberBand; mRubberBand = 0;
        mTemporaryEndPointX = layerPoint.x();
        mTemporaryEndPointY = layerPoint.y();
        mRadiusMode = true;

        //initial radius is distance( tempPoint - mPoints.last ) / 2.0
        mRadius = sqrt( QgsGeometryUtils::sqrDistance2D( mPoints.last(), QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ) ) ) / 2.0;
        mRadius += mRadius / 10.0;
        recalculateCircularString();
        createRadiusSpinBox();
      }
      else
      {
        QgsPointV2 result;
        if ( QgsGeometryUtils::segmentMidPoint( mPoints.last(), QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ),
                                                result, mRadius, mLeft ) )
        {
          mPoints.append( result );
          mPoints.append( QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ) );
        }
        mRadiusMode = false;
        deleteRadiusSpinBox();
      }
    }
    else
    {
      //can we get there?
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate();
  }
}

void QgsMapToolCircularStringRadius::canvasMoveEvent( QMouseEvent * e )
{
  if ( mPoints.size() > 0 && mRadiusMode )
  {
    //adjust mLeft depening on mouse position
    QgsPoint layerPoint;
    QgsPoint mapPoint;
    nextPoint( e->pos(), layerPoint, mapPoint );
    bool bk_left = mLeft;
    mLeft = QgsGeometryUtils::leftOfLine( mapPoint.x(), mapPoint.y(), mPoints.last().x(), mPoints.last().y(), mTemporaryEndPointX, mTemporaryEndPointY ) < 0;
    if ( bk_left != mLeft )
    {
      recalculateCircularString();
    }
  }
  else
  {
    QList<QgsSnappingResult> snapResults;
    backgroundSnap( snapResults, e->pos() );
  }
}

void QgsMapToolCircularStringRadius::recalculateCircularString()
{
  //new midpoint on circle segment
  QgsPointV2 midPoint;
  if ( !QgsGeometryUtils::segmentMidPoint( mPoints.last(), QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ),
       midPoint, mRadius, mLeft ) )
  {
    return;
  }

  QList<QgsPointV2> rubberBandPoints = mPoints; rubberBandPoints.append( midPoint ); rubberBandPoints.append( QgsPointV2( mTemporaryEndPointX, mTemporaryEndPointY ) );
  QgsCircularStringV2* cString = new QgsCircularStringV2();
  cString->setPoints( rubberBandPoints );
  delete mRubberBand;
  mRubberBand = createGeometryRubberBand(( mCaptureMode == CapturePolygon ) ? QGis::Polygon : QGis::Line );
  mRubberBand->setGeometry( cString );
  mRubberBand->show();
}

void QgsMapToolCircularStringRadius::createRadiusSpinBox()
{
  deleteRadiusSpinBox();
  mRadiusSpinBox = new QDoubleSpinBox();
  mRadiusSpinBox->setMaximum( 99999999 );
  mRadiusSpinBox->setDecimals( 2 );
  mRadiusSpinBox->setPrefix( tr( "Radius: " ) );
  mRadiusSpinBox->setValue( mRadius );
  QgisApp::instance()->statusBar()->addWidget( mRadiusSpinBox );
  QObject::connect( mRadiusSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( updateRadiusFromSpinBox( double ) ) );
  mRadiusSpinBox->setFocus( Qt::TabFocusReason );
}

void QgsMapToolCircularStringRadius::deleteRadiusSpinBox()
{
  if ( !mRadiusSpinBox )
  {
    return;
  }
  QgisApp::instance()->statusBar()->removeWidget( mRadiusSpinBox );
  delete mRadiusSpinBox; mRadiusSpinBox = 0;
}

void QgsMapToolCircularStringRadius::updateRadiusFromSpinBox( double radius )
{
  mRadius = radius;
  recalculateCircularString();
}
