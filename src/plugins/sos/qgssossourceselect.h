/***************************************************************************
                          qgssossourceselect.h  -  description
                          ------------------------------------
    begin                : June 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**Dialog to connect to sensor observation services and to load layers*/

#include "ui_qgssossourceselectbase.h"

class QgsSOSCapabilities;
class QgisInterface;

class QgsSOSSourceSelect: public QDialog, private Ui::QgsSOSSourceSelectBase
{
    Q_OBJECT
  public:
    QgsSOSSourceSelect( QgisInterface* iface, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsSOSSourceSelect();

  private slots:
    void on_mConnectButton_clicked();
    void on_mNewButton_clicked();
    void on_mEditButton_clicked();
    void on_mDeleteButton_clicked();
    void gotCapabilities();
    void addLayer();

  private:
    QgsSOSCapabilities* mCapabilities;
    QgisInterface* mIface;

    void populateConnectionList();
};
