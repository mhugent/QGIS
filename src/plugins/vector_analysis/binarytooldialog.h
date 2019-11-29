/***************************************************************************
 *  binarytooldialog.h                                                     *
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

#ifndef VECTORANALYSIS_BINARYTOOLDIALOG_H
#define VECTORANALYSIS_BINARYTOOLDIALOG_H

#include "geoprocessingtooldialog.h"
#include "abstracttool.h"
#include "vectoranalysisutils.h"

#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>

namespace Geoprocessing
{

  class AbstractBinaryToolDialog : public GeoprocessingToolDialog
  {
      Q_OBJECT

    public:
      AbstractBinaryToolDialog( QgisInterface *iface, const QString &title, int outputFields, int outputCRS );

    protected:
      QCheckBox *mCheckboxOperatorLayerSelected;
      QComboBox *mComboOperatorLayer;
      QComboBox *mComboOutputFields;
      QComboBox *mComboOutputCRS;

      bool getInputValid() const;

    private slots:
      virtual void updateLayers();
      void setOperatorLayer();
      void updateOperatorSelectionCheckBox();

    private:
      QgsVectorLayer *mOperatorLayer;
  };

  template < class T, int outputFields = -1, int outputCRS = -1 >
  class BinaryToolDialog : public AbstractBinaryToolDialog
  {
    public:
      BinaryToolDialog( QgisInterface *iface, const QString &title )
        : AbstractBinaryToolDialog( iface, title, outputFields, outputCRS ) {}

    private:
      AbstractTool *setupTool()
      {
        QgsVectorLayer *layerA = Utils::getSelectedLayer( ui.comboBox_inputLayer );
        QgsVectorLayer *layerB = Utils::getSelectedLayer( mComboOperatorLayer );
        AbstractTool::OutputFields fields = static_cast<AbstractTool::OutputFields>( mComboOutputFields->currentIndex() );
        AbstractTool::OutputCrs crs = static_cast<AbstractTool::OutputCrs>( mComboOutputCRS->currentIndex() );

        QString errorMsgs;
        if ( !T::validateInputs( layerA, layerB, errorMsgs ) )
        {
          QMessageBox::warning( this, QApplication::translate( "GenericGeoprocessingToolDialog", "Invalid input" ), errorMsgs );
          return 0;
        }

        return new T(
                 layerA, layerB,
                 ui.checkBox_inputLayerSelected->isChecked(),
                 mCheckboxOperatorLayerSelected->isChecked(),
                 ui.lineEdit_outputFileName->text(), getOutputDriverName(),
                 fields, crs, getPrecision() );
      }
  };

} // Geoprocessing

# endif // VECTORANALYSIS_BINARYTOOLDIALOG_H
