/***************************************************************************
                          qgssoscapabilities.cpp  -  description
                          --------------------------------------
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

#include "qgssoscapabilities.h"
#include "qgsnetworkaccessmanager.h"
#include <QDomDocument>
#include <QNetworkReply>

QgsSOSCapabilities::QgsSOSCapabilities( const QString& serviceUrl ): QObject( 0 ), mCapabilitiesReply( 0 )
{
  mUrl.setEncodedUri( serviceUrl );
  mBaseUrl = prepareUri( mUrl.param( "url" ) );
}

QgsSOSCapabilities::~QgsSOSCapabilities()
{
  delete mCapabilitiesReply;
}

void QgsSOSCapabilities::requestCapabilities()
{
  QString url = uriGetCapabilities();
  qWarning( url.toLocal8Bit().data() );
  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
  QObject::connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
}

void QgsSOSCapabilities::capabilitiesReplyFinished()
{
  QNetworkReply* reply = mCapabilitiesReply;
  QSet<QString> observedPropertiesSet;

  mObservableProperties.clear();
  mNetworkError.clear();
  mXmlError.clear();
  reply->deleteLater();
  mCapabilitiesReply = 0;

  // handle network errors
  if ( reply->error() != QNetworkReply::NoError )
  {
    mNetworkError = reply->errorString();
    emit gotCapabilities();
    return;
  }

  QByteArray buffer = reply->readAll();
  QDomDocument doc;

  QString xmlErrorMsg;
  int errorLine = 0;
  int errorColumn = 0;
  if ( !doc.setContent( buffer, true, &xmlErrorMsg, &errorLine, &errorColumn ) )
  {
    mXmlError = QString( "XML error: %1 on line %2, column %3" ).arg( xmlErrorMsg ).arg( errorLine ).arg( errorColumn );
    return;
  }

  //get list of observable properties
  QDomNodeList observablePropertyList = doc.elementsByTagNameNS( "http://www.opengis.net/swes/2.0" , "observableProperty" );
  for ( int i = 0; i < observablePropertyList.size(); ++i )
  {
    observedPropertiesSet.insert( observablePropertyList.at( i ).toElement().text() );
  }

  QSet<QString>::const_iterator propIt = observedPropertiesSet.constBegin();
  for ( ; propIt != observedPropertiesSet.constEnd(); ++propIt )
  {
    mObservableProperties.push_back( *propIt );
  }

  emit gotCapabilities();
}

QString QgsSOSCapabilities::uriGetCapabilities()
{
  return ( mBaseUrl + "SERVICE=SOS&REQUEST=GetCapabilities&DATASOURCE=0&VERSION=2.0.0" ); //leave out DATASOURCE and VERSION parameters?
}

QString QgsSOSCapabilities::prepareUri( QString uri )
{
  if ( !uri.contains( "?" ) )
  {
    uri.append( "?" );
  }
  else if ( uri.right( 1 ) != "?" && uri.right( 1 ) != "&" )
  {
    uri.append( "&" );
  }

  return uri;
}
