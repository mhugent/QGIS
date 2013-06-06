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

#include <QObject>
#include <QString>

class QgsSOSCapabilities: public QObject
{
    Q_OBJECT
  public:
    QgsSOSCapabilities( const QString& serviceUrl );
    ~QgsSOSCapabilities();

    void requestCapabilities();

  private:
    QString mUrl;

  private slots:
    void capabilitiesReplyFinished();

  signals:
    void gotCapabilities();
};

#endif // QGSSOSCAPABILITIES_H
