/***************************************************************************
    qgsmaptoolnodetool.h  - add/move/delete vertex integrated in one tool
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLNODETOOL_H
#define QGSMAPTOOLNODETOOL_H

#include "qgsmaptoolvertexedit.h"

class QRubberBand;

class QgsGeometryRubberBand;
class QgsVertexEntry;
class QgsSelectedFeature;
class QgsNodeEditor;

/**A maptool to move/deletes/adds vertices of line or polygon features*/
class QgsMapToolNodeTool: public QgsMapToolVertexEdit
{
    Q_OBJECT
  public:
    QgsMapToolNodeTool( QgsMapCanvas* canvas );
    virtual ~QgsMapToolNodeTool();

    void canvasMoveEvent( QMouseEvent * e );

    void canvasDoubleClickEvent( QMouseEvent * e );

    void canvasPressEvent( QMouseEvent * e );

    void canvasReleaseEvent( QMouseEvent * e );

    void keyPressEvent( QKeyEvent* e );

    //! called when map tool is being deactivated
    void deactivate();

  public slots:
    void selectedFeatureDestroyed();

    /*
     * the current layer changed
     */
    void currentLayerChanged( QgsMapLayer *layer );

    /*
     * the current edition state changed
     */
    void editingToggled();

  private:
    /**
     * Deletes the rubber band pointers and clears mRubberBands
     */
    void removeRubberBands();

    /**
     * Disconnects signals and clears objects
     */
    void cleanTool( bool deleteSelectedFeature = true );

    /**
     * Function to check if selected feature exists and is same with original one
     * stored in internal structures
     * @param vlayer vector layer for checking
     * @return if feature is same as one in internal structures
     */
    bool checkCorrectnessOfFeature( QgsVectorLayer* vlayer );

    /**
     * Creates rubber bands for ther features when topology editing is enabled
     */
    void createTopologyRubberBands();

    /**
     * Returns the index of first selected vertex, -1 when all unselected
     */
    int firstSelectedVertex();

    /**
     * Select the specified vertex bounded to current index range, returns the valid selected index
     */
    void safeSelectVertex( int vertexNr );

    /** rubber bands during node move */
    QMap<QgsFeatureId, QgsGeometryRubberBand*> mMoveRubberBands;

    /** vertices of features to move */
    QMap<QgsFeatureId, QList< QPair<QgsVertexId, QgsPointV2> > > mMoveVertices;

    /** object containing selected feature and it's vertexes */
    QgsSelectedFeature *mSelectedFeature;

    /** dock widget which allows to edit vertices */
    QgsNodeEditor* mNodeEditor;

    /** flag if moving of vertexes is occuring */
    bool mMoving;

    /** flag if selection of another feature can occur */
    bool mSelectAnother;

    /** feature id of another feature where user clicked */
    QgsFeatureId mAnother;

    /** stored position of last press down action to count how much vertexes should be moved */
    QPoint mPressCoordinates;

    /** closest vertex to click in map coordinates */
    QgsPoint mClosestMapVertex;

    /** active rubberband for selecting vertexes */
    QRubberBand *mSelectionRubberBand;

    /** rectangle defining area for selecting vertexes */
    QRect* mRect;

    /** flag to tell if edition points */
    bool mIsPoint;

    /** vertex to deselect on release */
    int mDeselectOnRelease;
};

#endif
