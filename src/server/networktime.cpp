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
    foreach (const QString objectPath, m_modemManager.getModemList())
        onModemAdded(objectPath);

    QObject::connect(&m_modemManager, SIGNAL(modemAdded(QString)),
                     this, SLOT(onModemAdded(QString)));

    QObject::connect(&m_modemManager, SIGNAL(modemRemoved(QString)),
                     this, SLOT(onModemRemoved(QString)));
}

NetworkTime::~NetworkTime()
{
    foreach (NetworkTimeWatcher* watcher, m_watcherMap)
        delete watcher;

    m_watcherMap.clear();
}

QString NetworkTime::defaultModem() const
{
    return m_defaultModem;
}

NetworkTimeInfo NetworkTime::timeInfo(const QString &modem) const
{
    if (m_networkTimeInfo.contains(modem))
        return m_networkTimeInfo[modem];

    if (!m_defaultModem.isEmpty())
        return m_networkTimeInfo[m_defaultModem];

    Q_FOREACH (const QString &m, m_networkTimeInfo.keys()) {
        if (m_networkTimeInfo[m].isValid()) {
            m_defaultModem = m;
            return m_networkTimeInfo[m];
        }
    }

    return NetworkTimeInfo();
}

#include <QtDebug>
void NetworkTime::queryTimeInfo(const QString &modem)
{
    bool querySent = false;
    if (!modem.isEmpty()) {
        if (m_watcherMap.contains(modem)) {
            if (m_watcherMap.value(modem)->interfaceAvailable()) {
                m_watcherMap.value(modem)->queryNetworkTime();
                return;
            } else {
                log_notice("cannot query time info for unavailable modem: %s", modem.toStdString().c_str());
            }
        } else {
            log_notice("cannot query time info for unknown modem: %s", modem.toStdString().c_str());
        }
    } else if (!m_defaultModem.isEmpty()) {
        if (m_watcherMap.contains(m_defaultModem)) {
            if (m_watcherMap.value(m_defaultModem)->interfaceAvailable()) {
                m_watcherMap.value(m_defaultModem)->queryNetworkTime();
                return;
            } else {
                log_notice("cannot query time info for unavailable default modem: %s", m_defaultModem.toStdString().c_str());
            }
        } else {
            log_error("cannot query time info for unknown default modem: %s", m_defaultModem.toStdString().c_str()); // should never happen.
        }
    } else {
        // query any available modem
        Q_FOREACH (const QString &objectPath, m_watcherMap.keys()) {
            if (m_watcherMap.value(objectPath)->interfaceAvailable()) {
                m_watcherMap.value(objectPath)->queryNetworkTime();
                m_defaultModem = objectPath;
                querySent = true;
            }
        }
    }

    if (!querySent) {
        emit timeInfoQueryCompleted(NetworkTimeInfo()); // Construct a invalid NetworkTimeInfo
    }
}

bool NetworkTime::isValid(const QString &modem) const
{
    if (m_networkTimeInfo.contains(modem))
        return m_networkTimeInfo[modem].isValid();

    if (!m_defaultModem.isEmpty())
        return m_networkTimeInfo[m_defaultModem].isValid();

    Q_FOREACH (const QString &m, m_networkTimeInfo.keys()) {
        if (m_networkTimeInfo[m].isValid()) {
            m_defaultModem = m;
            return true;
        }
    }

    return false;
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
    m_networkTimeInfo.remove(objectPath);
    if (m_defaultModem.compare(objectPath, Qt::CaseInsensitive) == 0) {
        Q_FOREACH (const QString &m, m_networkTimeInfo.keys()) {
            if (m_networkTimeInfo[m].isValid()) {
                m_defaultModem = m;
                break;
            }
        }
    }
    delete watcher;
}

NetworkTimeInfo NetworkTime::parseNetworkTimeInfoFromMap(QVariantMap map, const QString &modem)
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

    if (modem.isEmpty()) log_error("unknown modem path for network time info: mnc=%s, mcc=%s", mnc.toStdString().c_str(), mcc.toStdString().c_str());
    return NetworkTimeInfo(dateTime, daylightAdjustment, offsetFromUtc, received, 0, mnc, mcc, modem);
}

void NetworkTime::networkTimeChanged(QVariantMap map)
{
    QString modem = m_watcherMap.key(qobject_cast<NetworkTimeWatcher*>(sender()));
    m_networkTimeInfo.insert(modem, parseNetworkTimeInfoFromMap(map, modem));
    if (m_defaultModem.isEmpty() && m_networkTimeInfo[modem].isValid()) {
        m_defaultModem = modem;
    }
    log_debug("time: %s from modem: %s",
              m_networkTimeInfo[modem].toString().toStdString().c_str(), modem.toStdString().c_str());
    emit timeInfoChanged(m_networkTimeInfo[modem]);
}

void NetworkTime::networkTimeQueryCompletedSlot(QVariantMap map)
{
    QString modem = m_watcherMap.key(qobject_cast<NetworkTimeWatcher*>(sender()));
    m_networkTimeInfo.insert(modem, parseNetworkTimeInfoFromMap(map, modem));
    if (m_defaultModem.isEmpty() && m_networkTimeInfo[modem].isValid()) {
        m_defaultModem = modem;
    }
    log_debug("time: %s from modem: %s",
              m_networkTimeInfo[modem].toString().toStdString().c_str(), modem.toStdString().c_str());
    emit timeInfoQueryCompleted(m_networkTimeInfo[modem]);
}
