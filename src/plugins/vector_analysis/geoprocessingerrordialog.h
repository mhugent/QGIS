/***************************************************************************
 *  geoprocessingerrordialog.h                                             *
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

#ifndef GEOPROCESSING_ERRORDIALOG_H
#define GEOPROCESSING_ERRORDIALOG_H

#include <QDialog>
#include "ui_geoprocessingerrordialog.h"
#include "qgisinterface.h"
#include "abstracttool.h"

class QgsRubberBand;

namespace Geoprocessing
{

  class GeoprocessingErrorDialog : public QDialog
  {
      Q_OBJECT

    public:
      GeoprocessingErrorDialog( QgisInterface* iface, AbstractTool *tool );
      ~GeoprocessingErrorDialog();

    private:
      Ui::GeoprocessingErrorDialog ui;
      QgisInterface* mIface;
      AbstractTool *mTool;
      QVector<QgsRubberBand*> mRubberBands;

      static int sFeatureErrorRole;
      static int sGeometryErrorRole;

    private slots:
      void populateErrorList( const QString& section, const QList<AbstractTool::Error>& errors );
      void highlightFeature( QTreeWidgetItem* item, int column );
      void selectFaulty();
  };

} // Geoprocessing

#endif // GEOPROCESSING_ERRORDIALOG_H
