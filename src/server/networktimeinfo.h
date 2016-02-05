/***************************************************************************
**                                                                        **
**  Copyright (C) 2013 Jolla Ltd.                                         **
**  Contact: Petri M. Gerdt <petri.gerdt@jollamobile.com>                 **
**                                                                        **
**     This file is part of Timed                                         **
**                                                                        **
**     Timed is free software; you can redistribute it and/or modify      **
**     it under the terms of the GNU Lesser General Public License        **
**     version 2.1 as published by the Free Software Foundation.          **
**                                                                        **
**     Timed is distributed in the hope that it will be useful, but       **
**     WITHOUT ANY WARRANTY;  without even the implied warranty  of       **
**     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               **
**     See the GNU Lesser General Public License  for more details.       **
**                                                                        **
**   You should have received a copy of the GNU  Lesser General Public    **
**   License along with Timed. If not, see http://www.gnu.org/licenses/   **
**                                                                        **
***************************************************************************/

#ifndef NETWORKTIMEINFO_H
#define NETWORKTIMEINFO_H

#include <QDateTime>
#include <QString>
#include <time.h>

// See http://harmattan-dev.nokia.com/docs/platform-api-reference/xml/daily-docs/libcellular-qt/classCellular_1_1NetworkTimeInfo.html
class NetworkTimeInfo
{
public:
    NetworkTimeInfo(const QDateTime &dateTime, const int daylightAdjustment,
                    const int offsetFromUtc, const qlonglong timestampSeconds,
                    const qlonglong timestampNanoSeconds,
                    const QString &mnc, const QString &mcc, const QString &modem);
    NetworkTimeInfo(const NetworkTimeInfo &other);
    NetworkTimeInfo();
    ~NetworkTimeInfo();

    QDateTime dateTime() const;
    int offsetFromUtc() const;
    int daylightAdjustment() const;
    const timespec *timestamp() const;
    QString mnc() const;
    QString mcc() const;
    QString modem() const;
    bool isValid() const;
    QString toString() const;

private:
    QDateTime m_dateTime;
    int m_daylightAdjustment;
    int m_offsetFromUtc;
    QString m_mnc;
    QString m_mcc;
    QString m_modem;
    struct timespec m_timespec;
};
#endif // NETWORKTIMEINFO_H
