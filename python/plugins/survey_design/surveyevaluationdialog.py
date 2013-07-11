from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from ui_surveyevaluationdialogbase import Ui_SurveyEvaluationDialogBase

class SurveyEvaluationDialog( QDialog,  Ui_SurveyEvaluationDialogBase ):

    def __init__(self,  iface,  parent ):
        QDialog.__init__(self,  parent)
        self.setupUi( self )
        self.mIface = iface

        #insert available sample layers / ids into combo box
        layers = QgsMapLayerRegistry.instance().mapLayers()
        for key in layers:
            layer = layers[key]
            if layer.type() == QgsMapLayer.VectorLayer:
                self.mSampleLayerComboBox.addItem( layer.name(),  key )
                self.mBaselineLayerClipComboBox.addItem( layer.name(),  key )

        self.sampleLayerChanged( self.mSampleLayerComboBox.currentIndex() )
        QObject.connect( self.mSampleLayerComboBox, SIGNAL('currentIndexChanged( int )'), self.sampleLayerChanged )

#output
    def speciesVulnerability(self):
        return self.mSpeciesVulnerabilitySpinBox.value()

    def sampleLayerId(self):
        return self.mSampleLayerComboBox.itemData( self.mSampleLayerComboBox.currentIndex() ).toString()

    def baselineClipLayerId(self):
        return self.mBaselineLayerClipComboBox.itemData( self.mBaselineLayerClipComboBox.currentIndex() ).toString()

    def stratumId(self):
        return self.mStratumIdComboBox.itemText( self.mStratumIdComboBox.currentIndex() )

    def arealAvailability(self):
        return self.mArealAvailabilityComboBox.itemText( self.mArealAvailabilityComboBox.currentIndex() )

    def catch(self):
        return self.mCatchComboBox.itemText( self. mCatchComboBox.currentIndex() )

    def dist(self):
        return self.mDistComboBox.itemText( self.mDistComboBox.currentIndex() )

    def width(self):
        return self.mWidthComboBox.itemText( self.mWidthComboBox.currentIndex() )

    def verticalAvailability(self):
        return self.mVavailComboBox.itemText( self.mVavailComboBox.currentIndex() )

    def sampleLayerChanged(self,  index):

        self.mStratumIdComboBox.clear()
        self.mArealAvailabilityComboBox.clear()
        self.mCatchComboBox.clear()
        self.mDistComboBox.clear()
        self.mWidthComboBox.clear()
        self.mVavailComboBox.clear()

        layerKey = self.mSampleLayerComboBox.itemData( index )
        layer = QgsMapLayerRegistry.instance().mapLayer( layerKey )
        if layer is None or layer.type() != QgsMapLayer.VectorLayer:
            return

        fieldMap = layer.pendingFields()
        for key in fieldMap:
            field = fieldMap[key]
            if field.type() == QVariant.String:
                continue
            self.mStratumIdComboBox.addItem( field.name() )
            self.mArealAvailabilityComboBox.addItem( field.name() )
            self.mCatchComboBox.addItem( field.name() )
            self.mDistComboBox.addItem( field.name() )
            self.mWidthComboBox.addItem( field.name() )
            self.mVavailComboBox.addItem( field.name() )

