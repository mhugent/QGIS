from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from ui_surveyinitdialogbase import Ui_SurveyInitDialogBase

class SurveyInitDialog( QDialog,  Ui_SurveyInitDialogBase ):
    
    def __init__(self):
        QDialog.__init__(self, None)
        self.setupUi(self)
        
        #possible entries in layer combo boxes
        self.mSurveyAreaLayerComboBox.addItem( QCoreApplication.translate( 'SurveyInitDialog', 'None' ), '' )
        
        mapLayers = QgsMapLayerRegistry.instance().mapLayers()
        for id in mapLayers:
            currentLayer = mapLayers[id]
            if currentLayer.type() != QgsMapLayer.VectorLayer:
                continue
            
            if currentLayer.geometryType() == QGis.Polygon:
                self.mSurveyAreaLayerComboBox.addItem( currentLayer.name(), currentLayer.id() )
                self.mStrataLayerComboBox.addItem( currentLayer.name(), currentLayer.id() )
            elif currentLayer.geometryType() == QGis.Line:
                self.mSurveyBaselineLayerComboBox.addItem( currentLayer.name(), currentLayer.id() )
        
        #set layers from project file
        surveyAreaLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyAreaLayer','' )
        if not surveyAreaLayer[0].isEmpty():
            self.mSurveyAreaLayerComboBox.setCurrentIndex( self.mSurveyAreaLayerComboBox.findData( surveyAreaLayer[0] ) )
            
        surveyBaselineLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyBaselineLayer', '' )
        self.mSurveyBaselineLayerComboBox.setCurrentIndex( self.mSurveyBaselineLayerComboBox.findData( surveyBaselineLayer[0] ) )
        self.setBaselineStrataIdAttributes( self.mSurveyBaselineLayerComboBox.currentIndex() )
        
        strataLayer = QgsProject.instance().readEntry( 'Survey', 'StrataLayer', '' )
        self.mStrataLayerComboBox.setCurrentIndex( self.mStrataLayerComboBox.findData( strataLayer[0] ) )
        self.setMinimumDistanceAttributes( self.mStrataLayerComboBox.currentIndex() )
        self.setNSamplePointsAttributes( self.mStrataLayerComboBox.currentIndex() )
        self.setStrataIdAttributes ( self.mStrataLayerComboBox.currentIndex() )
        
        #update attribute combo boxes if layer change
        QObject.connect( self.mSurveyBaselineLayerComboBox, SIGNAL('currentIndexChanged( int )'), self.setBaselineStrataIdAttributes )
        QObject.connect( self.mStrataLayerComboBox, SIGNAL('currentIndexChanged( int )'), self.setMinimumDistanceAttributes )
        QObject.connect( self.mStrataLayerComboBox, SIGNAL('currentIndexChanged( int )'), self.setNSamplePointsAttributes )
        QObject.connect( self.mStrataLayerComboBox, SIGNAL('currentIndexChanged( int )'), self.setStrataIdAttributes )
        
        self.mStratumIdComboBox.setCurrentIndex(  self.mStratumIdComboBox.findData( QgsProject.instance().readNumEntry( 'Survey', 'BaselineStrataId', 0 )[0]  ) )
        self.mMinimumDistanceAttributeComboBox.setCurrentIndex( self.mMinimumDistanceAttributeComboBox.findData( QgsProject.instance().readNumEntry( 'Survey', 'StrataMinDistance', 0 )[0]  )  )
        self.mNSamplePointsComboBox.setCurrentIndex( self.mNSamplePointsComboBox.findData( QgsProject.instance().readNumEntry( 'Survey', 'StrataNSamplePoints', 0 )[0]  )  )
        self.mStrataIdAttributeComboBox.setCurrentIndex( self.mStrataIdAttributeComboBox.findData( QgsProject.instance().readNumEntry( 'Survey', 'StrataId',  0 ) [0] )  )
        
        
    #Return id of survey area layer (or empty string if none)
    def surveyAreaLayer(self):
        return self.mSurveyAreaLayerComboBox.itemData( self.mSurveyAreaLayerComboBox.currentIndex() ).toString()
        
    #Return id of baseline layer
    def surveyBaselineLayer(self):
        return self.mSurveyBaselineLayerComboBox.itemData( self.mSurveyBaselineLayerComboBox.currentIndex() ).toString()
     
    #Return attribut index of stratum id in baseline layer (-1 if none)
    def baselineStrataId( self ):
        return self.mStratumIdComboBox.itemData( self.mStratumIdComboBox.currentIndex() ).toInt()[0]
        
    #Return id of stratum layer
    def strataLayer( self ):
        return self.mStrataLayerComboBox.itemData( self.mStrataLayerComboBox.currentIndex() ).toString()
        
    #Return index of strata layer attribute containing the minimum distance between sample points
    def strataMinDistanceAttribute( self ):
        return self.mMinimumDistanceAttributeComboBox.itemData( self.mMinimumDistanceAttributeComboBox.currentIndex() ).toString()
        
    #Return index of strata layer attribut containing the number of sample points
    def strataNSamplePointsAttribute( self ):
        return self.mNSamplePointsComboBox.itemData(  self.mNSamplePointsComboBox.currentIndex() ).toInt()[0]
        
    #Return index of strata layer containing the feature id
    def strataId( self ):
        return self.mStrataIdAttributeComboBox.itemData( self.mStrataIdAttributeComboBox.currentIndex() ).toInt()[0]
        
    #Fill attributes into mStratumIdComboBox
    def setBaselineStrataIdAttributes( self, index ):
        self.mStratumIdComboBox.clear()
        self.mStratumIdComboBox.addItem( QCoreApplication.translate( 'SurveyInitDialog', 'None' ), -1 )
        layerId = self.mStrataLayerComboBox.itemData( index ).toString()
        if not layerId.isEmpty():
            layer = QgsMapLayerRegistry.instance().mapLayer( layerId )
            if not layer is None:
                if not layer.type() == QgsMapLayer.VectorLayer:
                    return
                
                fieldMap = layer.pendingFields()
                for key in fieldMap:
                    field = fieldMap[key]
                    self.mStratumIdComboBox.addItem( field.name(), key )
    
    def setMinimumDistanceAttributes( self, index ):
        self.mMinimumDistanceAttributeComboBox.clear()
        layerId = self.mStrataLayerComboBox.itemData( index ).toString()
        if not layerId.isEmpty():
            layer = QgsMapLayerRegistry.instance().mapLayer( layerId )
            if not layer is None:
                if not layer.type() == QgsMapLayer.VectorLayer:
                    return
                    
                fieldMap = layer.pendingFields()
                for key in fieldMap:
                    field = fieldMap[key]
                    self.mMinimumDistanceAttributeComboBox.addItem( field.name(), key )
                    
    def setNSamplePointsAttributes( self, index ):
        self.mNSamplePointsComboBox.clear()
        layerId = self.mStrataLayerComboBox.itemData( index ).toString()
        if not layerId.isEmpty():
            layer = QgsMapLayerRegistry.instance().mapLayer( layerId )
            if not layer is None:
                if not layer.type() == QgsMapLayer.VectorLayer:
                    return
                    
                fieldMap = layer.pendingFields()
                for key in fieldMap:
                    field = fieldMap[key]
                    self.mNSamplePointsComboBox.addItem( field.name(), key )
                    
    def setStrataIdAttributes( self, index ):
        self.mStrataIdAttributeComboBox.clear()
        layerId = self.mStrataLayerComboBox.itemData( index ).toString()
        if not layerId.isEmpty():
            layer = QgsMapLayerRegistry.instance().mapLayer( layerId )
            if not layer is None:
                if not layer.type() == QgsMapLayer.VectorLayer:
                    return
                    
                fieldMap = layer.pendingFields()
                for key in fieldMap:
                    field = fieldMap[key]
                    self.mStrataIdAttributeComboBox.addItem( field.name(), key )
        
    def accept( self ):
         #store the settings to the properties section of the project file
         QgsProject.instance().writeEntry( 'Survey', 'SurveyAreaLayer', self.surveyAreaLayer() )
         QgsProject.instance().writeEntry( 'Survey', 'SurveyBaselineLayer', self.surveyBaselineLayer() )
         QgsProject.instance().writeEntry( 'Survey', 'StrataLayer', self.strataLayer() )
         QgsProject.instance().writeEntry( 'Survey', 'BaselineStrataId', self.baselineStrataId() )
         QgsProject.instance().writeEntry( 'Survey', 'StrataMinDistance', self.strataMinDistanceAttribute() )
         QgsProject.instance().writeEntry( 'Survey', 'StrataNSamplePoints', self.strataNSamplePointsAttribute() )
         QgsProject.instance().writeEntry( 'Survey', 'StrataId', self.strataId() )
         QDialog.accept( self )
