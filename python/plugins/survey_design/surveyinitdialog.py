from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from qgis.analysis import *
from surveydigitizetool import SurveyDigitizeTool
from ui_surveyinitdialogbase import Ui_SurveyInitDialogBase

class SurveyInitDialog( QDialog,  Ui_SurveyInitDialogBase ):

    def __init__(self,  parent,  iface):
        QDialog.__init__(self, parent)
        self.setWindowFlags( Qt.Dialog | Qt.Tool | Qt.WindowStaysOnTopHint )
        self.iface = iface
        self.setupUi(self)

        #possible entries in layer combo boxes
        self.fillLayerComboBox( self.mSurveyAreaLayerComboBox,  QGis.Polygon,  True )
        self.fillLayerComboBox( self.mSurveyBaselineLayerComboBox, QGis.Line,  True )
        self.fillLayerComboBox( self.mStrataLayerComboBox,  QGis.Polygon,  False )

        #set layers from project file
        surveyAreaLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyAreaLayer','' )
        if surveyAreaLayer[0]:
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

        #slots for combo boxes (write to project)
        self.blockSignals( True )
        QObject.connect( self.mSurveyAreaLayerComboBox,  SIGNAL(  'currentIndexChanged( int )' ),  self.writeToProject  )
        QObject.connect( self.mSurveyBaselineLayerComboBox,  SIGNAL(  'currentIndexChanged( int )' ),  self.writeToProject  )
        QObject.connect( self.mStratumIdComboBox,  SIGNAL(  'currentIndexChanged( int )' ),  self.writeToProject  )
        QObject.connect( self.mStrataLayerComboBox,  SIGNAL(  'currentIndexChanged( int )' ),  self.writeToProject  )
        QObject.connect( self.mMinimumDistanceAttributeComboBox,  SIGNAL(  'currentIndexChanged( int )' ),  self.writeToProject  )
        QObject.connect( self.mMinDistanceUnitsComboBox,  SIGNAL(  'currentIndexChanged( int )' ),  self.writeToProject  )
        QObject.connect( self.mNSamplePointsComboBox,  SIGNAL(  'currentIndexChanged( int )' ),  self.writeToProject  )
        QObject.connect( self.mStrataIdAttributeComboBox,  SIGNAL(  'currentIndexChanged( int )' ),  self.writeToProject  )
        QObject.connect( self.mMinimumTransectLengthSpinBox,  SIGNAL( 'valueChanged( double )' ),  self.writeToProject )

        QObject.connect( self.mCreateSampleButton, SIGNAL('clicked()'), self.createSample )

        self.mStratumIdComboBox.setCurrentIndex(  self.mStratumIdComboBox.findText( QgsProject.instance().readEntry( 'Survey', 'BaselineStrataId' )[0]  ) )
        self.mMinimumDistanceAttributeComboBox.setCurrentIndex( self.mMinimumDistanceAttributeComboBox.findText( QgsProject.instance().readEntry( 'Survey', 'StrataMinDistance' )[0]  )  )
        self.mNSamplePointsComboBox.setCurrentIndex( self.mNSamplePointsComboBox.findText( QgsProject.instance().readEntry( 'Survey', 'StrataNSamplePoints' )[0]  )  )
        self.mStrataIdAttributeComboBox.setCurrentIndex( self.mStrataIdAttributeComboBox.findText( QgsProject.instance().readEntry( 'Survey', 'StrataId' ) [0] )  )
        self.mNSamplePointsComboBox.setCurrentIndex( self.mNSamplePointsComboBox.findText( QgsProject.instance().readEntry('Survey','StrataNSamplePoints' )[0] ) )

        self.mMinDistanceUnitsComboBox.addItem( self.tr( "Stratum units" ) )
        self.mMinDistanceUnitsComboBox.addItem( self.tr( "Meters" ) )
        minDistanceUnits = QgsProject.instance().readEntry( 'Survey',  'StrataMinDistanceUnits' )[0]
        if minDistanceUnits == u"Meters":
            self.mMinDistanceUnitsComboBox.setCurrentIndex( 1 )
        else:
            self.mMinDistanceUnitsComboBox.setCurrentIndex( 0 )

        self.mMinimumTransectLengthSpinBox.setValue( QgsProject.instance().readDoubleEntry( 'Survey',  'MinTransectLength',  0.0 )[0] )

        self.blockSignals( False )

        #init survey design tools
        QObject.connect( self.mAddStratumToolButton,  SIGNAL('clicked( bool )'),  self.addStratumToggled )
        QObject.connect( self.mAddSurveyAreaToolButton,  SIGNAL('clicked( bool )'),  self.addSurveyAreaToggled )
        QObject.connect( self.mAddBaselineToolButton,  SIGNAL('clicked( bool )'),  self.addBaselineToggled )
        self.mPointSurveyRadioButton.setChecked( True )
        self.setTransectButtonState()
        self.setSampleButtonState()
        QObject.connect( self.mShareBaselineCheckBox,  SIGNAL( 'stateChanged( int )'),  self.setTransectButtonState )

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
        return self.mSurveyAreaLayerComboBox.itemData( self.mSurveyAreaLayerComboBox.currentIndex() )

    #Return id of baseline layer
    def surveyBaselineLayer(self):
        return self.mSurveyBaselineLayerComboBox.itemData( self.mSurveyBaselineLayerComboBox.currentIndex() )

    #Return attribut index of stratum id in baseline layer (-1 if none)
    def baselineStrataId( self ):
        return self.mStratumIdComboBox.itemText( self.mStratumIdComboBox.currentIndex() )

    #Return id of stratum layer
    def strataLayer( self ):
        return self.mStrataLayerComboBox.itemData( self.mStrataLayerComboBox.currentIndex() )

    #Return index of strata layer attribute containing the minimum distance between sample points
    def strataMinDistanceAttribute( self ):
        return self.mMinimumDistanceAttributeComboBox.itemText( self.mMinimumDistanceAttributeComboBox.currentIndex() )

    #Return index of strata layer attribut containing the number of sample points
    def strataNSamplePointsAttribute( self ):
        return self.mNSamplePointsComboBox.itemText(  self.mNSamplePointsComboBox.currentIndex() )

    #Return index of strata layer containing the feature id
    def strataId( self ):
        return self.mStrataIdAttributeComboBox.itemText( self.mStrataIdAttributeComboBox.currentIndex() )

    #Fill attributes into mStratumIdComboBox
    def setBaselineStrataIdAttributes( self, index ):
        self.mStratumIdComboBox.clear()
        self.mStratumIdComboBox.addItem( self.tr( 'None' ), -1 )
        layerId = self.mSurveyBaselineLayerComboBox.itemData( index )
        if layerId:
            layer = QgsMapLayerRegistry.instance().mapLayer( layerId )
            if not layer is None:
                if not layer.type() == QgsMapLayer.VectorLayer:
                    return

                fieldList = layer.pendingFields().toList()
                for field in fieldList:
                    self.mStratumIdComboBox.addItem( field.name() )

    def setMinimumDistanceAttributes( self, index ):
        self.mMinimumDistanceAttributeComboBox.clear()

        self.mMinimumDistanceAttributeComboBox.addItem( self.tr('None') )

        layerId = self.mStrataLayerComboBox.itemData( index )
        if layerId:
            layer = QgsMapLayerRegistry.instance().mapLayer( layerId )
            if not layer is None:
                if not layer.type() == QgsMapLayer.VectorLayer:
                    return

                fieldList = layer.pendingFields().toList()
                for field in fieldList:
                    self.mMinimumDistanceAttributeComboBox.addItem( field.name() )

    def setNSamplePointsAttributes( self, index ):
        self.mNSamplePointsComboBox.clear()
        layerId = self.mStrataLayerComboBox.itemData( index )
        if layerId:
            layer = QgsMapLayerRegistry.instance().mapLayer( layerId )
            if not layer is None:
                if not layer.type() == QgsMapLayer.VectorLayer:
                    return

                fieldList = layer.pendingFields().toList()
                for field in fieldList:
                    self.mNSamplePointsComboBox.addItem( field.name() )

    def setStrataIdAttributes( self, index ):
        self.mStrataIdAttributeComboBox.clear()
        layerId = self.mStrataLayerComboBox.itemData( index )
        if layerId:
            layer = QgsMapLayerRegistry.instance().mapLayer( layerId )
            if not layer is None:
                if not layer.type() == QgsMapLayer.VectorLayer:
                    return

                fieldList = layer.pendingFields().toList()
                for field in fieldList:
                    self.mStrataIdAttributeComboBox.addItem( field.name() )

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
        if filename:
            vlayer = self.iface.addVectorLayer( filename,  QFileInfo( filename ).baseName(),  'ogr')
            self.fillLayerComboBox( self.mStrataLayerComboBox,  QGis.Polygon,  False )
            self.mStrataLayerComboBox.setCurrentIndex( self.mStrataLayerComboBox.findData( vlayer.id() ) )

    def createNewSurveyAreaLayer( self ):
        filename = QgsNewVectorLayerDialog.runAndCreateLayer( None, 'UTF-8', QGis.Polygon )
        if filename:
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
        if filename:
            vlayer = self.iface.addVectorLayer( filename,  QFileInfo( filename ).baseName(),  'ogr')
            self.fillLayerComboBox( self.mSurveyBaselineLayerComboBox,  QGis.Line,  True )
            self.mSurveyBaselineLayerComboBox.setCurrentIndex( self.mSurveyBaselineLayerComboBox.findData( vlayer.id() ) )

    def createStrataMinDistAttribute( self ):
        strataLayer = QgsMapLayerRegistry.instance().mapLayer( self.mStrataLayerComboBox.itemData( self.mStrataLayerComboBox.currentIndex() ) )
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
            strataLayer.updateFields()
            self.setMinimumDistanceAttributes( self. mStrataLayerComboBox.currentIndex() )
            #try to set new attribute as current item
            fieldIndex = strataLayer.dataProvider().fieldNameIndex( newField.name() )
            if fieldIndex != -1:
                self.mMinimumDistanceAttributeComboBox.setCurrentIndex( self.mMinimumDistanceAttributeComboBox.findData( fieldIndex ) )

    def createStrataNSamplePointsAttribute( self ):
        strataLayer = QgsMapLayerRegistry.instance().mapLayer( self.mStrataLayerComboBox.itemData( self.mStrataLayerComboBox.currentIndex() ) )
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
            strataLayer.updateFields()
            self.setNSamplePointsAttributes( self. mStrataLayerComboBox.currentIndex() )
            #try to set new attribute as current item
            fieldIndex = strataLayer.dataProvider().fieldNameIndex( newField.name() )
            if fieldIndex != -1:
                self.mNSamplePointsComboBox.setCurrentIndex( self.mNSamplePointsComboBox.findData( fieldIndex ) )


    def createStrataIdAttribute( self ):
        strataLayer = QgsMapLayerRegistry.instance().mapLayer( self.mStrataLayerComboBox.itemData( self.mStrataLayerComboBox.currentIndex() ) )
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
            strataLayer.updateFields()
            self.setStrataIdAttributes( self. mStrataLayerComboBox.currentIndex() )
            #try to set new attribute as current item
            fieldIndex = strataLayer.dataProvider().fieldNameIndex( newField.name() )
            if fieldIndex != -1:
                self.mStrataIdAttributeComboBox.setCurrentIndex( self.mStrataIdAttributeComboBox.findData( fieldIndex ) )


    def createBaselineStrataAttribute( self ):
        baselineLayer  = QgsMapLayerRegistry.instance().mapLayer( self.mSurveyBaselineLayerComboBox.itemData( self.mSurveyBaselineLayerComboBox.currentIndex() ) )
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
            baselineLayer.updateFields()
            self.setBaselineStrataIdAttributes( self.mSurveyBaselineLayerComboBox.currentIndex() )
            #try to set new attribute as current item
            fieldIndex = baselineLayer.dataProvider().fieldNameIndex( newField.name() )
            if fieldIndex != -1:
                self.mStratumIdComboBox.setCurrentIndex( self.mStratumIdComboBox.findData( fieldIndex ) )


    def writeToProject( self ):
        #store the settings to the properties section of the project file
        QgsProject.instance().writeEntry( 'Survey', 'SurveyAreaLayer', self.surveyAreaLayer() )
        QgsProject.instance().writeEntry( 'Survey', 'SurveyBaselineLayer', self.surveyBaselineLayer() )
        QgsProject.instance().writeEntry( 'Survey', 'StrataLayer', self.strataLayer() )
        QgsProject.instance().writeEntry( 'Survey', 'BaselineStrataId', self.baselineStrataId() )
        QgsProject.instance().writeEntry( 'Survey', 'StrataMinDistance', self.strataMinDistanceAttribute() )
        QgsProject.instance().writeEntry( 'Survey', 'StrataNSamplePoints', self.strataNSamplePointsAttribute() )
        QgsProject.instance().writeEntry( 'Survey', 'StrataId', self.strataId() )
        QgsProject.instance().writeEntry( 'Survey',  'MinTransectLength',  self.mMinimumTransectLengthSpinBox.value() )
        minDistanceUnitString = self.mMinDistanceUnitsComboBox.currentText()
        print minDistanceUnitString
        QgsProject.instance().writeEntry( 'Survey',  'StrataMinDistanceUnits',  minDistanceUnitString );
        self.setTransectButtonState()
        self.setSampleButtonState()


    def createSample( self ):
        if self.mPointSurveyRadioButton.isChecked():
            self.createPointSample()
        elif self.mTransectSurveyRadioButton.isChecked():
            self.createTransectSample()

    def createTransectSample( self ):
        s = QSettings()
        saveDir = s.value( '/SurveyPlugin/SaveDir','')

        outputPointShape = QFileDialog.getSaveFileName( self, QCoreApplication.translate( 'SurveyDesignDialog', 'Select output point shape file' ), saveDir, QCoreApplication.translate( 'SurveyDesignDialog', 'Shapefiles (*.shp)' ) )
        if not outputPointShape:
            return
        else:
            saveDir = QFileInfo( outputPointShape ).absolutePath()


        outputLineShape = QFileDialog.getSaveFileName( self, QCoreApplication.translate( 'SurveyDesignDialog', 'Select output line shape file' ), saveDir, QCoreApplication.translate( 'SurveyDesignDialog', 'Shapefiles (*.shp)' ) )
        if not outputLineShape:
            return
        else:
            saveDir = QFileInfo( outputLineShape ).absolutePath()

        usedBaselineShape = QFileDialog.getSaveFileName( self, QCoreApplication.translate( 'SurveyDesignDialog', 'Select output baseline file' ), saveDir, QCoreApplication.translate( 'SurveyDesignDialog', 'Shapefiles (*.shp)' ) )
        if not usedBaselineShape:
            return

        strataLayer = QgsProject.instance().readEntry( 'Survey', 'StrataLayer' )[0]
        surveyBaselineLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyBaselineLayer')[0]
        baselineStrataId = QgsProject.instance().readEntry( 'Survey', 'BaselineStrataId')[0]
        strataMinDistance = QgsProject.instance().readEntry( 'Survey', 'StrataMinDistance' )[0]
        strataNSamplePoints = QgsProject.instance().readEntry( 'Survey', 'StrataNSamplePoints' )[0]
        strataId = QgsProject.instance().readEntry( 'Survey',  'StrataId' )[0]

        minDistanceUnitsString = QgsProject.instance().readEntry( 'Survey',  'StrataMinDistanceUnits' )[0]
        minDistanceUnits = QgsTransectSample.StrataUnits
        if minDistanceUnitsString == u"Meters":
            minDistanceUnits = QgsTransectSample.Meters

        strataMapLayer = QgsMapLayerRegistry.instance().mapLayer( strataLayer )
        baselineMapLayer = QgsMapLayerRegistry.instance().mapLayer( surveyBaselineLayer )
        transectSample = QgsTransectSample(  strataMapLayer, strataId , strataMinDistance, strataNSamplePoints, minDistanceUnits, baselineMapLayer, self.mShareBaselineCheckBox.isChecked(),
        baselineStrataId, outputPointShape, outputLineShape,  usedBaselineShape,  self.mMinimumTransectLengthSpinBox.value() )
        pd = QProgressDialog(  'Calculating transects...', 'Abort',  0,  0,  self )
        pd.setWindowTitle( 'Transect generation' )
        pd.show();
        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        transectSample.createSample( pd )
        QApplication.restoreOverrideCursor()

        s.setValue( '/SurveyPlugin/SaveDir', QFileInfo( outputLineShape ).absolutePath() )
        self.iface.addVectorLayer( outputPointShape, QFileInfo( outputPointShape ).baseName(), 'ogr' )
        self.iface.addVectorLayer( outputLineShape, QFileInfo( outputLineShape ).baseName(), 'ogr' )
        self.iface.addVectorLayer( usedBaselineShape,  QFileInfo( usedBaselineShape ).baseName(),  'ogr' )

    def createPointSample( self ):
        #get StrataLayer, StrataMinDistance, StrataNSamplePoints
        strataLayer = QgsProject.instance().readEntry( 'Survey', 'StrataLayer' )[0]
        strataMinDistance = QgsProject.instance().readNumEntry( 'Survey', 'StrataMinDistance', -1 )[0]
        strataNSamplePoints = QgsProject.instance().readNumEntry( 'Survey', 'StrataNSamplePoints', -1 )[0]

        if not strataLayer or strataNSamplePoints < 0:
            print 'Error'
            return

        print 'no Error'
        inputLayer = QgsMapLayerRegistry.instance().mapLayer( strataLayer )

        s = QSettings()
        saveDir = s.value( '/SurveyPlugin/SaveDir','')

        outputShape = QFileDialog.getSaveFileName( self, QCoreApplication.translate( 'SurveyDesignDialog', 'Select output shape file' ), saveDir, QCoreApplication.translate( 'SurveyDesignDialog', 'Shapefiles (*.shp)' ) )
        print outputShape
        if not outputShape:
            return

        p = QgsPointSample (  inputLayer, outputShape, strataNSamplePoints, strataMinDistance )
        p.createRandomPoints( None )

        s.setValue( '/SurveyPlugin/SaveDir', QFileInfo( outputShape ).absolutePath() )
        self.iface.addVectorLayer( outputShape, 'sample', 'ogr' )

    def setTransectButtonState(self):
        surveyBaselineLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyBaselineLayer')[0]
        baselineStrataId = QgsProject.instance().readNumEntry( 'Survey', 'BaselineStrataId', -1 )[0]
        strataMinDistance = QgsProject.instance().readNumEntry( 'Survey', 'StrataMinDistance', -1 )[0]
        strataNSamplePoints = QgsProject.instance().readNumEntry( 'Survey', 'StrataNSamplePoints', -1 )[0]
        if not surveyBaselineLayer or ( baselineStrataId == -1 and not self.mShareBaselineCheckBox.isChecked() ) or strataMinDistance == -1 or strataNSamplePoints == -1:
            self.mTransectSurveyRadioButton.setEnabled( False )
            self.mPointSurveyRadioButton.setChecked( True )
        else:
            self.mTransectSurveyRadioButton.setEnabled( True )

    def setSampleButtonState(self ):
        strataLayerId = QgsProject.instance().readEntry( "Survey",  "StrataLayer" )[0]
        minDistAttribute = self.mMinimumDistanceAttributeComboBox.itemData( self.mMinimumDistanceAttributeComboBox.currentIndex() ) #[0]
        nSamplePointsIndex = self.mNSamplePointsComboBox.currentIndex()
        if strataLayerId and minDistAttribute != -1 and nSamplePointsIndex != -1:
            self.mCreateSampleButton.setEnabled( True )
        else:
            self.mCreateSampleButton.setEnabled( False )

    def blockSignals(self,  blocked ):
        self.mSurveyBaselineLayerComboBox.blockSignals( blocked )
        self.mStratumIdComboBox.blockSignals( blocked )
        self.mStrataLayerComboBox.blockSignals( blocked )
        self.mMinimumDistanceAttributeComboBox.blockSignals( blocked )
        self.mMinDistanceUnitsComboBox.blockSignals( blocked )
        self.mNSamplePointsComboBox.blockSignals( blocked )
        self.mStrataIdAttributeComboBox.blockSignals( blocked )

    @pyqtSlot()
    def addStratumToggled(self,  toggleState ):
        strataLayerId = QgsProject.instance().readEntry( 'Survey', 'StrataLayer' )[0]
        if not strataLayerId:
            return
        self.stratumDigiTool = SurveyDigitizeTool( strataLayerId,  self.iface.mapCanvas(), strataLayerId,  20,  True  )
        self.stratumDigiTool.setButton( self.mAddStratumToolButton )
        if toggleState == True:
            self.iface.mapCanvas().setMapTool( self.stratumDigiTool )
        else:
            self.stratumDigiTool.deactivate()
        self.iface.mapCanvas().refresh()

    @pyqtSlot()
    def addBaselineToggled(self,  toggleState ):
        surveyBaselineLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyBaselineLayer')[0]
        if not surveyBaselineLayer:
            return
        self.baselineDigiTool = SurveyDigitizeTool( surveyBaselineLayer,  self.iface.mapCanvas(),  surveyBaselineLayer,  20,  False )
        self.baselineDigiTool.setButton( self.mAddBaselineToolButton  )
        if toggleState:
            self.iface.mapCanvas().setMapTool( self.baselineDigiTool )
        else:
            self.baselineDigiTool.deactivate()
        self.iface.mapCanvas().refresh()

    @pyqtSlot()
    def addSurveyAreaToggled(self,  toggleState ):
        surveyAreaLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyAreaLayer' )[0]
        if not surveyAreaLayer:
            return
        self.surveyAreaTool = SurveyDigitizeTool( surveyAreaLayer,  self.iface.mapCanvas(),  surveyAreaLayer,  20,  True )
        self.surveyAreaTool.setButton( self.mAddSurveyAreaToolButton )
        if toggleState:
            self.iface.mapCanvas().setMapTool( self.surveyAreaTool )
        else:
            self.surveyAreaTool.deactivate()
        self.iface.mapCanvas().refresh()
