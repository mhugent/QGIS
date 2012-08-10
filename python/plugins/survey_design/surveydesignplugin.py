from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

class SurveyDesignPlugin:

    def __init__(self, iface):
        self.iface = iface
        
    def initGui(self):
        pass
        
    def unload(self):
        pass
