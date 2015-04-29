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
#include "qgsgeometryutils.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QVector2D>

static const int MinRadiusRole = Qt::UserRole + 1;


class CoordinateItemDelegate : public QStyledItemDelegate
{
  public:
    QString displayText( const QVariant & value, const QLocale & locale ) const
    {
      return locale.toString( value.toDouble(), 'f', 4 );
    }

  protected:
    QWidget* createEditor( QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const
    {
      QLineEdit* lineEdit = new QLineEdit( parent );
      QDoubleValidator* validator = new QDoubleValidator();
      if ( !index.data( MinRadiusRole ).isNull() )
        validator->setBottom( index.data( MinRadiusRole ).toDouble() );
      lineEdit->setValidator( validator );
      return lineEdit;
    }
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
    {
      QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor );
      if ( lineEdit->hasAcceptableInput() )
      {
        QStyledItemDelegate::setModelData( editor, model, index );
      }
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

  mTableWidget = new QTableWidget( 0, 6, this );
  mTableWidget->setHorizontalHeaderLabels( QStringList() << "id" << "x" << "y" << "z" << "m" << "r" );
  mTableWidget->setSelectionMode( QTableWidget::ExtendedSelection );
  mTableWidget->setSelectionBehavior( QTableWidget::SelectRows );
  mTableWidget->verticalHeader()->hide();
  mTableWidget->horizontalHeader()->setResizeMode( 1, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 2, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 3, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 4, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 5, QHeaderView::Stretch );
  mTableWidget->setItemDelegateForColumn( 1, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 2, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 3, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 4, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 5, new CoordinateItemDelegate() );

  setWidget( mTableWidget );

  connect( mSelectedFeature, SIGNAL( selectionChanged() ), this, SLOT( updateTableSelection() ) );
  connect( mSelectedFeature, SIGNAL( vertexMapChanged() ), this, SLOT( rebuildTable() ) );
  connect( mTableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( updateNodeSelection() ) );
  connect( mTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( tableValueChanged( int, int ) ) );

  rebuildTable();
}

void QgsNodeEditor::rebuildTable()
{
  QFont curvePointFont = mTableWidget->font();
  curvePointFont.setItalic( true );

  mTableWidget->blockSignals( true );
  mTableWidget->setRowCount( 0 );
  int row = 0;
  bool hasR = false;
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

    QTableWidgetItem* rItem = new QTableWidgetItem();
    mTableWidget->setItem( row, 5, rItem );

    bool curvePoint = ( entry->vertexId().type == QgsVertexId::CurveVertex );
    if ( curvePoint )
    {
      idItem->setFont( curvePointFont );
      xItem->setFont( curvePointFont );
      yItem->setFont( curvePointFont );
      zItem->setFont( curvePointFont );
      mItem->setFont( curvePointFont );
      rItem->setFont( curvePointFont );

      const QgsPointV2& p1 = mSelectedFeature->vertexMap()[row - 1]->point();
      const QgsPointV2& p2 = mSelectedFeature->vertexMap()[row]->point();
      const QgsPointV2& p3 = mSelectedFeature->vertexMap()[row + 1]->point();

      double r, cx, cy;
      QgsGeometryUtils::circleCenterRadius( p1, p2, p3, r, cx, cy );
      rItem->setData( Qt::EditRole, r );

      double x13 = p3.x() - p1.x(), y13 = p3.y() - p1.y();
      rItem->setData( MinRadiusRole, 0.5 * qSqrt( x13 * x13 + y13 * y13 ) );

      hasR = true;
    }
    else
    {
      rItem->setFlags( rItem->flags() & ~( Qt::ItemIsSelectable | Qt::ItemIsEnabled ) );
    }

    ++row;
  }
  mTableWidget->setColumnHidden( 3, !mSelectedFeature->vertexMap()[0]->point().is3D() );
  mTableWidget->setColumnHidden( 4, !mSelectedFeature->vertexMap()[0]->point().isMeasure() );
  mTableWidget->setColumnHidden( 5, !hasR );
  mTableWidget->resizeColumnToContents( 0 );
  mTableWidget->blockSignals( false );
}

void QgsNodeEditor::tableValueChanged( int row, int col )
{
  int nodeIdx = mTableWidget->item( row, 0 )->data( Qt::DisplayRole ).toInt();
  double x, y;
  if ( col == 5 ) // radius modified
  {
    double r = mTableWidget->item( row, 5 )->data( Qt::EditRole ).toDouble();
    double x1 = mTableWidget->item( row - 1, 1 )->data( Qt::EditRole ).toDouble();
    double y1 = mTableWidget->item( row - 1, 2 )->data( Qt::EditRole ).toDouble();
    double x2 = mTableWidget->item( row    , 1 )->data( Qt::EditRole ).toDouble();
    double y2 = mTableWidget->item( row    , 2 )->data( Qt::EditRole ).toDouble();
    double x3 = mTableWidget->item( row + 1, 1 )->data( Qt::EditRole ).toDouble();
    double y3 = mTableWidget->item( row + 1, 2 )->data( Qt::EditRole ).toDouble();

    // Determine old center and radius and sweep angles
    double oldR, oldCx, oldCy;
    QgsGeometryUtils::circleCenterRadius( QgsPointV2( x1, y1 ), QgsPointV2( x2, y2 ), QgsPointV2( x3, y3 ), oldR, oldCx, oldCy );

    // See https://bugreports.qt.io/browse/QTBUG-12515 about qAtan2...
    double oldA1 = qAtan2( y1 - oldCy, x1 - oldCx );
    if ( oldA1 < 0 ) oldA1 += 2 * M_PI;
    double oldA2 = qAtan2( y2 - oldCy, x2 - oldCx );
    if ( oldA2 < 0 ) oldA2 += 2 * M_PI;
    double oldA3 = qAtan2( y3 - oldCy, x3 - oldCx );
    if ( oldA3 < 0 ) oldA3 += 2 * M_PI;

    // Find possible centers of new arc
    /*           * C1 (C2 at mirrored position wrt P1-P3 line)
     *          /|\
     *         / | \
     *       r/ h|  \
     *       /   ^   \
     *      /    |e   \
     *  P1 *-----*-----* P3
     *        d  M
     */
    QVector2D P1( x1, y1 ), P2( x2, y2 ), P3( x3, y3 );
    QVector2D M = 0.5 * ( P1 + P3 );
    double d = 0.5 * ( P3 - P1 ).length();
    double h = qSqrt( r * r - d * d );
    QVector2D e = QVector2D( y3 - y1, x1 - x3 ).normalized();
    QVector2D C1 = M + h * e;
    QVector2D C2 = M - h * e;

    // Pick center on same side as before: depending on whether the sweep angle
    // between the arc endpoints is more or less than 180deg, the new center
    // needs to be on the same or opposite side than P2 WRT P1-P2
    double cross1 = ( P3.x() - P1.x() ) * ( C1.y() - P1.y() ) - ( P3.y() - P1.y() ) * ( C1.x() - P1.x() );
    double cross2 = ( P3.x() - P1.x() ) * ( P2.y() - P1.y() ) - ( P3.y() - P1.y() ) * ( P2.x() - P1.x() );
    QVector2D C;
    if ( qAbs( oldA3 - oldA1 ) < M_PI ) // opposite side
    {
      C = cross1 * cross2 < 0 ? C1 : C2;
    }
    else // same side
    {
      C = cross1 * cross2 > 0 ? C1 : C2;
    }

    // Place the third point in such way that it is proportionally on the same
    // spot as before
    double ratio = ( oldA2 - oldA1 ) / ( oldA3 - oldA1 );

    double newA1 = qAtan2( y1 - C.y(), x1 - C.x() );
    if ( newA1 < 0 ) newA1 += 2 * M_PI;
    double newA3 = qAtan2( y3 - C.y(), x3 - C.x() );
    if ( newA3 < 0 ) newA3 += 2 * M_PI;
    double newA2 = newA1 + ratio * ( newA3 - newA1 );

    x = C.x() + r * qCos( newA2 );
    y = C.y() + r * qSin( newA2 );
  }
  else
  {
    x = mTableWidget->item( row, 1 )->data( Qt::EditRole ).toDouble();
    y = mTableWidget->item( row, 2 )->data( Qt::EditRole ).toDouble();
  }
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
