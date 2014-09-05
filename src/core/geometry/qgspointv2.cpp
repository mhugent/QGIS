/***************************************************************************
                         qgspointv2.cpp
                         --------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgspointv2.h"

QgsPointV2::QgsPointV2( double x, double y ): QgsAbstractGeometryV2( 2, false ), mX( x ), mY( y ), mZ( 0.0 ), mM( 0.0 )
{}

QgsPointV2::QgsPointV2( double x, double y, double mz, bool hasM ): QgsAbstractGeometryV2( hasM ? 2 : 3, false ),
    mX( x ), mY( y ), mZ( hasM ? 0.0 : mz ), mM( hasM ? mz : 0.0  )
{}

QgsPointV2::QgsPointV2( double x, double y, double z, double m ): QgsAbstractGeometryV2( 3, true ), mX( x ), mY( y ), mZ( z ), mM( m )
{}

QgsPointV2::~QgsPointV2()
{}

QgsAbstractGeometryV2* QgsPointV2::clone() const
{
    switch( mCoordDimension )
    {
        case 3:
            return mHasM ? new QgsPointV2( mX, mY, mZ, mM ) : new QgsPointV2( mX, mY, mZ, false );
            break;
        default:
            return mHasM ? new QgsPointV2( mX, mY, mM, true ) : new QgsPointV2( mX, mY );
    }
}

void QgsPointV2::fromWkb( const unsigned char * wkb, size_t length )
{
    //soon
}

void QgsPointV2::fromGeos( GEOSGeometry* geos )
{
    //soon
}

void QgsPointV2::fromWkt( const QString& wkt )
{
    //soon
}

QString QgsPointV2::asText( int precision ) const
{
    return QString(); //soon...
}

unsigned char* QgsPointV2::asBinary( int& binarySize ) const
{
    return 0; //soon...
}

QString QgsPointV2::asGML() const
{
    return QString();
}
