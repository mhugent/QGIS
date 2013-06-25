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

#include <QDateTime>
#include <QObject>
#include <QVector>
#include <expat.h>

/**A class to read water ML data from a sensor observation service*/
class QgsWaterMLData: public QObject
{
    Q_OBJECT
  public:
    QgsWaterMLData();
    ~QgsWaterMLData();

    int getData( const QString& url, QVector<double>* time, QVector<double>* value );

    /**XML handler methods*/
    void startElement( const XML_Char* el, const XML_Char** attr );
    void endElement( const XML_Char* el );
    void characters( const XML_Char* chars, int len );
    static void start( void* data, const XML_Char* el, const XML_Char** attr )
    {
      static_cast<QgsWaterMLData*>( data )->startElement( el, attr );
    }
    static void end( void* data, const XML_Char* el )
    {
      static_cast<QgsWaterMLData*>( data )->endElement( el );
    }
    static void chars( void* data, const XML_Char* chars, int len )
    {
      static_cast<QgsWaterMLData*>( data )->characters( chars, len );
    }

  private:

    enum ParseMode
    {
      None,
      Time,
      Value,
    };


    bool mFinished;
    QVector<double>* mTimeVector;
    QVector<double>* mValueVector;
    ParseMode mParseMode;
    QDateTime mCurrentDateTime;
    double mCurrentValue;
    QString mStringCache;

  private slots:
    void setFinished();
    void handleProgressEvent( qint64 progress, qint64 maximum );
    QDateTime convertTimeString( const QString& str );
};

#endif // QGSWATERMLDATA_H
