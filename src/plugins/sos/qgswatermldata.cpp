/***************************************************************************
                          qgswatermldata.cpp  -  description
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

#include "qgswatermldata.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include <QApplication>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>

const char NS_SEPARATOR = '?';
const QString WML_NS = "http://www.opengis.net/waterml/2.0";
const QString timeString = WML_NS + NS_SEPARATOR + "time";
const QString valueString = WML_NS + NS_SEPARATOR + "value";
const QString measurementString = WML_NS + NS_SEPARATOR + "MeasurementTVP";

QgsWaterMLData::QgsWaterMLData(): mFinished( false ), mTimeVector( 0 ), mValueVector( 0 ), mParseMode( None )
{

}

QgsWaterMLData::~QgsWaterMLData()
{

}

int QgsWaterMLData::getData( const QString& url, QVector<double>* time, QVector<double>* value )
{
  if ( !time || !value )
  {
    return 1;
  }

  mTimeVector = time;
  mValueVector = value;

  XML_Parser p = XML_ParserCreateNS( NULL, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsWaterMLData::start, QgsWaterMLData::end );
  XML_SetCharacterDataHandler( p, QgsWaterMLData::chars );

  QNetworkRequest request( url );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, SIGNAL( finished() ), this, SLOT( setFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleProgressEvent( qint64, qint64 ) ) );

  //find out if there is a QGIS main window. If yes, display a progress dialog
  QProgressDialog* progressDialog = 0;
  QWidget* mainWindow = QApplication::activeWindow();
  if ( mainWindow )
  {
    progressDialog = new QProgressDialog( tr( "Loading data" ), tr( "Abort" ), 0, 0, mainWindow );
    progressDialog->setWindowModality( Qt::ApplicationModal );
    connect( this, SIGNAL( dataReadProgress( int ) ), progressDialog, SLOT( setValue( int ) ) );
    connect( this, SIGNAL( totalStepsUpdate( int ) )    , progressDialog, SLOT( setMaximum( int ) ) );
    connect( progressDialog, SIGNAL( canceled() ), this, SLOT( setFinished() ) );
    // connect( this, SIGNAL( progressMessage(QString) ), mainWindow, SLOT( showStatusMessage( QString ) ) );
    progressDialog->show();
  }

  int atEnd = 0;
  int totalData = 0;
  while ( !atEnd )
  {
    //sometimes, the network reply emits the finished signal even if something is still to come...
    if ( !totalData > 0 )
    {
      mFinished = false;
    }

    if ( mFinished )
    {
      atEnd = 1;
    }

    QByteArray readData = reply->readAll();

    //debug
    //qWarning( QString( readData ).toLocal8Bit().data() );

    totalData += readData.size();
    if ( readData.size() > 0 )
    {
      if ( XML_Parse( p, readData.constData(), readData.size(), atEnd ) == 0 )
      {
        XML_Error errorCode = XML_GetErrorCode( p );
        QString errorString = QObject::tr( "Error: %1 on line %2, column %3" )
                              .arg( XML_ErrorString( errorCode ) )
                              .arg( XML_GetCurrentLineNumber( p ) )
                              .arg( XML_GetCurrentColumnNumber( p ) );
        QgsMessageLog::instance()->logMessage( errorString, QObject::tr( "SOS" ) );
      }
    }

    QCoreApplication::processEvents();
  }
  delete reply;
  delete progressDialog;
  return 0;
}

void QgsWaterMLData::startElement( const XML_Char* el, const XML_Char** attr )
{
  if ( el == timeString )
  {
    mParseMode = Time;
    mStringCache.clear();
  }
  else if ( el == valueString )
  {
    mParseMode = Value;
    mStringCache.clear();
  }
  else
  {
    mParseMode = None;
  }
}

void QgsWaterMLData::endElement( const XML_Char* el )
{
  if ( el == timeString )
  {
    mCurrentDateTime = convertTimeString( mStringCache );
    mParseMode = None;
  }
  else if ( el == valueString )
  {
    mCurrentValue =  mStringCache.toDouble();
    mParseMode = None;
  }
  else if ( el ==  measurementString )
  {
    mTimeVector->append( mCurrentDateTime.toTime_t() );
    mValueVector->append( mCurrentValue );
  }
}

void QgsWaterMLData::characters( const XML_Char* chars, int len )
{
  if ( mParseMode == Time || mParseMode == Value )
  {
    mStringCache.append( QString::fromUtf8( chars, len ) );
  }
}

void QgsWaterMLData::setFinished()
{
  mFinished = true;
}

void QgsWaterMLData::handleProgressEvent( qint64 progress, qint64 maximum )
{

}

QDateTime QgsWaterMLData::convertTimeString( const QString& str )
{
  QStringList plusSplit = str.split( "+" );
  QDateTime time = QDateTime::fromString( plusSplit.at( 0 ), "yyyy'-'MM'-'dd'T'HH':'mm':'ss'.'z" );
  time.setTimeSpec( Qt::OffsetFromUTC );

  if ( plusSplit.size() > 1 )
  {
    //offset from UTC
  }

  qWarning( time.toString( "dd.MM.yyyy" ).toLocal8Bit().data() );
  qWarning( QString::number( time.toTime_t() ).toLocal8Bit().data() );
  return time;
}
