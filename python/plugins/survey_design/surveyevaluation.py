from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
import math

class SurveyEvaluation:
    def __init__(self,  iface,  sampleLayerId, stratumId,  arealAvailability,  catch,  dist,  width,  verticalAvailability,  excludeStrata ):
        self.mIface = iface
        self.mSampleLayerId = sampleLayerId
        self.mStratumId = stratumId
        self.mArealAvailability = arealAvailability
        self.mCatch = catch
        self.mDist = dist
        self.mWidth = width
        self.mVerticalAvailability = verticalAvailability
        self.mExcludeStrata = excludeStrata
    
    def evaluateSurvey(self,  speciesVulnerability):
        #get stratum layer from project
        strataLayerId = QgsProject.instance().readEntry( "Survey",  "StrataLayer" )[0]
        if strataLayerId.isEmpty():
            return False
        strataLayer = QgsMapLayerRegistry.instance().mapLayer( strataLayerId )
        if strataLayer is None or not strataLayer.isValid():
            return False
        strataProvider = strataLayer.dataProvider()
        
        #get station layer
        stationLayer = QgsMapLayerRegistry.instance().mapLayer( self.mSampleLayerId )
        stationProvider = stationLayer.dataProvider()
        if stationLayer is None or not stationLayer.isValid():
            return False
        
        scrAttribute = self.calculateScrStationTable( stationLayer,  stationProvider,  speciesVulnerability )
        if scrAttribute == -1:
            return False
            
        #keep stratum statistics in a dict of lists (nstations, sumscr, meanscr, varscr, area, sumvar, bio)
        stratumInfo = {}
        stratumFeature = QgsFeature()
        stratumIdIndex = QgsProject.instance().readNumEntry( 'Survey', 'StrataId',  0 ) [0]
        stratumAttributeList = [stratumIdIndex]
        strataProvider.select( stratumAttributeList )
        while strataProvider.nextFeature( stratumFeature ):
            
            #get stratum id (!= feature id) and go to next one if stratum is in the exclude list
            currentStratumId = stratumFeature.attributeMap()[stratumIdIndex].toInt()[0]
            if currentStratumId in self.mExcludeStrata:
                continue
            
            stratumInfo[ currentStratumId ] = [0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0] #1. number of stations / 2. sumscr / 3. meanscr / 4. variance scr / 5. area / 6. sumvar / 7. bio
            
        stationProvider.select( stationProvider.attributeIndexes() )
        stationFeature = QgsFeature()
        while stationProvider.nextFeature( stationFeature ):
            stratumId = stationFeature.attributeMap()[self.mStratumId].toInt()[0]
            if stratumId in self.mExcludeStrata:
                continue
            stratumInfo[stratumId][0] += 1 #number of stations
            stratumInfo[stratumId][1] += ( stationFeature.attributeMap()[scrAttribute].toDouble()[0] ) #sumscr
                
        #calculate strata area and scr mean
        strataProvider.select(  stratumAttributeList )
        while strataProvider.nextFeature( stratumFeature ):
            stratumId = stratumFeature.attributeMap()[stratumIdIndex].toInt()[0]
            if stratumId in self.mExcludeStrata:
               continue
            stratInfo = stratumInfo[stratumId]
            if stratInfo[0] == 0:
                stratInfo[2] = 0
            else:
                stratInfo[2] = stratInfo[1] / stratInfo[0]
            stratInfo[4] = stratumFeature.geometry().area()
            
        #calculate scr variance per stratum
        stationProvider.select( stationProvider.attributeIndexes() )
        while stationProvider.nextFeature( stationFeature ):
            stratumId = stationFeature.attributeMap()[self.mStratumId].toInt()[0]
            if stratumId in self.mExcludeStrata:
                continue
            meanscr = stratumInfo[stratumId][2]
            scrValue = stationFeature.attributeMap()[scrAttribute].toDouble()[0]
            nStations = stratumInfo[stratumId][0]
            if nStations > 1:
                stratumInfo[stratumId][3] += ( (scrValue - meanscr)* (scrValue - meanscr) / ( nStations - 1 ) ) 
                
        #calculate sumvar per stratum
        stationProvider.select( stationProvider.attributeIndexes() )
        while stationProvider.nextFeature( stationFeature ):
            stratumId = stationFeature.attributeMap()[self.mStratumId].toInt()[0]
            if stratumId in self.mExcludeStrata:
                continue
            width = stationFeature.attributeMap()[self.mWidth].toDouble()[0]
            aavail = stationFeature.attributeMap()[self.mArealAvailability].toDouble()[0]
            
            denomSumvar = width * width * aavail * aavail * stratumInfo[stratumId][0]
            if denomSumvar != 0:
                stratumInfo[stratumId][5] += ( stratumInfo[stratumId][3] * stratumInfo[stratumId][4] ) / denomSumvar #sumvar
            denomBio = width * aavail
            if denomBio != 0:
                stratumInfo[stratumId][6] +=   (stratumInfo[stratumId][2] * stratumInfo[stratumId][4] ) / denomBio #bio
            
        #write the calculated stratum values to the datasource
        #first create the fields if they are not already there
        nstationsIndex = self.attributeIndex( strataProvider,  'nstations',  QVariant.Int )
        sumsrcIndex = self.attributeIndex( strataProvider,  'sumscr',  QVariant.Double )
        meansrcIndex= self.attributeIndex( strataProvider,  'meanscr',  QVariant.Double )
        varscrIndex = self.attributeIndex( strataProvider,  'varscr',  QVariant.Double )
        areaIndex = self.attributeIndex( strataProvider,  'area',  QVariant.Double )
        sumvarIndex = self.attributeIndex( strataProvider,  'sumvar',  QVariant.Double )
        bioIndex = self.attributeIndex( strataProvider,  'bio',  QVariant.Double )
        cvIndex = self.attributeIndex( strataProvider,  'cv',  QVariant.Double )
        
        strataProvider.select( strataProvider.attributeIndexes() )
        while strataProvider.nextFeature( stratumFeature ):
            stratumId = stratumFeature.attributeMap()[stratumIdIndex].toInt()[0]
            if stratumId in self.mExcludeStrata:
                continue
            featureAttributeMap = stratumFeature.attributeMap()
            if nstationsIndex != -1:
                featureAttributeMap[nstationsIndex] = QVariant(stratumInfo[stratumId][0])
            if sumsrcIndex != -1:
                featureAttributeMap[sumsrcIndex] = QVariant(stratumInfo[stratumId][1])
            if meansrcIndex != -1:
                featureAttributeMap[meansrcIndex] = QVariant(stratumInfo[stratumId][2])
            if varscrIndex != -1:
                featureAttributeMap[varscrIndex] = QVariant(stratumInfo[stratumId][3])
            if areaIndex != -1:
                featureAttributeMap[areaIndex] = QVariant(stratumInfo[stratumId][4])
            if sumvarIndex != -1:
                featureAttributeMap[sumvarIndex] = QVariant(stratumInfo[stratumId][5])
            if bioIndex != -1:
                featureAttributeMap[bioIndex] =  QVariant(stratumInfo[stratumId][6])
            if cvIndex != -1:
                if stratumInfo[stratumId][6] != 0:
                    featureAttributeMap[cvIndex] = QVariant(  100 * math.sqrt( stratumInfo[stratumId][5] ) / stratumInfo[stratumId][6]) #100.0 * sqrt( sumvar) / bio
            strataProvider.changeAttributeValues( { stratumId : featureAttributeMap} )

        return True
        
    #adds scr attribute to station table and returns index of scr attribute (or -1 in case of error)
    def calculateScrStationTable(self,  stationLayer,  stationProvider,  speciesVulnerability ):
        scrAttribute = self.attributeIndex( stationProvider,  'scr',  QVariant.Double )
        if scrAttribute == -1:
            return scrAttribute
        
        stationLayer.updateFieldMap()
            
        #loop over station table to calculate src
        changeAttributeMap = {}
        stationProvider.select( stationProvider.attributeIndexes() )
        f = QgsFeature()
        while( stationProvider.nextFeature( f ) ):
            #stat[stncounter].scr = catchin / (distin*widthin*vulnin*vavailin);
            featureAttributes = f.attributeMap()
            stationCatch = featureAttributes[self.mCatch].toDouble()[0]
            stationDist = featureAttributes[self.mDist].toDouble()[0]
            stationWidth = featureAttributes[self.mWidth].toDouble()[0]
            stationVavail = featureAttributes[self.mVerticalAvailability].toDouble()[0]
            
            denominator = stationDist * stationWidth / 1000.0 * speciesVulnerability * stationVavail
            scr = 0.0
            if denominator == 0:
                scr = 0.0
            else:
                scr = stationCatch / denominator
            changeAttribute = { scrAttribute : QVariant(scr) }
            changeAttributeMap[f.id()] = changeAttribute
        stationProvider.changeAttributeValues(  changeAttributeMap )
        return scrAttribute
            
    #Returns index of attribute by name. Tries to create the field if it does not yet exist
    def attributeIndex(self,  provider,  fieldName,  fieldType ):
        index = provider.fieldNameIndex( fieldName )
        if index == -1:
            newField = QgsField( fieldName,  fieldType )
            newFieldList = [newField]
            provider.addAttributes( newFieldList )
            return provider.fieldNameIndex( fieldName )
        return index
