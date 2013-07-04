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

QgsWaterMLData::QgsWaterMLData( const QString& url ): QgsXMLData( url ), mParseMode( None )
{

}

QgsWaterMLData::~QgsWaterMLData()
{

}

int QgsWaterMLData::getData( QMap< QDateTime, double >* timeValueMap )
{
  if ( !timeValueMap )
  {
    return 2;
  }
  mTimeValueMap = timeValueMap;
  mTimeValueMap->clear();

  QWidget* mainWindow = 0;
  QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  for ( QWidgetList::iterator it = topLevelWidgets.begin(); it != topLevelWidgets.end(); ++it )
  {
    if (( *it )->objectName() == "QgisApp" )
    {
      mainWindow = *it;
      break;
    }
  }

  QProgressDialog* progressDialog = 0;
  if ( mainWindow )
  {
    progressDialog = new QProgressDialog( tr( "Loading sensor data" ), tr( "Abort" ), 0, 0, mainWindow );
    connect( this, SIGNAL( dataReadProgress( int ) ), progressDialog, SLOT( setValue( int ) ) );
    connect( this, SIGNAL( totalStepsUpdate( int ) )    , progressDialog, SLOT( setMaximum( int ) ) );
    connect( progressDialog, SIGNAL( canceled() ), this, SLOT( setFinished() ) );
  }

  return getXMLData( progressDialog );
  delete progressDialog;
  return 0;
}

void QgsWaterMLData::startElement( const XML_Char* el, const XML_Char** attr )
{
  Q_UNUSED( attr );
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
    mTimeValueMap->insert( mCurrentDateTime, mCurrentValue );
  }
}

void QgsWaterMLData::characters( const XML_Char* chars, int len )
{
  if ( mParseMode == Time || mParseMode == Value )
  {
    mStringCache.append( QString::fromUtf8( chars, len ) );
  }
}

QDateTime QgsWaterMLData::convertTimeString( const QString& str )
{
  //format yyyy-mm
  QStringList yearMonthSplit = str.split( "-" );

  int day = 1;
  int month = 1;
  int year = 1;

  if ( yearMonthSplit.size() > 0 )
  {
    year = yearMonthSplit.at( 0 ).toInt();
  }
  if ( yearMonthSplit.size() > 1 )
  {
    month = yearMonthSplit.at( 1 ).toInt();
  }
  if ( yearMonthSplit.size() > 2 )
  {
    day = yearMonthSplit.at( 2 ).toInt();
  }

  QDate date;
  date.setDate( year, month, day );
  QDateTime dateTime( date );

  //debug
  QString debug = dateTime.toString();
  int time = dateTime.toTime_t();

  return dateTime;

#if 0
  QStringList plusSplit = str.split( "+" );
  QDateTime time = QDateTime::fromString( plusSplit.at( 0 ), "yyyy'-'MM'-'dd'T'HH':'mm':'ss'.'z" );
  time.setTimeSpec( Qt::OffsetFromUTC );

  if ( plusSplit.size() > 1 )
  {
    //offset from UTC
  }

  //qWarning( time.toString( "dd.MM.yyyy" ).toLocal8Bit().data() );
  //qWarning( QString::number( time.toTime_t() ).toLocal8Bit().data() );
  return time;
#endif //0

}
