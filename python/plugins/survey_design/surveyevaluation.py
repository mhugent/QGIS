from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *

class SurveyEvaluation:
    def __init__(self,  iface,  sampleLayerId, arealAvailability,  catch,  dist,  width,  verticalAvailability):
        self.mIface = iface
        self.mSampleLayerId = sampleLayerId
        self.mArealAvailability = arealAvailability
        self.mCatch = catch
        self.mDist = dist
        self.mWidth = width
        self.mVerticalAvailability = verticalAvailability
    
    def evaluateSurvey(self,  speciesVulnerability):
        print speciesVulnerability
        
        #get stratum layer from project
        strataLayerId = QgsProject.instance().readEntry( "Survey",  "StrataLayer" )[0]
        if strataLayerId.isEmpty():
            return False
        strataLayer = QgsMapLayerRegistry.instance().mapLayer( strataLayerId )
        if stratalayer is None or not strataLayer.isValid():
            return
        
        #get station layer (should be passed as argument)
        stationLayer = QgsMapLayerRegistry.instance().mapLayer( self.mSampleLayerId )
        if stationLayer is None or not stationLayer.isValid():
            return False
            
        #loop over station table to calculate src
        stationLayer.select( stationLayer.pendingAllAttributesList() )
        f = QgsFeature()
        while( stationLayer.nextFeature( f ) ):
            #stat[stncounter].scr = catchin / (distin*widthin*vulnin*vavailin);
            featureAttributes = f.attributeMap()
            stationCatch = featureAttributes[self.mCatch].toDouble()[0]
            stationDist = featureAttributes[self.mDist].toDouble()[0]
            stationWidth = featureAttributes[self.mWidth].toDouble()[0]
            stationVavail = featureAttributes[self.mVerticalAvailability].toDouble()[0]
            
            scr = stationCatch / ( stationDist * stationWidth / 1000.0 * speciesVlunerability * stationVavail )
            
            #write scr into station table 
        
        #loop over station table to calculate strata cv, bio, ...
        
        print self.mSampleLayerId
        print self.mArealAvailability
        print self.mCatch
        print self.mDist
        print self.mWidth
        print self.mVerticalAvailability
        
        return True
