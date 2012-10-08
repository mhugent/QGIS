from PyQt4.QtCore import *
from PyQt4.QtGui import *

class SurveyEvaluation:
    def __init__(self,  iface):
        self.mIface = iface
    
    def evaluateSurvey(self,  speciesVulnerability):
        print speciesVulnerability
        
        #get stratum layer from project
        strataLayerId = QgsProject.instance().readEntry( "Survey",  "StrataLayer" )[0]
        if strataLayerId.isEmpty():
            return False
        
        #get transect layer (should be passed as argument)
        
        return True
