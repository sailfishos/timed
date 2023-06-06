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

#ifndef NETWORKTIME_H
#define NETWORKTIME_H

#include <QMap>
#include <QObject>
#include <QVariant>

#include "networktimeinfo.h"
#include "ofonomodemmanager.h"

class QDBusPendingCallWatcher;
class NetworkTimeWatcher;

// See http://harmattan-dev.nokia.com/docs/platform-api-reference/xml/daily-docs/libcellular-qt/classCellular_1_1NetworkTime.html
class NetworkTime : public QObject
{
    Q_OBJECT

public:
    explicit NetworkTime(QObject *parent = 0);
    ~NetworkTime();
    QString defaultModem() const;
    NetworkTimeInfo timeInfo(const QString &modemPath = QString()) const;
    void queryTimeInfo(const QString &modemPath = QString());
    bool isValid(const QString &modemPath = QString()) const;

signals:
    void timeInfoChanged(const NetworkTimeInfo &timeInfo);
    void timeInfoQueryCompleted(const NetworkTimeInfo &timeInfo);

private:
    mutable QString m_defaultModem;
    QMap<QString, NetworkTimeInfo> m_networkTimeInfo;
    QMap<QString, NetworkTimeWatcher *> m_watcherMap;
    NetworkTimeInfo parseNetworkTimeInfoFromMap(QVariantMap map, const QString &modemPath);
    OfonoModemManager m_modemManager;

private slots:
    void onModemAdded(QString objectPath);
    void onModemRemoved(QString objectPath);
    void networkTimeChanged(QVariantMap map);
    void networkTimeQueryCompletedSlot(QVariantMap map);
};
#endif // NETWORKTIME_H
