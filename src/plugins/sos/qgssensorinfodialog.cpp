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
#include <QDateTimeEdit>
#include <QPushButton>
#include <QUrl>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>

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
    mObservableTreeWidget->setItemWidget( observableItem, 2, new QDateTimeEdit( *bIt ) );
    mObservableTreeWidget->setItemWidget( observableItem, 3, new QDateTimeEdit( *eIt ) );
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
  QDateTimeEdit* beginEdit = qobject_cast<QDateTimeEdit*>( mObservableTreeWidget->itemWidget( item, 2 ) );
  QDateTimeEdit* endEdit = qobject_cast<QDateTimeEdit*>( mObservableTreeWidget->itemWidget( item, 3 ) );
  if ( !beginEdit || ! endEdit )
  {
    return;
  }

  QString temporalFilter = "om:phenomenonTime," + beginEdit->dateTime().toString( Qt::ISODate )
                           + "/" + endEdit->dateTime().toString( Qt::ISODate ); //soon...temporalFilter=om:phenomenonTime,1978-01-01T00:00:00Z/1980-01-01T00:00:00Z
  //http://hydro-sos.niwa.co.nz/KiWIS/KiWIS?service=SOS&request=getobservation&datasource=0&version=2.0&featureOfInterest=http://hydro-sos.niwa.co.nz/stations/4&observedProperty=http://hydro-sos.niwa.co.nz/parameters/Stage/Flow%20rating&temporalFilter=om:phenomenonTime,1978-01-01T00:00:00Z/1980-01-01T00:00:00Z

  QUrl url( serviceUrl );
  url.removeQueryItem( "request" );
  url.addQueryItem( "request", "GetObservation" );
  url.addQueryItem( "featureOfInterest", featureOfInterest );
  url.addQueryItem( "observedProperty", observedProperty );
  url.addQueryItem( "temporalFilter", temporalFilter );

  QgsWaterMLData data( url.toString().toLocal8Bit().data() );
  QMap< QDateTime, double > timeValueMap;
  data.getData( &timeValueMap );

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
  curve->attach( diagram );
  mTabWidget->addTab( diagram, featureOfInterest );
  diagram->replot();
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
