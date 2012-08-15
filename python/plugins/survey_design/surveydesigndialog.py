from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.analysis import *
from ui_surveydesigndialogbase import Ui_SurveyDesignDialogBase

class SurveyDesignDialog( QDialog, Ui_SurveyDesignDialogBase ):
    
    def __init__(self):
        QDialog.__init__(self, None)
        self.setupUi(self)
        QObject.connect( self.mCreateSampleButton, SIGNAL('clicked()'), self.createSample )
        self.mPointSurveyRadioButton.setChecked( True )
       
        surveyBaselineLayer = QgsProject.instance().readEntry( 'Survey', 'SurveyBaselineLayer')[0]
        if surveyBaselineLayer.isEmpty():
           self.mTransectSurveyRadioButton.setEnabled( False )
        
        
    def createSample( self ):
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
