from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from ui_surveyinitdialogbase import Ui_SurveyInitDialogBase

class SurveyInitDialog( QDialog,  Ui_SurveyInitDialogBase ):
    
    def __init__(self):
        QDialog.__init__(self, None)
        self.setupUi(self)
        
    #Return id of survey area layer (or empty string if none)
    def surveyAreaLayer(self):
        return mSurveyAreaLayerComboBox.itemData( mSurveyAreaLayerComboBox.currentIndex() ).toString()
        
    #Return id of baseline layer
    def surveyBaselineLayer(self):
        return mSurveyBaselineLayerComboBox.itemData( mSurveyBaselineLayerComboBox.currentIndex() ).toString()
     
    #Return attribut index of stratum id in baseline layer (-1 if none)
    def baselineStrataId( self ):
        return mStratumIdComboBox.itemData( mStratumIdComboBox.currentIndex() ).toInt()
        
    #Return id of stratum layer
    def strataLayer( self ):
        return mStrataLayerLabelComboBox.itemData( mStrataLayerLabelComboBox.currentIndex() ).toString()
        
    #Return index of strata layer attribute containing the minimum distance between sample points
    def strataMinDistanceAttribute( self ):
        return mMinimumDistanceAttributeComboBox.itemData( mMinimumDistanceAttributeComboBox.currentIndex() ).toString()
        
    #Return index of strata layer attribut containing the number of sample points
    def strataNSamplePointsAttribute( self ):
        return mNSamplePointsComboBox.itemData(  mNSamplePointsComboBox.currentIndex() ).toInt()
        
    #Return index of strata layer containing the feature id
    def strataId( self ):
        return mStrataIdAttributeComboBox.itemData( mStrataIdAttributeComboBox.currentIndex() ).toInt()
    
