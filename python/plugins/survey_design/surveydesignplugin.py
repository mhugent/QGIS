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
        surveyDesignMenu = QMenu( QCoreApplication.translate("SurveyDesignPlugin","Survey") ,  menuBar)
        
        isSurveyProject = self.projectContainsSurveyDesign()

        self.actionInitSurvey = QAction( 'Init survey',  self.iface.mainWindow() )
        QObject.connect( self.actionInitSurvey, SIGNAL("triggered()"), self.initSurveyDesign)
      
        self.actionSurveyProperties = QAction( 'Survey design',  self.iface.mainWindow() )
        QObject.connect( self.actionSurveyProperties, SIGNAL("triggered()"), self.openSurveyDesignWidget )
        self.checkSurveyDesignPossible()
        
        surveyDesignMenu.addAction( self.actionInitSurvey  )
        surveyDesignMenu.addAction(  self.actionSurveyProperties )
        self.surveyDesignAction = menuBar.addMenu( surveyDesignMenu )
        
        QObject.connect( self.iface, SIGNAL('projectRead()'), self.projectLoaded )
        
    def unload(self):
        self.iface.mainWindow().menuBar().removeAction( self.surveyDesignAction )
        
    def initSurveyDesign(self):
        dialog = SurveyInitDialog( self.iface)
        if dialog.exec_() == QDialog.Accepted:
            self.checkSurveyDesignPossible()
        
    def openSurveyDesignWidget(self):
        self.designDialog = SurveyDesignDialog( self.iface.mainWindow(),  self.iface )
        self.designDialog.show()
        
    def checkSurveyDesignPossible( self ):
            self.actionSurveyProperties.setEnabled( self.projectContainsSurveyDesign() )
        
    def projectContainsSurveyDesign(self):
        surveyAreaLayerId = QgsProject.instance().readEntry( "Survey",  "SurveyAreaLayer" )[0]
        strataLayerId = QgsProject.instance().readEntry( "Survey",  "StrataLayer" )[0]
        
        if strataLayerId.isEmpty():
            return False
        else:
            return True
            
    def projectLoaded( self ):
        self.checkSurveyDesignPossible()
