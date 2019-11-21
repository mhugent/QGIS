/***************************************************************************
 *  vectoranalysis.h                                                               *
 *  -------------------                                                    *
 *  begin                : Jun 10, 2014                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VECTORANALYSIS_H
#define VECTORANALYSIS_H

#include "qgisplugin.h"
#include "qgsvectorlayer.h"
#include "qgscoordinatereferencesystem.h"

class QgisInterface;
class QMenu;

class VectorAnalysis: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    VectorAnalysis( QgisInterface * theInterface );

    void initGui();
    void unload();

  private:
    QgisInterface* mQGisIface;
    QMenu* mGeoprocessingMenu;
    QList<QAction*> mActions;
    QDialog* mCurrDialog;

    QIcon getPluginIcon( const QString& name );

  private slots:
    void runBufferTool();
    void runClipTool();
    void runConvexhullTool();
    void runDifferenceTool();
    void runDissolveTool();
    void runIntersectionTool();
    void runSliverPolygonTool();
    void runSymDifferenceTool();
    void runUnionTool();
};

#endif //VECTORANALYSIS_H
