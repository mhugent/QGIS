/***************************************************************************
                          qgsxmldata.cpp  -  description
                          ------------------------------
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

#include "qgsxmldata.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include <QApplication>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>

QgsXMLData::QgsXMLData( const QString& url ): mUrl( url ), mFinished( false )
{
}

QgsXMLData::~QgsXMLData()
{
}

int QgsXMLData::getXMLData( QProgressDialog* progress )
{
  XML_Parser p = XML_ParserCreateNS( NULL, XML_NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsXMLData::start, QgsXMLData::end );
  XML_SetCharacterDataHandler( p, QgsXMLData::chars );

  QUrl url = QUrl::fromEncoded( mUrl.toLocal8Bit() ); //( mUrl );
  QNetworkRequest request( url );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, SIGNAL( finished() ), this, SLOT( setFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleProgressEvent( qint64, qint64 ) ) );

  if ( progress )
  {
    progress->setWindowModality( Qt::ApplicationModal );
    connect( this, SIGNAL( dataReadProgress( int ) ), progress, SLOT( setValue( int ) ) );
    connect( this, SIGNAL( totalStepsUpdate( int ) )    , progress, SLOT( setMaximum( int ) ) );
    connect( progress, SIGNAL( canceled() ), this, SLOT( setFinished() ) );
    progress->show();
  }

  //show loading progress in qgis main window (if it exists)
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

  if ( mainWindow )
  {
    QObject::connect( this, SIGNAL( progressMessage( QString ) ), mainWindow, SLOT( showStatusMessage( QString ) ) );
  }

  int atEnd = 0;
  while ( !atEnd )
  {
    if ( mFinished )
    {
      atEnd = 1;
    }

    QByteArray readData = reply->readAll();

    //debug
    if ( readData.size() > 0 )
    {
      qWarning( QString( readData ).toLocal8Bit().data() );
    }

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

  if ( mainWindow )
  {
    QObject::disconnect( this, SIGNAL( progressMessage( QString ) ), mainWindow, SLOT( showStatusMessage( QString ) ) );
  }

  delete reply;
  if ( progress )
  {
    progress->hide();
  }

  return 0;
}

void QgsXMLData::setFinished()
{
  mFinished = true;
}

void QgsXMLData::handleProgressEvent( qint64 progress, qint64 maximum )
{
  emit dataReadProgress( progress );
  emit totalStepsUpdate( maximum );
  emit progressMessage( QString( "Received %1 bytes from %2" ).arg( progress ).arg( maximum ) );
}
