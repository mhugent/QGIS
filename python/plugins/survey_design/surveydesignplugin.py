from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *
from surveyinitdialog import SurveyInitDialog
from surveydesigndialog import SurveyDesignDialog

class SurveyDesignPlugin:

    def __init__(self, iface):
        self.iface = iface
        
    def initGui(self):
        menuBar = self.iface.mainWindow().menuBar()
        surveyDesignMenu = QMenu( QCoreApplication.translate("SurveyDesignPlugin","Survey design") ,  menuBar)
        
        isSurveyProject = self.projectContainsSurveyDesign()

        self.actionInitSurvey = QAction( 'Init survey design',  self.iface.mainWindow() )
        QObject.connect( self.actionInitSurvey, SIGNAL("triggered()"), self.initSurveyDesign)
        self.actionInitSurvey.setEnabled( isSurveyProject == False )
      
        self.actionSurveyProperties = QAction( 'Survey design settings',  self.iface.mainWindow() )
        QObject.connect( self.actionSurveyProperties, SIGNAL("triggered()"), self.openSurveyDesignWidget )
        self.actionSurveyProperties.setEnabled( isSurveyProject == True )
        
        surveyDesignMenu.addAction( self.actionInitSurvey  )
        surveyDesignMenu.addAction(  self.actionSurveyProperties )
        self.surveyDesignAction = menuBar.addMenu( surveyDesignMenu )
        
    def unload(self):
        self.iface.mainWindow().menuBar().removeAction( self.surveyDesignAction )
        
    def initSurveyDesign(self):
        dialog = SurveyInitDialog()
        dialog.exec_()
        
    def openSurveyDesignWidget(self):
        dialog = SurveyDesignDialog()
        dialog.exec_()
        
    def projectContainsSurveyDesign(self):
        surveyAreaLayerId = QgsProject.instance().readEntry( "SurveyDesign",  "SurveyAreaLayerId" )
        strataLayerId = QgsProject.instance().readEntry( "SurveyDesign",  "StrataLayerId" )
        baseLineLayerId = QgsProject.instance().readEntry( "SurveyDesign",  "BaseLineLayerId" )
        
        if surveyAreaLayerId[0].isEmpty() or strataLayerId[0].isEmpty() or baseLineLayerId[0].isEmpty():
            return False
