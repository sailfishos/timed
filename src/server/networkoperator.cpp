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

#include <QStringList>

#include "../common/log.h"

#include "networkoperator.h"
#include "networkregistrationwatcher.h"

NetworkOperator::NetworkOperator(QObject *parent) :
    QObject(parent), m_mccUpdated(false), m_mncUpdated(false)
{
    foreach (const QString objectPath, m_modemManager.getModems())
        onModemAdded(objectPath);

    QObject::connect(&m_modemManager, SIGNAL(modemAdded(QString)),
                     this, SLOT(onModemAdded(QString)));

    QObject::connect(&m_modemManager, SIGNAL(modemRemoved(QString)),
                     this, SLOT(onModemRemoved(QString)));
}

QString NetworkOperator::mnc() const
{
    return m_mnc;
}

QString NetworkOperator::mcc() const
{
    return m_mcc;
}

bool NetworkOperator::isValid() const
{
    return !(m_mcc.isEmpty() || m_mnc.isEmpty());
}

void NetworkOperator::onModemAdded(QString objectPath)
{
    if (m_watcherMap.contains(objectPath))
        return;

    NetworkRegistrationWatcher *watcher = new NetworkRegistrationWatcher(objectPath, this);
    QObject::connect(watcher, SIGNAL(propertyChanged(QString, QString, QVariant)),
                     this, SLOT(onWatcherPropertyChanged(QString, QString, QVariant)));
    watcher->getProperties();
    m_watcherMap.insert(objectPath, watcher);
}

void NetworkOperator::onModemRemoved(QString objectPath)
{
    if (!m_watcherMap.contains(objectPath))
        return;

    NetworkRegistrationWatcher *watcher = m_watcherMap.value(objectPath);
    m_watcherMap.remove(objectPath);
    delete watcher;
}

void NetworkOperator::onWatcherPropertyChanged(QString objectPath, QString name, QVariant value)
{
    if (m_currentObjectPath.compare(objectPath) != 0
            && (name.compare("MobileCountryCode") == 0 || name.compare("MobileNetworkCode") == 0)) {
        m_currentObjectPath = objectPath;
        m_mccUpdated = false;
        m_mncUpdated = false;
        m_mnc = "";
        m_mcc = "";
    }

    if (name.compare("MobileCountryCode") == 0) {
        if (value.type() == QVariant::String) {
            m_mcc = value.toString();
            m_mccUpdated = true;
        }
    } else if (name.compare("MobileNetworkCode") == 0) {
        if (value.type() == QVariant::String) {
            m_mnc = value.toString();
            m_mncUpdated = true;
        }
    }

    if (m_mccUpdated && m_mncUpdated) {
        m_mccUpdated = false;
        m_mncUpdated = false;
        log_debug("operator changed: MNC: %s, MCC: %s, modem: %s",
                  m_mnc.toStdString().c_str(),
                  m_mcc.toStdString().c_str(),
                  objectPath.toStdString().c_str());
        emit operatorChanged(m_mnc, m_mcc);
    }
}
