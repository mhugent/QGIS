#include "qgsmaptoolcircularstringthreepoints.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgsgeometryrubberband.h"
#include "qgspointv2.h"
#include <QMouseEvent>

QgsMapToolCircularStringThreePoints::QgsMapToolCircularStringThreePoints( QgsMapToolCapture* parentTool,
    QgsMapCanvas* canvas, CaptureMode mode ): QgsMapToolAddCircularString( parentTool, canvas, mode )
{

}

QgsMapToolCircularStringThreePoints::~QgsMapToolCircularStringThreePoints()
{
}

void QgsMapToolCircularStringThreePoints::canvasReleaseEvent( QMouseEvent * e )
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

void QgsMapToolCircularStringThreePoints::canvasMoveEvent( QMouseEvent * e )
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
