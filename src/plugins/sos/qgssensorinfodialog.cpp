/***************************************************************************
                          qgssensorinfodialog.cpp  -  description
                          ---------------------------------------
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

#include "qgssensorinfodialog.h"
#include <QDateTimeEdit>
#include <QPushButton>

QgsSensorInfoDialog::QgsSensorInfoDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  QPushButton* displayButton = new QPushButton( tr( "Display" ), this );
  connect( displayButton, SIGNAL( clicked() ), this, SLOT( showDiagram() ) );
  mButtonBox->addButton( displayButton, QDialogButtonBox::ActionRole );
}

QgsSensorInfoDialog::~QgsSensorInfoDialog()
{

}

void QgsSensorInfoDialog::clearObservables()
{
  mObservableTreeWidget->clear();
}

void QgsSensorInfoDialog::addObservables( const QString stationId, const QStringList& observables, const QList< QDateTime >& begin, const QList< QDateTime >& end )
{
  QTreeWidgetItem* stationIdWidget = new QTreeWidgetItem( QStringList() << stationId );
  mObservableTreeWidget->addTopLevelItem( stationIdWidget );

  QStringList::const_iterator obsIt = observables.constBegin();
  QList< QDateTime >::const_iterator bIt = begin.constBegin();
  QList< QDateTime >::const_iterator eIt = end.constBegin();
  for ( ; obsIt != observables.constEnd() && bIt != begin.constEnd() && eIt != end.constEnd(); ++obsIt )
  {
    QTreeWidgetItem* observableItem = new QTreeWidgetItem( stationIdWidget, QStringList() << "" << *obsIt );
    mObservableTreeWidget->setItemWidget( observableItem, 2, new QDateTimeEdit( *bIt ) );
    mObservableTreeWidget->setItemWidget( observableItem, 3, new QDateTimeEdit( *eIt ) );
  }
  mObservableTreeWidget->expandAll();
}

void QgsSensorInfoDialog::showDiagram()
{
  //todo...
}
