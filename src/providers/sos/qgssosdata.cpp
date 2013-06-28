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

const QString GML_NS = "http://www.opengis.net/gml/3.2";
const QString SOS_NS =  "http://www.opengis.net/sos/2.0";
const QString SAMPLING_NS = "http://www.opengis.net/sampling/2.0";
const QString identifierElementString = GML_NS + XML_NS_SEPARATOR + "identifier";
const QString nameElementString = GML_NS + XML_NS_SEPARATOR + "name";
const QString posElementString = GML_NS + XML_NS_SEPARATOR + "pos";
const QString featureElemString = SOS_NS + XML_NS_SEPARATOR + "featureMember";

QgsSOSData::QgsSOSData( const QString& uri ): QgsXMLData( uri ), mFeatures( 0 ), mCrs( 0 ), mExtent( 0 ), mCurrentFeature( 0 ), mParseMode( None ), mFinished( false )
{

}

QgsSOSData::~QgsSOSData()
{

}

int QgsSOSData::getFeatures( QMap<QgsFeatureId, QgsFeature* >* features,
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

  int val = getXMLData( progressDialog );
  delete progressDialog;
  return val;
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
