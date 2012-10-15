from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from ui_surveyinitdialogbase import Ui_SurveyInitDialogBase

class SurveyInitDialog( QDialog,  Ui_SurveyInitDialogBase ):
    
    def __init__(self,  iface):
        QDialog.__init__(self, None)
        self.iface = iface
        self.setupUi(self)
        
        #possible entries in layer combo boxes
        self.fillLayerComboBox( self.mSurveyAreaLayerComboBox,  QGis.Polygon,  True )
        self.fillLayerComboBox( self.mSurveyBaselineLayerComboBox, QGis.Line,  True )
        self.fillLayerComboBox( self.mStrataLayerComboBox,  QGis.Polygon,  False )
        
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
        QObject.connect( self.mStratumIdToolButton, SIGNAL('currentIndexChanged( int )'), self.createBaselineStrataAttribute )
        
        #connections for buttons
        QObject.connect( self.mNewStrataLayerButton, SIGNAL('clicked()'), self.createNewStrataLayer )
        QObject.connect( self.mNewSurveyLayerButton, SIGNAL('clicked()'), self.createNewSurveyAreaLayer )
        QObject.connect( self.mNewBaselineLayerButton, SIGNAL('clicked()'), self.createNewBaselineLayer )
        QObject.connect( self.mStrataMinDistToolButton, SIGNAL('clicked()'), self.createStrataMinDistAttribute )
        QObject.connect( self.mStrataNSamplePointsToolButton, SIGNAL('clicked()') , self.createStrataNSamplePointsAttribute )
        QObject.connect(self.mStrataIdAttributeToolButton, SIGNAL('clicked()'), self.createStrataIdAttribute )
        QObject.connect(self.mStratumIdToolButton, SIGNAL('clicked()'), self.createBaselineStrataAttribute )
        
        self.mStratumIdComboBox.setCurrentIndex(  self.mStratumIdComboBox.findData( QgsProject.instance().readNumEntry( 'Survey', 'BaselineStrataId', 0 )[0]  ) )
        self.mMinimumDistanceAttributeComboBox.setCurrentIndex( self.mMinimumDistanceAttributeComboBox.findData( QgsProject.instance().readNumEntry( 'Survey', 'StrataMinDistance', 0 )[0]  )  )
        self.mNSamplePointsComboBox.setCurrentIndex( self.mNSamplePointsComboBox.findData( QgsProject.instance().readNumEntry( 'Survey', 'StrataNSamplePoints', 0 )[0]  )  )
        self.mStrataIdAttributeComboBox.setCurrentIndex( self.mStrataIdAttributeComboBox.findData( QgsProject.instance().readNumEntry( 'Survey', 'StrataId',  0 ) [0] )  )
     
    def fillLayerComboBox( self,  comboBox,  geometryType,  noneEntry ):
        comboBox.clear()
        if noneEntry == True:
            comboBox.addItem( self.tr( 'None' ) )
            
        mapLayers = QgsMapLayerRegistry.instance().mapLayers()
        for id in mapLayers:
            currentLayer = mapLayers[id]
            if currentLayer.type() != QgsMapLayer.VectorLayer:
                continue
            
            if currentLayer.geometryType() == geometryType:
                comboBox.addItem( currentLayer.name(), currentLayer.id() )
        
        
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
        return self.mMinimumDistanceAttributeComboBox.itemData( self.mMinimumDistanceAttributeComboBox.currentIndex() ).toInt()[0]
        
    #Return index of strata layer attribut containing the number of sample points
    def strataNSamplePointsAttribute( self ):
        return self.mNSamplePointsComboBox.itemData(  self.mNSamplePointsComboBox.currentIndex() ).toInt()[0]
        
    #Return index of strata layer containing the feature id
    def strataId( self ):
        return self.mStrataIdAttributeComboBox.itemData( self.mStrataIdAttributeComboBox.currentIndex() ).toInt()[0]
        
    #Fill attributes into mStratumIdComboBox
    def setBaselineStrataIdAttributes( self, index ):
        self.mStratumIdComboBox.clear()
        self.mStratumIdComboBox.addItem( self.tr( 'None' ), -1 )
        layerId = self.mSurveyBaselineLayerComboBox.itemData( index ).toString()
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
        
        self.mMinimumDistanceAttributeComboBox.addItem( self.tr('None'), -1 )
        
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
                    
    def createNewStrataLayer( self ):
        attributeList = []
        #samples per polygon
        nPointsAttribute = QgsNewVectorLayerDialog.AttributeEntry()
        nPointsAttribute.name = 'nPoints'
        nPointsAttribute.type = 'Integer'
        nPointsAttribute.width = 10
        nPointsAttribute.precision = 0
        attributeList.append( nPointsAttribute )
        #min distance between samples
        minDistAttribute = QgsNewVectorLayerDialog.AttributeEntry()
        minDistAttribute.name = 'minDist'
        minDistAttribute.type = 'Real'
        minDistAttribute.width = 14
        minDistAttribute.precision = 6
        attributeList.append( minDistAttribute )
        
        filename = QgsNewVectorLayerDialog.runAndCreateLayer( None, 'UTF-8', QGis.Polygon, attributeList )
        if not filename.isEmpty():
            vlayer = self.iface.addVectorLayer( filename,  QFileInfo( filename ).baseName(),  'ogr')
            self.fillLayerComboBox( self.mStrataLayerComboBox,  QGis.Polygon,  False )
            self.mStrataLayerComboBox.setCurrentIndex( self.mStrataLayerComboBox.findData( vlayer.id() ) )
        
    def createNewSurveyAreaLayer( self ):
        filename = QgsNewVectorLayerDialog.runAndCreateLayer( None, 'UTF-8', QGis.Polygon )
        if not filename.isEmpty():
            vlayer = self.iface.addVectorLayer( filename,  QFileInfo( filename ).baseName(),  'ogr')
            self.fillLayerComboBox( self.mSurveyAreaLayerComboBox,  QGis.Polygon,  True )
            self.mSurveyAreaLayerComboBox.setCurrentIndex( self.mSurveyAreaLayerComboBox.findData(  vlayer.id())  )
        
    def createNewBaselineLayer( self ):
        attributeList = []
        #strata id
        strataIdAttribute = QgsNewVectorLayerDialog.AttributeEntry()
        strataIdAttribute.name = 'strata_id'
        strataIdAttribute.type = 'Integer'
        strataIdAttribute.width = 10
        strataIdAttribute.precision = 0
        attributeList.append( strataIdAttribute )
        filename = QgsNewVectorLayerDialog.runAndCreateLayer( None, 'UTF-8', QGis.Line, attributeList )
        if not filename.isEmpty():
            vlayer = self.iface.addVectorLayer( filename,  QFileInfo( filename ).baseName(),  'ogr')
            self.fillLayerComboBox( self.mSurveyBaselineLayerComboBox,  QGis.Line,  True )
            self.mSurveyBaselineLayerComboBox.setCurrentIndex( self.mSurveyBaselineLayerComboBox.findData( vlayer.id() ) )
        
    def createStrataMinDistAttribute( self ):
        strataLayer = QgsMapLayerRegistry.instance().mapLayer( self.mStrataLayerComboBox.itemData( self.mStrataLayerComboBox.currentIndex() ).toString() )
        if strataLayer is None:
            return
            
        d = QgsAddAttrDialog( strataLayer )
        d.setType( 1 )
        d.setFieldName( 'min_dist' )
        d.setWidth( 14 )
        d.setPrecision( 6 )
        if d.exec_() == QDialog.Accepted:
            fieldList = []
            newField = d.field()
            fieldList.append( newField )
            strataLayer.dataProvider().addAttributes( fieldList )
            strataLayer.updateFieldMap()
            self.setMinimumDistanceAttributes( self. mStrataLayerComboBox.currentIndex() )
            #try to set new attribute as current item
            fieldIndex = strataLayer.dataProvider().fieldNameIndex( newField.name() )
            if fieldIndex != -1:
                self.mMinimumDistanceAttributeComboBox.setCurrentIndex( self.mMinimumDistanceAttributeComboBox.findData( fieldIndex ) )
                
    def createStrataNSamplePointsAttribute( self ):
        strataLayer = QgsMapLayerRegistry.instance().mapLayer( self.mStrataLayerComboBox.itemData( self.mStrataLayerComboBox.currentIndex() ).toString() )
        if strataLayer is None:
            return
            
        d = QgsAddAttrDialog( strataLayer )
        d.setType( 0 )
        d.setFieldName( 'n_points' )
        d.setWidth( 10 )
        d.setPrecision( 0 )
        if d.exec_() == QDialog.Accepted:
            fieldList = []
            newField = d.field()
            fieldList.append( newField )
            strataLayer.dataProvider().addAttributes( fieldList )
            strataLayer.updateFieldMap()
            self.setNSamplePointsAttributes( self. mStrataLayerComboBox.currentIndex() )
            #try to set new attribute as current item
            fieldIndex = strataLayer.dataProvider().fieldNameIndex( newField.name() )
            if fieldIndex != -1:
                self.mNSamplePointsComboBox.setCurrentIndex( self.mNSamplePointsComboBox.findData( fieldIndex ) )
            
            
    def createStrataIdAttribute( self ):
        strataLayer = QgsMapLayerRegistry.instance().mapLayer( self.mStrataLayerComboBox.itemData( self.mStrataLayerComboBox.currentIndex() ).toString() )
        if strataLayer is None:
            return
            
        d = QgsAddAttrDialog( strataLayer )
        d.setType( 0 )
        d.setFieldName( 'strata_id' )
        d.setWidth( 10 )
        d.setPrecision( 0 )
        if d.exec_() == QDialog.Accepted:
            fieldList = []
            newField = d.field()
            fieldList.append( newField )
            strataLayer.dataProvider().addAttributes( fieldList )
            strataLayer.updateFieldMap()
            self.setStrataIdAttributes( self. mStrataLayerComboBox.currentIndex() )
            #try to set new attribute as current item
            fieldIndex = strataLayer.dataProvider().fieldNameIndex( newField.name() )
            if fieldIndex != -1:
                self.mStrataIdAttributeComboBox.setCurrentIndex( self.mStrataIdAttributeComboBox.findData( fieldIndex ) )
            
            
    def createBaselineStrataAttribute( self ):
        baselineLayer  = QgsMapLayerRegistry.instance().mapLayer( self.mSurveyBaselineLayerComboBox.itemData( self.mSurveyBaselineLayerComboBox.currentIndex() ).toString() )
        if not baselineLayer:
            return
    
        d = QgsAddAttrDialog( baselineLayer )
        d.setType( 0 )
        d.setFieldName( 'strata_id' )
        d.setWidth( 10 )
        d.setPrecision( 0 )
        if d.exec_() == QDialog.Accepted:
            fieldList = []
            newField = d.field()
            fieldList.append( newField )
            baselineLayer.dataProvider().addAttributes( fieldList )
            baselineLayer.updateFieldMap()
            self.setBaselineStrataIdAttributes( self.mSurveyBaselineLayerComboBox.currentIndex() )
            #try to set new attribute as current item
            fieldIndex = baselineLayer.dataProvider().fieldNameIndex( newField.name() )
            if fieldIndex != -1:
                self.mStratumIdComboBox.setCurrentIndex( self.mStratumIdComboBox.findData( fieldIndex ) )
        
        
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
