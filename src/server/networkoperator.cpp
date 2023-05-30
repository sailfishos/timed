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
#include <QtDebug>

#include "../common/log.h"

#include "networkoperator.h"
#include "networkregistrationwatcher.h"

NetworkOperator::NetworkOperator(QObject *parent)
    : QObject(parent)
{
    foreach (const QString objectPath, m_modemManager.getModemList())
        onModemAdded(objectPath);

    QObject::connect(&m_modemManager,
                     SIGNAL(modemAdded(QString)),
                     this,
                     SLOT(onModemAdded(QString)));

    QObject::connect(&m_modemManager,
                     SIGNAL(modemRemoved(QString)),
                     this,
                     SLOT(onModemRemoved(QString)));
}

NetworkOperator::~NetworkOperator()
{
    foreach (NetworkRegistrationWatcher *watcher, m_watcherMap)
        delete watcher;

    m_watcherMap.clear();
}

QStringList NetworkOperator::modems() const
{
    return m_operatorInfo.keys();
}

QString NetworkOperator::defaultModem() const
{
    return m_defaultModem;
}

QString NetworkOperator::mnc(const QString &modem) const
{
    // return the mnc for the specified modem if specified
    if (!modem.isEmpty()) {
        if (m_operatorInfo.contains(modem)) {
            return m_operatorInfo[modem].mnc;
        }
        log_debug("modem specified but not known, returning empty mnc: %s",
                  modem.toStdString().c_str());
        return QString();
    }

    // return the mnc for the default modem if we have one
    if (!m_defaultModem.isEmpty()) {
        return m_operatorInfo[m_defaultModem].mnc;
    }

    // return the mnc for the first valid modem in our dictionary
    Q_FOREACH (const QString &mdm, m_operatorInfo.keys()) {
        const OperatorInfo &i(m_operatorInfo[mdm]);
        if (!i.mnc.isEmpty() && !i.mcc.isEmpty()) {
            m_defaultModem = mdm; // set the default modem to this valid modem
            return i.mnc;
        }
    }

    log_notice("mnc: no modem path specified and no default modem set");
    return QString();
}

QString NetworkOperator::mcc(const QString &modem) const
{
    // return the mcc for the specified modem if specified
    if (!modem.isEmpty()) {
        if (m_operatorInfo.contains(modem)) {
            return m_operatorInfo[modem].mcc;
        }
        log_debug("modem specified but not known, returning empty mcc: %s",
                  modem.toStdString().c_str());
        return QString();
    }

    // return the mcc for the default modem if we have one
    if (!m_defaultModem.isEmpty()) {
        return m_operatorInfo[m_defaultModem].mcc;
    }

    // return the mcc for the first valid modem in our dictionary
    Q_FOREACH (const QString &mdm, m_operatorInfo.keys()) {
        const OperatorInfo &i(m_operatorInfo[mdm]);
        if (!i.mnc.isEmpty() && !i.mcc.isEmpty()) {
            m_defaultModem = mdm; // set the default modem to this valid modem
            return i.mcc;
        }
    }

    log_notice("mcc: no modem path specified and no default modem set");
    return QString();
}

bool NetworkOperator::isValid(const QString &modem) const
{
    if (!modem.isEmpty()) {
        return m_operatorInfo.contains(modem)
               && !(m_operatorInfo[modem].mcc.isEmpty() || m_operatorInfo[modem].mnc.isEmpty());
    }

    if (!m_defaultModem.isEmpty()) {
        return !(m_operatorInfo[m_defaultModem].mcc.isEmpty()
                 || m_operatorInfo[m_defaultModem].mnc.isEmpty());
    }

    Q_FOREACH (const QString &mdm, m_operatorInfo.keys()) {
        const OperatorInfo &i(m_operatorInfo[mdm]);
        if (!i.mnc.isEmpty() && !i.mcc.isEmpty()) {
            m_defaultModem = mdm; // set the default modem to this valid modem
            return true;
        }
    }

    log_notice("isValid: no modem path specified and no default modem set");
    return false;
}

void NetworkOperator::onModemAdded(QString objectPath)
{
    if (m_watcherMap.contains(objectPath))
        return;

    NetworkRegistrationWatcher *watcher = new NetworkRegistrationWatcher(objectPath, this);
    QObject::connect(watcher,
                     SIGNAL(propertyChanged(QString, QString, QVariant)),
                     this,
                     SLOT(onWatcherPropertyChanged(QString, QString, QVariant)));
    m_watcherMap.insert(objectPath, watcher);
    m_operatorInfo.insert(objectPath, OperatorInfo());
    watcher->getProperties();
}

void NetworkOperator::onModemRemoved(QString objectPath)
{
    if (!m_watcherMap.contains(objectPath))
        return;

    NetworkRegistrationWatcher *watcher = m_watcherMap.value(objectPath);
    m_watcherMap.remove(objectPath);
    m_operatorInfo.remove(objectPath);
    if (m_defaultModem == objectPath) {
        m_defaultModem = QString();
        Q_FOREACH (const QString &mdm, m_operatorInfo.keys()) {
            const OperatorInfo &i(m_operatorInfo[mdm]);
            if (!i.mnc.isEmpty() && !i.mcc.isEmpty()) {
                m_defaultModem = mdm; // set the default modem to this valid modem
                break;
            }
        }
    }
    delete watcher;
}

void NetworkOperator::onWatcherPropertyChanged(QString objectPath, QString name, QVariant value)
{
    if (!m_operatorInfo.contains(objectPath)
        && (name.compare("MobileCountryCode") == 0 || name.compare("MobileNetworkCode") == 0)) {
        m_operatorInfo.insert(objectPath, OperatorInfo());
    }

    if (name.compare("MobileCountryCode") == 0) {
        if (value.type() == QVariant::String) {
            m_operatorInfo[objectPath].mcc = value.toString();
            m_operatorInfo[objectPath].mccUpdated = true;
        }
    } else if (name.compare("MobileNetworkCode") == 0) {
        if (value.type() == QVariant::String) {
            m_operatorInfo[objectPath].mnc = value.toString();
            m_operatorInfo[objectPath].mncUpdated = true;
        }
    }

    if (m_operatorInfo[objectPath].mccUpdated && m_operatorInfo[objectPath].mncUpdated) {
        m_operatorInfo[objectPath].mccUpdated = false;
        m_operatorInfo[objectPath].mncUpdated = false;
        if (m_defaultModem.isEmpty()) {
            m_defaultModem = objectPath;
        }
        log_debug("operator changed: MNC: %s, MCC: %s, modem: %s",
                  m_operatorInfo[objectPath].mnc.toStdString().c_str(),
                  m_operatorInfo[objectPath].mcc.toStdString().c_str(),
                  objectPath.toStdString().c_str());
        emit operatorChanged(objectPath,
                             m_operatorInfo[objectPath].mnc,
                             m_operatorInfo[objectPath].mcc);
    }
}
