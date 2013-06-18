/***************************************************************************
                          qgssosdata.h  -  description
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

#ifndef QGSSOSDATA_H
#define QGSSOSDATA_H

#include "qgsfeature.h"
#include <QMap>
#include <QObject>
#include <QString>
#include <expat.h>

class QgsCoordinateReferenceSystem;
class QgsRectangle;

class QgsSOSData: public QObject
{
    Q_OBJECT
    public:
        QgsSOSData();
        ~QgsSOSData();
        /**Load features into map
            @param uri  service url (GetFeatureOfInterest)
            @param features sensor points are added to the feature map*/
        int getFeatures( const QString& uri, QMap<QgsFeatureId, QgsFeature* >* features,
                         QgsCoordinateReferenceSystem* crs, QgsRectangle* extent );

    private slots:
        void setFinished();

    private:
        enum ParseMode
        {
            None,
            Identifier,
            Name,
            Pos
        };

        /**XML handler methods*/
        void startElement( const XML_Char* el, const XML_Char** attr );
        void endElement( const XML_Char* el );
        void characters( const XML_Char* chars, int len );
        static void start( void* data, const XML_Char* el, const XML_Char** attr )
        {
          static_cast<QgsSOSData*>( data )->startElement( el, attr );
        }
        static void end( void* data, const XML_Char* el )
        {
          static_cast<QgsSOSData*>( data )->endElement( el );
        }
        static void chars( void* data, const XML_Char* chars, int len )
        {
          static_cast<QgsSOSData*>( data )->characters( chars, len );
        }

        QMap<QgsFeatureId, QgsFeature* >* mFeatures;
        QgsCoordinateReferenceSystem* mCrs;
        QgsRectangle* mExtent;
        QgsFeature* mCurrentFeature;
        ParseMode mParseMode;
        bool mFinished;
        QString mStringCache;
};

#endif // QGSSOSDATA_H
