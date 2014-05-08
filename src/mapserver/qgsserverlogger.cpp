/***************************************************************************
                              qgsserverlogger.cpp
                              -------------------
  begin                : May 5, 2014
  copyright            : (C) 2014 by Marco Hugentobler
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

#include "qgsserverlogger.h"
#include <QFile>
#include <QTextStream>
#include <QTime>

QgsServerLogger* QgsServerLogger::mInstance = 0;

QgsServerLogger* QgsServerLogger::instance()
{
    if ( mInstance == 0 )
    {
      mInstance = new QgsServerLogger();
    }
    return mInstance;
}

QgsServerLogger::QgsServerLogger()
{
    mLogFile = getenv( "QGIS_LOG_FILE" );
    char* logLevelChar = getenv( "QGIS_LOG_LEVEL" );
    if( logLevelChar )
    {
        mLogLevel = atoi( logLevelChar );
    }
    else
    {
        mLogLevel = 0;
    }
}

void QgsServerLogger::logMessage( const QString& message, int logLevel )
{
    if ( !mLogFile.isEmpty() && logLevel <= mLogLevel )
    {
      QFile file( mLogFile );
      file.open( QIODevice::Append );  
      QTextStream stream( &file );
      stream << ( "[" + QTime::currentTime().toString() + "] " + message + "\n" );
    }
    return;
}
