#ifndef WEBDATAFILTERMODEL_H
#define WEBDATAFILTERMODEL_H

#include <QSortFilterProxyModel>

class WebDataFilterModel: public QSortFilterProxyModel
{
  public:
    WebDataFilterModel( QObject* parent = 0 );
    ~WebDataFilterModel();

    bool showOnlyFavourites() const { return mShowOnlyFavourites; }
    void setShowOnlyFavourites( bool b );

    /**Calls QSortFilterProxyModel::setFilterWildcard and triggers update*/
    void _setFilterWildcard( const QString& pattern );

  protected:
    bool mShowOnlyFavourites;

    bool filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const;
};

#endif // WEBDATAFILTERMODEL_H
