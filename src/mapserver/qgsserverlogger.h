/***************************************************************************
                              qgsserverlogger.h
                              -----------------
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

#ifndef QGSSERVERLOGGER_H
#define QGSSERVERLOGGER_H

#include <QString>

class QgsServerLogger
{
    public:
        static QgsServerLogger* instance();
        void logMessage( const QString& message, int logLevel );
        int logLevel() const { return mLogLevel; }
        QString logFile() const { return mLogFile; }

    protected:
        QgsServerLogger();

    private:
        static QgsServerLogger* mInstance;

        QString mLogFile;
        int mLogLevel;
};

#endif // QGSSERVERLOGGER_H
