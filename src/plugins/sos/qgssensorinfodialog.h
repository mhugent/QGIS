/***************************************************************************
                          qgssensorinfodialog.h  -  description
                          -------------------------------------
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

#ifndef QGSSENSORINFODIALOG_H
#define QGSSENSORINFODIALOG_H

#include "ui_qgssensorinfodialogbase.h"
#include <qwt_double_rect.h>
#include <QDateTime>

class QwtPlot;
class QwtPlotMarker;

class QgsSensorInfoDialog: public QDialog, private Ui::QgsSensorInfoDialogBase
{
    Q_OBJECT
  public:
    QgsSensorInfoDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsSensorInfoDialog();

    void clearObservables();
    void addObservables( const QString& serviceUrl, const QString stationId, const QStringList& observables, const QList< QDateTime >& begin, const QList< QDateTime >& end );

  private slots:
    void showDiagram();
    int convertTimeToInt( const QDateTime& dt ) const;
    QDateTime convertIntToTime( int t ) const;
    void onDiagramSelected( const QwtDoublePoint &pt );
    void on_mTabWidget_tabCloseRequested( int index );

  private:
    QwtPlot* currentPlot();
    QwtPlotMarker* plotMarker( QwtPlot* plot );

    QMap< QwtPlot*, QwtPlotMarker* > mPlotMarkers;
};

#endif // QGSSENSORINFODIALOG_H
