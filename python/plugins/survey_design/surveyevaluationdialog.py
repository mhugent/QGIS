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
            self.mSampleLayerComboBox.addItem( layer.name(),  key )
        
        self.sampleLayerChanged( self.mSampleLayerComboBox.currentIndex() )
        QObject.connect( self.mSampleLayerComboBox, SIGNAL('currentIndexChanged( int )'), self.sampleLayerChanged )
        
    def speciesVulnerability(self):
        return self.mSpeciesVulnerabilitySpinBox.value()
        
    def sampleLayerChanged(self,  index):
        
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
            self.mArealAvailabilityComboBox.addItem( field.name(),  key )
            self.mCatchComboBox.addItem( field.name(),  key )
            self.mDistComboBox.addItem( field.name(),  key )
            self.mWidthComboBox.addItem( field.name(),  key )
            self.mVavailComboBox.addItem( field.name(),  key )
            
