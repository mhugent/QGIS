from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from surveyinitdialog import SurveyInitDialog
from surveydesigndialog import SurveyDesignDialog
from surveyevaluationdialog import SurveyEvaluationDialog
from surveyevaluation import SurveyEvaluation

class SurveyDesignPlugin:

    def __init__(self, iface):
        self.iface = iface

    def initGui(self):
        menuBar = self.iface.mainWindow().menuBar()
        surveyDesignMenu = QMenu( QCoreApplication.translate("SurveyDesignPlugin","Survey") ,  menuBar)

        isSurveyProject = self.projectContainsSurveyDesign()

        self.actionSurveyProperties = QAction( 'Survey design',  self.iface.mainWindow() )
        QObject.connect( self.actionSurveyProperties, SIGNAL("triggered()"), self.initSurveyDesign)

        self.actionEvaluateSurvey = QAction( 'Survey evaluation',  self.iface.mainWindow() )
        QObject.connect( self.actionEvaluateSurvey,  SIGNAL("triggered()"),  self.openSurveyEvaluationDialog )
        self.checkSurveyDesignPossible()
        surveyDesignMenu.addAction(  self.actionSurveyProperties )
        surveyDesignMenu.addAction( self.actionEvaluateSurvey )
        self.surveyDesignAction = menuBar.addMenu( surveyDesignMenu )

        QObject.connect( self.iface, SIGNAL('projectRead()'), self.projectLoaded )

    def unload(self):
        self.iface.mainWindow().menuBar().removeAction( self.surveyDesignAction )

    def initSurveyDesign(self):
        dialog = SurveyInitDialog( self.iface.mainWindow(),  self.iface)
        mainWindowGeom = self.iface.mainWindow().frameGeometry()
        dialog.move( mainWindowGeom.width() / 2.0,  mainWindowGeom.top() )
        dialog.show()

    def openSurveyDesignWidget(self):
        self.designDialog = SurveyDesignDialog( self.iface.mainWindow(),  self.iface )
        self.designDialog.show()

    def openSurveyEvaluationDialog(self):
        dialog = SurveyEvaluationDialog( self.iface,  self.iface.mainWindow() )
        if dialog.exec_() != QDialog.Accepted:
            return

        #check for which strata to run the analysis (the user has to change the 'ok' attribute of the baselines to something different than 'f')
        strataExcludeSet = set()
        baselineClipLayerId = dialog.baselineClipLayerId()
        if not baselineClipLayerId.isEmpty():
            baselineClipLayer = QgsMapLayerRegistry.instance().mapLayers()[ baselineClipLayerId ]
            okAttribute = baselineClipLayer.fieldNameIndex("ok" )
            strataIdAttribute = baselineClipLayer.fieldNameIndex( "stratum_id" )

            if okAttribute != -1 and strataIdAttribute != 1:
                attList = [okAttribute,  strataIdAttribute]
                baselineClipLayer.select( attList,  QgsRectangle(),  False )

                f = QgsFeature()
                while baselineClipLayer.nextFeature( f ):
                    okString = f.attributeMap()[okAttribute].toString()[0]
                    stratumId = f.attributeMap()[strataIdAttribute].toInt()[0]
                    if okString == 'f' or okString == 'F':
                        strataExcludeSet.add( stratumId )

        eval = SurveyEvaluation( self.iface,  dialog.sampleLayerId(), dialog.stratumId(),  dialog.arealAvailability(),  dialog.catch(),  dialog.dist(),  dialog.width(),  dialog.verticalAvailability(),  strataExcludeSet )
        eval.evaluateSurvey( dialog.speciesVulnerability() )

    def checkSurveyDesignPossible( self ):
            projectContainsDesign = self.projectContainsSurveyDesign()
            self.actionSurveyProperties.setEnabled( True )
            self.actionEvaluateSurvey.setEnabled( True )

    def projectContainsSurveyDesign(self):
        surveyAreaLayerId = QgsProject.instance().readEntry( "Survey",  "SurveyAreaLayer" )[0]
        strataLayerId = QgsProject.instance().readEntry( "Survey",  "StrataLayer" )[0]

        if not strataLayerId:
            return False
        else:
            return True

    def projectLoaded( self ):
        self.checkSurveyDesignPossible()
