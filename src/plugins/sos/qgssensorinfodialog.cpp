/***************************************************************************
                          qgssensorinfodialog.cpp  -  description
                          ---------------------------------------
    begin                : June 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorinfodialog.h"
#include "qgswatermldata.h"
#include <QCheckBox>
#include <QCursor>
#include <QDateTimeEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QToolTip>
#include <qwt_picker_machine.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_draw.h>
#include <qwt_symbol.h>

class TimeScaleDraw: public QwtScaleDraw
{
  public:
    TimeScaleDraw() {}
    virtual QwtText label( double v ) const
    {
      if ( v >= 0 )
      {
        return QDateTime::fromTime_t( v ).toString();
      }
      else
      {
        return QDateTime::fromTime_t( 0 ).addSecs( v ).toString();
      }
    }
};

QgsSensorInfoDialog::QgsSensorInfoDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  QPushButton* displayButton = new QPushButton( tr( "Display" ), this );
  connect( displayButton, SIGNAL( clicked() ), this, SLOT( showDiagram() ) );
  mButtonBox->addButton( displayButton, QDialogButtonBox::ActionRole );

  mTabWidget->setTabsClosable( true );
}

QgsSensorInfoDialog::~QgsSensorInfoDialog()
{
}

void QgsSensorInfoDialog::clearObservables()
{
  mObservableTreeWidget->clear();
}

void QgsSensorInfoDialog::addObservables( const QString& serviceUrl, const QString stationId, const QStringList& observables, const QList< QDateTime >& begin,
    const QList< QDateTime >& end )
{
  QTreeWidgetItem* stationIdWidget = new QTreeWidgetItem( QStringList() << stationId );
  stationIdWidget->setData( 0, Qt::UserRole, serviceUrl );
  stationIdWidget->setFlags( Qt::ItemIsEnabled );
  mObservableTreeWidget->addTopLevelItem( stationIdWidget );

  QStringList::const_iterator obsIt = observables.constBegin();
  QList< QDateTime >::const_iterator bIt = begin.constBegin();
  QList< QDateTime >::const_iterator eIt = end.constBegin();
  for ( ; obsIt != observables.constEnd() && bIt != begin.constEnd() && eIt != end.constEnd(); ++obsIt )
  {

    QTreeWidgetItem* observableItem = new QTreeWidgetItem( stationIdWidget, QStringList() << "" << *obsIt );
    QCheckBox* cb = new QCheckBox();
    mObservableTreeWidget->setItemWidget( observableItem, 2, cb );
    //begin
    QDateTime beginDateTime = bIt->isValid() ? *bIt : QDateTime( QDate( 1900, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC );
    mObservableTreeWidget->setItemWidget( observableItem, 3, new QDateTimeEdit( beginDateTime ) );
    //end
    QDateTime endDateTime;
    if ( eIt->isValid() )
    {
      endDateTime = *eIt;
    }
    else
    {
      endDateTime =  QDateTime::currentDateTime().toTimeSpec( Qt::UTC );
      endDateTime.setTime( QTime( 0, 0, 0 ) );
    }
    mObservableTreeWidget->setItemWidget( observableItem, 4, new QDateTimeEdit( endDateTime ) );
    if ( bIt->isValid() && eIt->isValid() )
    {
      cb->setCheckState( Qt::Checked );
    }
    mObservableTreeWidget->setCurrentItem( observableItem );
  }
  mObservableTreeWidget->expandAll();
}

void QgsSensorInfoDialog::showDiagram()
{
  QList<QTreeWidgetItem*> selectedItems = mObservableTreeWidget->selectedItems();
  if ( selectedItems.size() < 1 )
  {
    return;
  }

  QTreeWidgetItem* item  = selectedItems.at( 0 );
  QTreeWidgetItem* parent = item->parent();
  if ( !parent )
  {
    return;
  }

  QString featureOfInterest = parent->text( 0 );
  QString serviceUrl = parent->data( 0, Qt::UserRole ).toString();
  QString observedProperty = item->text( 1 );

  bool useTemporalFilter = qobject_cast<QCheckBox*>( mObservableTreeWidget->itemWidget( item, 2 ) )->isChecked();
  QString temporalFilter;

  if ( useTemporalFilter )
  {
    QDateTimeEdit* beginEdit = qobject_cast<QDateTimeEdit*>( mObservableTreeWidget->itemWidget( item, 3 ) );
    QDateTimeEdit* endEdit = qobject_cast<QDateTimeEdit*>( mObservableTreeWidget->itemWidget( item, 4 ) );
    if ( !beginEdit || ! endEdit )
    {
      return;
    }

    temporalFilter = "om:phenomenonTime," + beginEdit->dateTime().toString( Qt::ISODate )
                     + "/" + endEdit->dateTime().toString( Qt::ISODate );
  }

  QUrl url( serviceUrl );
  url.removeQueryItem( "request" );
  url.addQueryItem( "request", "GetObservation" );
  url.addQueryItem( "featureOfInterest", featureOfInterest );
  url.addQueryItem( "observedProperty", observedProperty );
  if ( useTemporalFilter )
  {
    url.addQueryItem( "temporalFilter", temporalFilter );
  }

  QgsWaterMLData data( url.toString().toLocal8Bit().data() );
  QMap< QDateTime, double > timeValueMap;
  data.getData( &timeValueMap );

  //cancel diagram if result contains no data
  if ( timeValueMap.size() < 1 )
  {
    QMessageBox::information( this, tr( "No data" ), tr( "The GetObservation request returned no observation" ) );
    return;
  }

  QVector<double> timeVector( timeValueMap.size() );
  QVector<double> valueVector( timeValueMap.size() );
  int i = 0;
  QMap< QDateTime, double >::const_iterator it = timeValueMap.constBegin();
  for ( ; it != timeValueMap.constEnd(); ++it )
  {
    timeVector[i] = convertTimeToInt( it.key() );
    valueVector[i] = it.value();
    ++i;
  }

  //create QWtPlot
  QwtPlot* diagram = new QwtPlot( featureOfInterest, this );
  diagram->setAxisScaleDraw( QwtPlot::xBottom, new TimeScaleDraw() );
  QwtPlotCurve* curve = new QwtPlotCurve( observedProperty );
  curve->setPen( QPen( Qt::red ) );
  curve->setData( timeVector.constData(), valueVector.constData(), timeValueMap.size() );
  QwtSymbol symbol( QwtSymbol::Rect, QBrush( Qt::NoBrush ), QPen( Qt::red ), QSize( 5, 5 ) );
  curve->setSymbol( symbol );
  curve->attach( diagram );

  //plot picker to show tooltips
  QwtPlotPicker* plotPicker = new QwtPlotPicker( diagram->canvas() );
  plotPicker->setSelectionFlags( QwtPicker::PointSelection | QwtPicker::ClickSelection );
  plotPicker->setRubberBand( QwtPlotPicker::RectRubberBand );
  connect( plotPicker, SIGNAL( selected( const QwtDoublePoint& ) ), this, SLOT( onDiagramSelected( const QwtDoublePoint& ) ) );

  mTabWidget->addTab( diagram, featureOfInterest );
  diagram->replot();
  raise();
}

int QgsSensorInfoDialog::convertTimeToInt( const QDateTime& dt ) const
{
  int i = dt.toTime_t();
  if ( i < 0 )
  {
    i = - dt.secsTo( QDateTime::fromTime_t( 0 ) );
  }
  return i;
}

QDateTime QgsSensorInfoDialog::convertIntToTime( int t ) const
{
  if ( t > 0 )
  {
    return QDateTime::fromTime_t( t );
  }
  else
  {
    return QDateTime::fromTime_t( 0 ).addSecs( t );
  }
}

void QgsSensorInfoDialog::onDiagramSelected( const QwtDoublePoint &pt )
{
  QwtPlot* cPlot = currentPlot();

  //snap to next point

  if ( cPlot )
  {
    QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>( cPlot->itemList()[0] );
    QwtPlotItemList debugList =  cPlot->itemList();

    int xPixel = cPlot->transform( QwtPlot::xBottom, pt.x() );
    int yPixel = cPlot->transform( QwtPlot::yLeft , pt.y() );

    int snapPoint = curve->closestPoint( QPoint( xPixel, yPixel ) );
    if ( snapPoint >= 0 )
    {
      QString snappedTime = convertIntToTime( curve->data().x( snapPoint ) ).toString();
      double snappedValue = curve->data().y( snapPoint );

      qWarning( "Snapped value" );
      qWarning( snappedTime.toLocal8Bit().data() );
      qWarning( QString::number( snappedValue ).toLocal8Bit().data() );

      QwtPlotMarker* mark = plotMarker( cPlot );
      mark->setValue( curve->data().x( snapPoint ), curve->data().y( snapPoint ) );
      mark->show();

      //marker label

      QwtText  labelText;
      labelText.setText( "Value: " + QString::number( snappedValue ) + "\n" + snappedTime );
      labelText.setColor( QColor( Qt::black ) );
      QBrush labelBrush( QColor( Qt::white ), Qt::SolidPattern );
      labelText.setBackgroundBrush( labelBrush );
      mark->setLabel( labelText );
      mark->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
      cPlot->replot();
    }
  }
}

QwtPlot* QgsSensorInfoDialog::currentPlot()
{
  return qobject_cast<QwtPlot*>( mTabWidget->currentWidget() );
}

QwtPlotMarker* QgsSensorInfoDialog::plotMarker( QwtPlot* plot )
{
  if ( !plot )
  {
    return 0;
  }

  QMap< QwtPlot*, QwtPlotMarker* >::const_iterator plotIt = mPlotMarkers.find( plot );
  if ( plotIt != mPlotMarkers.constEnd() )
  {
    return plotIt.value();
  }

  QwtPlotMarker* marker = new QwtPlotMarker();
  marker->setXAxis( QwtPlot::xBottom );
  marker->setYAxis( QwtPlot::yLeft );
  marker->setSymbol( QwtSymbol( QwtSymbol::Rect, QBrush( Qt::blue ), QPen( Qt::green ), QSize( 5, 5 ) ) );
  marker->attach( plot );
  mPlotMarkers.insert( plot, marker );
  return marker;
}

void QgsSensorInfoDialog::on_mTabWidget_tabCloseRequested( int index )
{
  if ( index < 1 )
  {
    return;
  }
  mTabWidget->removeTab( index );
}
