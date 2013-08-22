/***************************************************************************
                          qgssoscapabilities.h  -  description
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

#ifndef QGSSOSCAPABILITIES_H
#define QGSSOSCAPABILITIES_H

#include "qgsdatasourceuri.h"

#include <QObject>
#include <QString>
#include <QStringList>

class QNetworkReply;

class QgsSOSCapabilities: public QObject
{
    Q_OBJECT
  public:
    QgsSOSCapabilities( const QString& serviceUrl );
    ~QgsSOSCapabilities();

    void requestCapabilities();


    const QStringList* observableProperties() const { return &mObservableProperties; }
    const QString& networkError() const { return mNetworkError; }
    const QString& xmlError() const { return mXmlError; }

  private:
    QgsDataSourceURI mUrl;
    QString mBaseUrl;
    QNetworkReply* mCapabilitiesReply;
    QString mNetworkError;
    QString mXmlError;
    QStringList mObservableProperties;

    QString uriGetCapabilities();
    //! Append ? or & if necessary
    QString prepareUri( QString uri );

  private slots:
    void capabilitiesReplyFinished();

  signals:
    void gotCapabilities();
};

#endif // QGSSOSCAPABILITIES_H
