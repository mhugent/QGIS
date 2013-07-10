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
        return self.mStratumIdComboBox.itemData( self.mStratumIdComboBox.currentIndex() ).toInt()[0]

    def arealAvailability(self):
        index = self.mArealAvailabilityComboBox.currentIndex()
        if index == -1:
            return -1

        return self.mArealAvailabilityComboBox.itemData( index ).toInt()[0]

    def catch(self):
        index = self.mCatchComboBox.currentIndex()
        if index == -1:
            return -1

        return self.mCatchComboBox.itemData( index ).toInt()[0]

    def dist(self):
        index = self.mDistComboBox.currentIndex()
        if index == -1:
            return -1

        return self.mDistComboBox.itemData( index ).toInt()[0]

    def width(self):
        index = self.mWidthComboBox.currentIndex()
        if index == -1:
            return -1

        return self.mWidthComboBox.itemData( index ).toInt()[0]

    def verticalAvailability(self):
        index = self.mVavailComboBox.currentIndex()
        if index == -1:
            return -1

        return self.mVavailComboBox.itemData( index ).toInt()[0]

    def sampleLayerChanged(self,  index):

        self.mStratumIdComboBox.clear()
        self.mArealAvailabilityComboBox.clear()
        self.mCatchComboBox.clear()
        self.mDistComboBox.clear()
        self.mWidthComboBox.clear()
        self.mVavailComboBox.clear()

        layerKey = self.mSampleLayerComboBox.itemData( index ).toString()
        layer = QgsMapLayerRegistry.instance().mapLayer( layerKey )
        if layer is None or layer.type() != QgsMapLayer.VectorLayer:
            return

        fieldMap = layer.pendingFields()
        for key in fieldMap:
            field = fieldMap[key]
            if field.type() == QVariant.String:
                continue
            self.mStratumIdComboBox.addItem( field.name(),  key )
            self.mArealAvailabilityComboBox.addItem( field.name(),  key )
            self.mCatchComboBox.addItem( field.name(),  key )
            self.mDistComboBox.addItem( field.name(),  key )
            self.mWidthComboBox.addItem( field.name(),  key )
            self.mVavailComboBox.addItem( field.name(),  key )

