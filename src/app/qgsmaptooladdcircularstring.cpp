/***************************************************************************
    qgsmaptooladdcirularstring.h  -  map tool for adding circular strings
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
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspointv2.h"
#include <QMouseEvent>

QgsMapToolAddCircularString::QgsMapToolAddCircularString( QgsMapToolAddFeature* parentTool, QgsMapCanvas* canvas, CaptureMode mode ): QgsMapToolCapture( canvas, mode ),
    mParentTool( parentTool ), mRubberBand( 0 )
{

}

QgsMapToolAddCircularString::QgsMapToolAddCircularString( QgsMapCanvas* canvas ): QgsMapToolCapture( canvas ), mParentTool( 0 )
{

}

QgsMapToolAddCircularString::~QgsMapToolAddCircularString()
{
  delete mRubberBand;
}

void QgsMapToolAddCircularString::canvasMoveEvent( QMouseEvent * e )
{
  if ( mRubberBand )
  {
    QgsPoint layerPoint;
    QgsPoint mapPoint;
    nextPoint( e->pos(), layerPoint, mapPoint );

    QgsVertexId idx; idx.feature = 0; idx.ring = 0; idx.vertex = mPoints.size();
    mRubberBand->moveVertex( idx, QgsPointV2( layerPoint.x(), layerPoint.y() ) );
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
    if ( mPoints.size() >= 3 )
    {
      QgsCircularStringV2* c = new QgsCircularStringV2();
      int nRubberBandPoints = ( mPoints.size() % 2 == 0 ) ? mPoints.size() - 1 : mPoints.size();
      c->setPoints( mPoints.mid( 0, nRubberBandPoints ) );
      mParentTool->setCurve( c );
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

void QgsMapToolAddCircularString::deactivate()
{
  if ( mParentTool )
  {
    QgsCircularStringV2* c = new QgsCircularStringV2();
    c->setPoints( mPoints );
    mParentTool->setCurve( c );
  }
  mPoints.clear();
  delete mRubberBand; mRubberBand = 0;
}
