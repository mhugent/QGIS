/***************************************************************************
                          qgssosdata.cpp  -  description
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

#include "qgssosdata.h"
#include "qgsgeometry.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsrectangle.h"
#include <QApplication>
#include <QCoreApplication>
#include <QNetworkReply>
#include <QObject>
#include <QProgressDialog>

const char NS_SEPARATOR = '?';
const QString GML_NS = "http://www.opengis.net/gml/3.2";
const QString SOS_NS =  "http://www.opengis.net/sos/2.0";
const QString SAMPLING_NS = "http://www.opengis.net/sampling/2.0";
const QString identifierElementString = GML_NS + NS_SEPARATOR + "identifier";
const QString nameElementString = GML_NS + NS_SEPARATOR + "name";
const QString posElementString = GML_NS + NS_SEPARATOR + "pos";
const QString featureElemString = SOS_NS + NS_SEPARATOR + "featureMember";

QgsSOSData::QgsSOSData(): mFeatures( 0 ), mCrs( 0 ), mExtent( 0 ), mCurrentFeature( 0 ), mParseMode( None ), mFinished( false )
{

}

QgsSOSData::~QgsSOSData()
{

}

int QgsSOSData::getFeatures( const QString& uri, QMap<QgsFeatureId, QgsFeature* >* features,
                             QgsCoordinateReferenceSystem* crs, QgsRectangle* extent )
{
  if ( !features || !crs || !extent )
  {
    return 1;
  }
  mFeatures = features;
  mFeatures->clear();
  mCrs = crs;
  mExtent = extent;
  mExtent->setMinimal();

  XML_Parser p = XML_ParserCreateNS( NULL, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsSOSData::start, QgsSOSData::end );
  XML_SetCharacterDataHandler( p, QgsSOSData::chars );

  QNetworkRequest request( uri );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, SIGNAL( finished() ), this, SLOT( setFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleProgressEvent( qint64, qint64 ) ) );

  //find out if there is a QGIS main window. If yes, display a progress dialog
  QProgressDialog* progressDialog = 0;
  QWidget* mainWindow = QApplication::activeWindow();
  if ( mainWindow )
  {
    progressDialog = new QProgressDialog( tr( "Loading SOS data" ), tr( "Abort" ), 0, 0, mainWindow );
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

void QgsSOSData::handleProgressEvent( qint64 progress, qint64 maximum )
{
  emit dataReadProgress( progress );
  emit totalStepsUpdate( maximum );
  emit progressMessage( QString( "Received %1 bytes from %2" ).arg( progress ).arg( maximum ) );
}

void QgsSOSData::startElement( const XML_Char* el, const XML_Char** attr )
{
  Q_UNUSED( attr );
  QString elementName( el );
  if ( el == featureElemString )
  {
    delete mCurrentFeature;
    mCurrentFeature = new QgsFeature( mFeatures->size() );
    mCurrentFeature->initAttributes( 2 );
  }
  else if ( el == identifierElementString )
  {
    mParseMode = Identifier;
  }
  else if ( el == nameElementString )
  {
    mParseMode = Name;
  }
  else if ( el ==  posElementString )
  {
    mParseMode = Pos;
  }
  else
  {
    mParseMode = None;
  }
}

void QgsSOSData::endElement( const XML_Char* el )
{
  if ( !mCurrentFeature )
  {
    return;
  }

  if ( el == featureElemString )
  {
    if ( mCurrentFeature )
    {
      mFeatures->insert( mCurrentFeature->id(), mCurrentFeature );
      mCurrentFeature = 0;
    }
  }

  if ( el == identifierElementString )
  {
    mParseMode = None;
    mCurrentFeature->setAttribute( 0, mStringCache );
    mStringCache.clear();
  }
  else if ( el == nameElementString )
  {
    mParseMode = None;
    mCurrentFeature->setAttribute( 1, mStringCache );
    mStringCache.clear();
  }
  else if ( el == posElementString )
  {
    mParseMode = None;
    QStringList coordList = QString( mStringCache ).split( " " );
    if ( coordList.size() > 1 )
    {
      double x = coordList.at( 0 ).toDouble();
      double y = coordList.at( 1 ).toDouble();
      mCurrentFeature->setGeometry( QgsGeometry::fromPoint( QgsPoint( x, y ) ) );
      if ( mExtent )
      {
        mExtent->combineExtentWith( x, y );
      }
    }
    mStringCache.clear();
  }
}

void QgsSOSData::characters( const XML_Char* chars, int len )
{
  if ( !mCurrentFeature )
  {
    return;
  }

  if ( mParseMode == Identifier || mParseMode == Name || mParseMode == Pos )
  {
    mStringCache.append( QString::fromUtf8( chars, len ) );
  }
}

void QgsSOSData::setFinished()
{
  mFinished = true;
}
