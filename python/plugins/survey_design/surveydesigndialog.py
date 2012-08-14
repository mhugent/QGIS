from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from ui_surveydesigndialogbase import Ui_SurveyDesignDialogBase

class SurveyDesignDialog( QDialog, Ui_SurveyDesignDialogBase ):
    
    def __init__(self):
        QDialog.__init__(self, None)
        self.setupUi(self)
