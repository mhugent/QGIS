#include "webdatafiltermodel.h"
#include <QStandardItemModel>

WebDataFilterModel::WebDataFilterModel( QObject* parent ): QSortFilterProxyModel( parent ), mShowOnlyFavourites( false )
{
}

WebDataFilterModel::~WebDataFilterModel()
{
}

void WebDataFilterModel::setShowOnlyFavourites( bool b )
{
  mShowOnlyFavourites = b;
  invalidateFilter();
}

void WebDataFilterModel::_setFilterWildcard( const QString& pattern )
{
  QSortFilterProxyModel::setFilterWildcard( pattern );
  invalidateFilter();
}

bool WebDataFilterModel::filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const
{
  //if parent is valid, we have a toplevel item that should be always shown
  if ( !source_parent.isValid() )
  {
    return true;
  }

  if ( mShowOnlyFavourites )
  {
    QStandardItemModel* model = dynamic_cast<QStandardItemModel*>( sourceModel() );
    if ( model )
    {
      QStandardItem* item = model->itemFromIndex( model->index( source_row, 1, source_parent ) );
      {
        if ( item && item->checkState() != Qt::Checked )
        {
          return false;
        }
      }
    }

  }

  //else we have a row that describes a table and that
  //should be tested using the given wildcard/regexp
  return QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );
}
