/***************************************************************************
                          qgssosfeatureiterator.h  -  description
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

#ifndef QGSSOSFEATUREITERATOR_H
#define QGSSOSFEATUREITERATOR_H

#include"qgsfeatureiterator.h"

class QgsSOSProvider;

class QgsSOSFeatureIterator: public QgsAbstractFeatureIterator
{
  public:
    QgsSOSFeatureIterator( QgsSOSProvider* provider, const QgsFeatureRequest& request );
    ~QgsSOSFeatureIterator();

    //! fetch next feature, return true on success
    bool nextFeature( QgsFeature& f );
    //! reset the iterator to the starting position
    bool rewind();
    //! end of iterating: free the resources / lock
    bool close();

  private:
    QgsSOSProvider* mProvider;
    QList<QgsFeatureId> mSelectedFeatures;
    QList<QgsFeatureId>::const_iterator mFeatureIterator;
};

#endif // QGSSOSFEATUREITERATOR_H
