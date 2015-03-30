/***************************************************************************
                               qgsnodeeditor.cpp
                               -----------------
        begin                : Tue Mar 24 2015
        copyright            : (C) 2015 Sandro Mani / Sourcepole AG
        email                : smani@sourcepole.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnodeeditor.h"
#include "qgsmapcanvas.h"
#include "qgsselectedfeature.h"
#include "qgsvertexentry.h"
#include "qgsvectorlayer.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStyledItemDelegate>
#include <QLineEdit>


class CoordinateItemDelegate : public QStyledItemDelegate
{
  public:
    QString displayText( const QVariant & value, const QLocale & locale ) const
    {
      return locale.toString( value.toDouble(), 'f', 4 );
    }

  protected:
    QWidget* createEditor( QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/ ) const
    {
      QLineEdit* lineEdit = new QLineEdit( parent );
      lineEdit->setValidator( new QDoubleValidator );
      return lineEdit;
    }
};


QgsNodeEditor::QgsNodeEditor(
  QgsVectorLayer *layer,
  QgsSelectedFeature *selectedFeature,
  QgsMapCanvas *canvas )
{
  setWindowTitle( tr( "Vertex editor" ) );
  setFeatures( features() ^ QDockWidget::DockWidgetClosable );

  mLayer = layer;
  mSelectedFeature = selectedFeature;
  mCanvas = canvas;

  mTableWidget = new QTableWidget( 0, 5, this );
  mTableWidget->setHorizontalHeaderLabels( QStringList() << "id" << "x" << "y" << "z" << "m" );
  mTableWidget->setSelectionMode( QTableWidget::ExtendedSelection );
  mTableWidget->setSelectionBehavior( QTableWidget::SelectRows );
  mTableWidget->verticalHeader()->hide();
  mTableWidget->horizontalHeader()->setResizeMode( 1, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 2, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 3, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 4, QHeaderView::Stretch );
  mTableWidget->setItemDelegateForColumn( 1, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 2, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 3, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 4, new CoordinateItemDelegate() );

  setWidget( mTableWidget );

  connect( mSelectedFeature, SIGNAL( selectionChanged() ), this, SLOT( updateTableSelection() ) );
  connect( mSelectedFeature, SIGNAL( vertexMapChanged() ), this, SLOT( rebuildTable() ) );
  connect( mTableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( updateNodeSelection() ) );
  connect( mTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( tableValueChanged( int, int ) ) );

  rebuildTable();
}

void QgsNodeEditor::rebuildTable()
{
  mTableWidget->blockSignals( true );
  mTableWidget->setRowCount( 0 );
  int row = 0;
  foreach ( const QgsVertexEntry* entry, mSelectedFeature->vertexMap() )
  {
    mTableWidget->insertRow( row );

    QTableWidgetItem* idItem = new QTableWidgetItem();
    idItem->setData( Qt::DisplayRole, row );
    idItem->setFlags( idItem->flags() ^ Qt::ItemIsEditable );
    mTableWidget->setItem( row, 0, idItem );

    QTableWidgetItem* xItem = new QTableWidgetItem();
    xItem->setData( Qt::EditRole, entry->point().x() );
    mTableWidget->setItem( row, 1, xItem );

    QTableWidgetItem* yItem = new QTableWidgetItem();
    yItem->setData( Qt::EditRole, entry->point().y() );
    mTableWidget->setItem( row, 2, yItem );

    QTableWidgetItem* zItem = new QTableWidgetItem();
    zItem->setData( Qt::EditRole, entry->point().z() );
    mTableWidget->setItem( row, 3, zItem );

    QTableWidgetItem* mItem = new QTableWidgetItem();
    mItem->setData( Qt::EditRole, entry->point().m() );
    mTableWidget->setItem( row, 4, mItem );

    ++row;
  }
  mTableWidget->setRowHidden( row - 1, true ); // Hide last row since it is the same as the first one
  mTableWidget->setColumnHidden( 3, !mSelectedFeature->vertexMap()[0]->point().is3D() );
  mTableWidget->setColumnHidden( 4, !mSelectedFeature->vertexMap()[0]->point().isMeasure() );
  mTableWidget->resizeColumnToContents( 0 );
  mTableWidget->blockSignals( false );
}

void QgsNodeEditor::tableValueChanged( int row, int /*col*/ )
{
  int nodeIdx = mTableWidget->item( row, 0 )->data( Qt::DisplayRole ).toInt();
  double x = mTableWidget->item( row, 1 )->data( Qt::EditRole ).toDouble();
  double y = mTableWidget->item( row, 2 )->data( Qt::EditRole ).toDouble();
  double z = mTableWidget->item( row, 3 )->data( Qt::EditRole ).toDouble();
  double m = mTableWidget->item( row, 4 )->data( Qt::EditRole ).toDouble();
  QgsPointV2 p( QgsWKBTypes::PointZM, x, y, z, m );

  mLayer->beginEditCommand( QObject::tr( "Moved vertices" ) );
  mLayer->moveVertex( p, mSelectedFeature->featureId(), nodeIdx );
  mLayer->endEditCommand();
  mCanvas->refresh();
}

void QgsNodeEditor::updateTableSelection()
{
  mTableWidget->blockSignals( true );
  mTableWidget->clearSelection();
  const QList<QgsVertexEntry*>& vertexMap = mSelectedFeature->vertexMap();
  for ( int i = 0, n = vertexMap.size(); i < n; ++i )
  {
    if ( vertexMap[i]->isSelected() )
    {
      mTableWidget->selectRow( i );
    }
  }
  mTableWidget->blockSignals( false );
}

void QgsNodeEditor::updateNodeSelection()
{
  mSelectedFeature->blockSignals( true );
  mSelectedFeature->deselectAllVertexes();
  foreach ( const QModelIndex& index, mTableWidget->selectionModel()->selectedRows() )
  {
    int nodeIdx = mTableWidget->item( index.row(), 0 )->data( Qt::DisplayRole ).toInt();
    mSelectedFeature->selectVertex( nodeIdx );
  }
  mSelectedFeature->blockSignals( false );
}
