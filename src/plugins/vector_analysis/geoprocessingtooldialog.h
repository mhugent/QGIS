/***************************************************************************
 *  geoprocessingtooldialog.h                                              *
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

#ifndef VECTORANALYSIS_GEOPROCESSINGTOOLDIALOG_H
#define VECTORANALYSIS_GEOPROCESSINGTOOLDIALOG_H

#include "ui_geoprocessingtooldialog.h"
#include <QDialog>

class QgisInterface;
class QgsVectorLayer;

namespace Geoprocessing
{

  class AbstractTool;

  class GeoprocessingToolDialog : public QDialog
  {
      Q_OBJECT
    public:
      GeoprocessingToolDialog( QgisInterface* iface, const QString& title );

    private:
      int mProgressMin, mProgressMax;
      QString mOutputDriverName;

      void closeEvent( QCloseEvent *ev );
      virtual AbstractTool* setupTool() = 0;
      void showEvent( QShowEvent * ) { emit shown(); }

    signals:
      void shown();

    private slots:
      void runTool();
      void selectOutputFile();
      void accept() {}
      void reject() {}
      void setProgressRange( int min, int max );
      void setProgressValue( int value );
      void setInputLayer();
      void updateInputSelectionCheckBox();

    protected:
      QgisInterface* mIface;
      Ui::GeoprocessingToolDialog ui;
      double mProgressCurTask;
      double mProgressNTasks;
      QgsVectorLayer* mInputLayer;

      virtual bool getInputValid() const;
      const QString& getOutputDriverName() const { return mOutputDriverName; }
      double getPrecision() const;
      void updateSelectionCheckBox( QCheckBox* selectionCheckBox, QComboBox* layerComboBox );

    protected slots:
      virtual void updateLayers();
      void validateInput();

  };

} // Geoprocessing

#endif // VECTORANALYSIS_GEOPROCESSINGTOOLDIALOG_H
