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

#include <QList>
#include <QStringList>

#include "../common/log.h"

#include "networktime.h"
#include "networktimewatcher.h"

NetworkTime::NetworkTime(QObject *parent) :
    QObject(parent)
{
    foreach (const QString objectPath, m_modemManager.getModems())
        onModemAdded(objectPath);

    QObject::connect(&m_modemManager, SIGNAL(modemAdded(QString)),
                     this, SLOT(onModemAdded(QString)));

    QObject::connect(&m_modemManager, SIGNAL(modemRemoved(QString)),
                     this, SLOT(onModemRemoved(QString)));
}

NetworkTimeInfo NetworkTime::timeInfo() const
{
    return m_networkTimeInfo;
}

void NetworkTime::queryTimeInfo()
{
    bool querySent = false;
    foreach (const QString objectPath, m_watcherMap.keys()) {
        if (m_watcherMap.value(objectPath)->interfaceAvailable()) {
            m_watcherMap.value(objectPath)->queryNetworkTime();
            querySent = true;
        }
    }

    if (!querySent) {
        m_networkTimeInfo = NetworkTimeInfo(); // Construct a invalid NetworkTimeInfo
        emit timeInfoQueryCompleted(m_networkTimeInfo);
    }
}

bool NetworkTime::isValid() const
{
    return m_networkTimeInfo.isValid();
}

void NetworkTime::onModemAdded(QString objectPath)
{
    if (m_watcherMap.contains(objectPath))
        return;

    NetworkTimeWatcher *watcher = new NetworkTimeWatcher(objectPath, this);
    QObject::connect(watcher, SIGNAL(networkTimeChanged(QVariantMap)),
                     this, SLOT(networkTimeChanged(QVariantMap)));
    QObject::connect(watcher, SIGNAL(networkTimeQueryCompleted(QVariantMap)),
                     this, SLOT(networkTimeQueryCompletedSlot(QVariantMap)));
    watcher->queryNetworkTime();
    m_watcherMap.insert(objectPath, watcher);
}

void NetworkTime::onModemRemoved(QString objectPath)
{
    if (!m_watcherMap.contains(objectPath))
        return;

    NetworkTimeWatcher *watcher = m_watcherMap.value(objectPath);
    m_watcherMap.remove(objectPath);
    delete watcher;
}

NetworkTimeInfo NetworkTime::parseNetworkTimeInfoFromMap(QVariantMap map)
{
    QVariant tmp;
    QDateTime dateTime;
    int daylightAdjustment = -1; // -1 means "value not available"
    int offsetFromUtc = 0;
    qlonglong received = 0;
    QString mnc, mcc;
    bool ok;
    if (!map.isEmpty()) {
        tmp = map.value("UTC");
        if (tmp.isValid() && tmp.type() == QVariant::LongLong) {
            dateTime.setTimeSpec(Qt::UTC);
            qlonglong secs = tmp.toLongLong(&ok);
            if (ok) {
                dateTime.setMSecsSinceEpoch(secs*1000);
            } else {
                QDateTime tmp;
                dateTime = tmp;
            }
        }

        tmp = map.value("Received");
        if (tmp.isValid() && tmp.type() == QVariant::LongLong)
            received = tmp.toLongLong(&ok);

        tmp = map.value("Timezone");
        if (tmp.isValid() && tmp.type() == QVariant::Int)
            offsetFromUtc = tmp.toInt(&ok);

        tmp = map.value("DST");
        if (tmp.isValid() && tmp.type() == QVariant::UInt) {
            daylightAdjustment = tmp.toInt(&ok);
            if (!ok)
                daylightAdjustment = -1;
        }

        tmp = map.value("MobileCountryCode");
        if (tmp.isValid() && tmp.type() == QVariant::String)
            mcc = tmp.toString();

        tmp = map.value("MobileNetworkCode");
        if (tmp.isValid() && tmp.type() == QVariant::String)
            mnc = tmp.toString();
    }

    return NetworkTimeInfo(dateTime, daylightAdjustment, offsetFromUtc, received, 0, mnc, mcc);
}

void NetworkTime::networkTimeChanged(QVariantMap map)
{
    m_networkTimeInfo = parseNetworkTimeInfoFromMap(map);
    log_debug("time: %s",
              m_networkTimeInfo.toString().toStdString().c_str());
    emit timeInfoChanged(m_networkTimeInfo);
}

void NetworkTime::networkTimeQueryCompletedSlot(QVariantMap map)
{
    m_networkTimeInfo = parseNetworkTimeInfoFromMap(map);
    log_debug("time: %s",
              m_networkTimeInfo.toString().toStdString().c_str());
    emit timeInfoQueryCompleted(m_networkTimeInfo);
}
