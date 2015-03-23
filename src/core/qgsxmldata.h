/***************************************************************************
                          qgsxmldata.h  -  description
                          ----------------------------
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

#ifndef QGSXMLDATA_H
#define QGSXMLDATA_H

#include <expat.h>
#include <QObject>
#include <QString>

const char XML_NS_SEPARATOR = '?';

class QProgressDialog;

/**Base class to download and parse xml data. Handles network request, streaming, error and progress handling*/
class CORE_EXPORT QgsXMLData: public QObject
{
    Q_OBJECT
  public:
    QgsXMLData( const QString& url );
    virtual ~QgsXMLData();

  protected:
    int getXMLData( QProgressDialog* progress );

    //derived classes need to implement xml event methods
    virtual void startElement( const XML_Char* el, const XML_Char** attr ) = 0;
    virtual void endElement( const XML_Char* el ) = 0;
    virtual void characters( const XML_Char* chars, int len ) = 0;

    QString mStringCache;

  private:
    static void start( void* data, const XML_Char* el, const XML_Char** attr )
    {
      static_cast<QgsXMLData*>( data )->startElement( el, attr );
    }
    static void end( void* data, const XML_Char* el )
    {
      static_cast<QgsXMLData*>( data )->endElement( el );
    }
    static void chars( void* data, const XML_Char* chars, int len )
    {
      static_cast<QgsXMLData*>( data )->characters( chars, len );
    }

    QString mUrl;
    bool mFinished;

  private slots:
    void setFinished();
    void handleProgressEvent( qint64 progress, qint64 maximum );

  signals:
    void dataReadProgress( int );
    void totalStepsUpdate( int );
    void progressMessage( QString );
};

#endif // QGSXMLDATA_H
