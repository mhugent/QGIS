from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.analysis import *
from ui_surveydesigndialogbase import Ui_SurveyDesignDialogBase
from surveydigitizetool import SurveyDigitizeTool

class SurveyDesignDialog( QDialog, Ui_SurveyDesignDialogBase ):
    
    def __init__(self, parent,  iface):
        QDialog.__init__(self, None)
        self.setWindowFlags( Qt.Dialog | Qt.Tool | Qt.WindowStaysOnTopHint )
        
        self.setupUi(self)
        self.iface = iface
        self.mPointSurveyRadioButton.setChecked( True )
       
        surveyBaselineLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyBaselineLayer')[0]
        baselineStrataId = QgsProject.instance().readNumEntry( 'Survey', 'BaselineStrataId', -1 )[0]
        strataMinDistance = QgsProject.instance().readNumEntry( 'Survey', 'StrataMinDistance', -1 )[0]
        strataNSamplePoints = QgsProject.instance().readNumEntry( 'Survey', 'StrataNSamplePoints', -1 )[0]
        if surveyBaselineLayer.isEmpty() or baselineStrataId == -1 or strataMinDistance == -1 or strataNSamplePoints == -1:
           self.mTransectSurveyRadioButton.setEnabled( False )
           
        QObject.connect( self.mCreateSampleButton, SIGNAL('clicked()'), self.createSample )
    
        #create digitize map tools
        strataLayerId = QgsProject.instance().readEntry( 'Survey', 'StrataLayer' )[0]
        self.stratumDigiTool = SurveyDigitizeTool( strataLayerId,  self.iface.mapCanvas(), strataLayerId,  20,  True  )
        self.stratumDigiTool.setButton( self.mAddStratumToolButton )
        QObject.connect( self.mAddStratumToolButton,  SIGNAL('clicked( bool )'),  self.addStratumToggled )
        
        if not surveyBaselineLayer.isEmpty():
            self.baselineDigiTool = SurveyDigitizeTool( surveyBaselineLayer,  self.iface.mapCanvas(),  strataLayerId,  20,  False )
            self.baselineDigiTool.setButton( self.mAddBaselineToolButton  )
            QObject.connect( self.mAddBaselineToolButton,  SIGNAL('clicked( bool )'),  self.addBaselineToggled )
        
        surveyAreaLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyAreaLayer' )[0]
        if not surveyAreaLayer.isEmpty():
            self.surveyAreaTool = SurveyDigitizeTool( surveyAreaLayer,  self.iface.mapCanvas(),  strataLayerId,  20,  True )
            self.surveyAreaTool.setButton( self.mAddSurveyAreaToolButton )
            QObject.connect( self.mAddSurveyAreaToolButton,  SIGNAL('clicked( bool )'),  self.addSurveyAreaToggled )
           
    def createSample( self ):
        if self.mPointSurveyRadioButton.isChecked():
            self.createPointSample()
        elif self.mTransectSurveyRadioButton.isChecked():
            self.createTransectSample()
   
    def createTransectSample( self ):
        s = QSettings()
        saveDir = s.value( '/SurveyPlugin/SaveDir','').toString()
        
        outputPointShape = QFileDialog.getSaveFileName( self, QCoreApplication.translate( 'SurveyDesignDialog', 'Select output point shape file' ), saveDir, QCoreApplication.translate( 'SurveyDesignDialog', 'Shapefiles (*.shp)' ) )
        if outputPointShape.isEmpty():
            return
        
        outputLineShape = QFileDialog.getSaveFileName( self, QCoreApplication.translate( 'SurveyDesignDialog', 'Select output line shape file' ), saveDir, QCoreApplication.translate( 'SurveyDesignDialog', 'Shapefiles (*.shp)' ) )
        if outputLineShape.isEmpty():
            return
            
        usedBaselineShape = QFileDialog.getSaveFileName( self, QCoreApplication.translate( 'SurveyDesignDialog', 'Select output baseline file' ), saveDir, QCoreApplication.translate( 'SurveyDesignDialog', 'Shapefiles (*.shp)' ) )
        if usedBaselineShape.isEmpty():
            return
        
        strataLayer = QgsProject.instance().readEntry( 'Survey', 'StrataLayer' )[0]
        surveyBaselineLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyBaselineLayer')[0]
        baselineStrataId = QgsProject.instance().readNumEntry( 'Survey', 'BaselineStrataId', -1 )[0]
        strataMinDistance = QgsProject.instance().readNumEntry( 'Survey', 'StrataMinDistance', -1 )[0]
        strataNSamplePoints = QgsProject.instance().readNumEntry( 'Survey', 'StrataNSamplePoints', -1 )[0]
        strataId = QgsProject.instance().readNumEntry( 'Survey',  'StrataId',  -1 )[0]
        
        minDistanceUnitsString = QgsProject.instance().readEntry( 'Survey',  'StrataMinDistanceUnits' )[0]
        minDistanceUnits = QgsTransectSample.StrataUnits
        if minDistanceUnitsString == "Meters":
            minDistanceUnits = QgsTransectSample.Meters
        
        strataMapLayer = QgsMapLayerRegistry.instance().mapLayer( strataLayer )
        baselineMapLayer = QgsMapLayerRegistry.instance().mapLayer( surveyBaselineLayer )
        transectSample = QgsTransectSample(  strataMapLayer, strataId , strataMinDistance, minDistanceUnits,  strataNSamplePoints, baselineMapLayer, self.mShareBaselineCheckBox.isChecked(), 
        baselineStrataId, outputPointShape, outputLineShape,  usedBaselineShape )
        transectSample.createSample( None )
        
        s.setValue( '/SurveyPlugin/SaveDir', QFileInfo( outputLineShape ).absolutePath() )
        self.iface.addVectorLayer( outputPointShape, 'sample_point', 'ogr' )
        self.iface.addVectorLayer( outputLineShape, 'sample_line', 'ogr' )
        
    def createPointSample( self ):
        #get StrataLayer, StrataMinDistance, StrataNSamplePoints
        strataLayer = QgsProject.instance().readEntry( 'Survey', 'StrataLayer' )[0]
        strataMinDistance = QgsProject.instance().readNumEntry( 'Survey', 'StrataMinDistance', -1 )[0]
        strataNSamplePoints = QgsProject.instance().readNumEntry( 'Survey', 'StrataNSamplePoints', -1 )[0]
        
        if strataLayer.isEmpty() or strataNSamplePoints < 0:
            print 'Error'
            return
        
        print 'no Error'
        inputLayer = QgsMapLayerRegistry.instance().mapLayer( strataLayer )
        
        s = QSettings()
        saveDir = s.value( '/SurveyPlugin/SaveDir','').toString()
        
        outputShape = QFileDialog.getSaveFileName( self, QCoreApplication.translate( 'SurveyDesignDialog', 'Select output shape file' ), saveDir, QCoreApplication.translate( 'SurveyDesignDialog', 'Shapefiles (*.shp)' ) )
        print outputShape
        if outputShape.isEmpty():
            return
        
        p = QgsPointSample (  inputLayer, outputShape, strataNSamplePoints, strataMinDistance )
        p.createRandomPoints( None )
        
        s.setValue( '/SurveyPlugin/SaveDir', QFileInfo( outputShape ).absolutePath() )
        self.iface.addVectorLayer( outputShape, 'sample', 'ogr' )
     
    @pyqtSlot()
    def addStratumToggled(self,  toggleState ):
        if toggleState == True:
            self.iface.mapCanvas().setMapTool( self.stratumDigiTool )
        else:
            self.stratumDigiTool.deactivate()
        self.iface.mapCanvas().refresh()
        
    @pyqtSlot()
    def addBaselineToggled(self,  toggleState ):
        if toggleState:
            self.iface.mapCanvas().setMapTool( self.baselineDigiTool )
        else:
            self.baselineDigiTool.deactivate()
        self.iface.mapCanvas().refresh()
        
    @pyqtSlot()
    def addSurveyAreaToggled(self,  toggleState ):
        if toggleState:
            self.iface.mapCanvas().setMapTool( self.surveyAreaTool )
        else:
            self.surveyAreaTool.deactivate()
        self.iface.mapCanvas().refresh()
