#ifndef QGSWKBPTR_H
#define QGSWKBPTR_H

#include "qgswkbtypes.h"
#include "qgis.h"

class CORE_EXPORT QgsWkbPtr
{
    mutable unsigned char *mP;

  public:
    QgsWkbPtr( unsigned char *p ) { mP = p; }

    inline const QgsWkbPtr &operator>>( double &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( unsigned int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( char &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( QgsWKBTypes::Type &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsWkbPtr &operator>>( QGis::WkbType &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }

    inline QgsWkbPtr &operator<<( const double &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const int &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const unsigned int &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const char &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const QgsWKBTypes::Type &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline QgsWkbPtr &operator<<( const QGis::WkbType &v ) { memcpy( mP, &v, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline void operator+=( int n ) { mP += n; }

    inline operator unsigned char *() const { return mP; }
};

class CORE_EXPORT QgsConstWkbPtr
{
    mutable unsigned char *mP;

  public:
    QgsConstWkbPtr( const unsigned char *p ) { mP = ( unsigned char * ) p; }

    inline const QgsConstWkbPtr &operator>>( double &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( unsigned int &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( char &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( QGis::WkbType &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( QgsWKBTypes::Type &v ) const { memcpy( &v, mP, sizeof( v ) ); mP += sizeof( v ); return *this; }

    inline void operator+=( int n ) { mP += n; }
    inline void operator-=( int n ) { mP -= n; }

    inline operator const unsigned char *() const { return mP; }
};

#endif // QGSWKBPTR_H
