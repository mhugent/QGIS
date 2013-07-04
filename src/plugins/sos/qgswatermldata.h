/***************************************************************************
                          qgswatermldata.h  -  description
                          --------------------------------
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

#ifndef QGSWATERMLDATA_H
#define QGSWATERMLDATA_H

#include "qgsxmldata.h"
#include <QDateTime>
#include <QMap>
#include <QObject>
#include <expat.h>

/**A class to read water ML data from a sensor observation service*/
class QgsWaterMLData: public QgsXMLData
{
  public:
    QgsWaterMLData( const QString& url );
    ~QgsWaterMLData();

    int getData( QMap< QDateTime, double >* timeValueMap );

    /**XML handler methods*/
    void startElement( const XML_Char* el, const XML_Char** attr );
    void endElement( const XML_Char* el );
    void characters( const XML_Char* chars, int len );

  private:

    enum ParseMode
    {
      None,
      Time,
      Value,
    };

    QMap< QDateTime, double >* mTimeValueMap;
    ParseMode mParseMode;
    QDateTime mCurrentDateTime;
    double mCurrentValue;

    QDateTime convertTimeString( const QString& str );
};

#endif // QGSWATERMLDATA_H
