from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

class SurveyDigitizeTool( QgsMapTool ):
    
    def __init__( self,  digitizeLayer,  mapCanvas,  snapLayerId, snapTolerancePixels,  polygonMode ):
        QgsMapTool.__init__(self,  mapCanvas )
        self.mPolygonMode = polygonMode #True: polygon, False: line
        self.mRubberBand = QgsRubberBand( mapCanvas,  polygonMode )
        self.mLayerCoordList = []
        self.mMapCanvas = mapCanvas
        self.mDigitizeLayerId = digitizeLayer
        self.mEditLayer = QgsMapLayerRegistry.instance().mapLayer(  self.mDigitizeLayerId )
        
        #Configure QgsSnapper with 20 pixel to given snap layer
        snapLayer = QgsSnapper.SnapLayer()
        snapLayer.mTolerance = 20
        snapLayer.mUnitType = QgsTolerance.Pixels
        snapLayer.mSnapTo = QgsSnapper.SnapToVertexAndSegment
        snapLayer.mLayer = QgsMapLayerRegistry.instance().mapLayer(  snapLayerId )
        snapLayerList = []
        snapLayerList.append( snapLayer )
        self.mSnapper = QgsSnapper( mapCanvas.mapRenderer() )
        self.mSnapper.setSnapLayers( snapLayerList )
        self.mSnapper.setSnapMode( QgsSnapper.SnapWithOneResult )
        
        self.cursor = QCursor(QPixmap(["16 16 3 1",
                                  "      c None",
                                  ".     c #FF0000",
                                  "+     c #FFFFFF",
                                  "                ",
                                  "       +.+      ",
                                  "      ++.++     ",
                                  "     +.....+    ",
                                  "    +.     .+   ",
                                  "   +.   .   .+  ",
                                  "  +.    .    .+ ",
                                  " ++.    .    .++",
                                  " ... ...+... ...",
                                  " ++.    .    .++",
                                  "  +.    .    .+ ",
                                  "   +.   .   .+  ",
                                  "   ++.     .+   ",
                                  "    ++.....+    ",
                                  "      ++.++     ",
                                  "       +.+      "]))
        
    def activate( self ):
        QApplication.setOverrideCursor(self.cursor)
        if not self.mEditLayer is None:
            self.mEditLayer.startEditing()
        
    def deactivate( self ):
        self.mRubberBand.reset( self.mPolygonMode )
      #  QApplication.restoreOverrideCursor()
        
        if not self.mEditLayer is None:
            self.mEditLayer.commitChanges()
        
    def canvasMoveEvent(self,  event ):
        self.mRubberBand.movePoint( self.snappedPoint( event.pos() ) )
        
    def canvasReleaseEvent(self,  event ):
        point = self.snappedPoint( event.pos() )
        self.mLayerCoordList.append(  self.toLayerCoordinates(  self.mEditLayer,  point ) )
        self.mRubberBand.addPoint( point )
        if event.button() == Qt.RightButton:
            if self.mEditLayer is None:
                return
            feature = QgsFeature()
            feature.setGeometry( self.geometryFromPointList( self.mLayerCoordList ) )
            self.mEditLayer.beginEditCommand('Add feature')
            self.mEditLayer.addFeature( feature )
            self.mEditLayer.endEditCommand()
            self.mRubberBand.reset()
            del self.mLayerCoordList[:]
            self.mMapCanvas.refresh()
            
            
    def snappedPoint(self,  pos ):
        excludeList = []
        result = self.mSnapper.snapPoint(  pos,  excludeList )
        if result[0] == 0 and not len( result[1] ) < 1:
            resultPt = result[1][0].snappedVertex
            return QgsPoint( resultPt.x(),  resultPt.y() )
        else:
            return self.toMapCoordinates( pos )
            
    def geometryFromPointList(self,  pointList):
        if self.mPolygonMode:
            return QgsGeometry.fromPolygon(  [pointList] )
        else:
            return QgsGeometry.fromPolyline( pointList )
