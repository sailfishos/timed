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

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMetaType>
#include <QDebug>

#include "fakeofono.h"

QDBusArgument &operator<<(QDBusArgument &argument, const FakeOfono::OfonoModemProperties &modemProperties)
{
    argument.beginStructure();
    argument << modemProperties.name << modemProperties.dict;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FakeOfono::OfonoModemProperties &modemProperties)
{
    argument.beginStructure();
    argument >> modemProperties.name >> modemProperties.dict;
    argument.endStructure();
    return argument;
}

FakeOfono::FakeOfono(QObject *parent) :
    QObject(parent)
{
    qDBusRegisterMetaType<OfonoModemProperties>();
    qDBusRegisterMetaType<OfonoModemList>();

    if (!QDBusConnection::systemBus().isConnected()) {
        qDebug() << Q_FUNC_INFO << "Cannot connect to the D-Bus system bus";
        return;
    }

    if (!QDBusConnection::systemBus().registerService(OfonoConstants::OFONO_SERVICE)) {
        qDebug() << Q_FUNC_INFO << QString("Failed to register service: %1, error %2")
                    .arg(OfonoConstants::OFONO_SERVICE)
                    .arg(QDBusConnection::systemBus().lastError().message());
        return;
    }

    if (!QDBusConnection::systemBus().registerObject(OfonoConstants::OFONO_MANAGER_PATH, this,
                                                     QDBusConnection::ExportAllContents)) {
        qDebug() << Q_FUNC_INFO << "Failed to register object at" << OfonoConstants::OFONO_MANAGER_PATH;
        return;
    }
}

void FakeOfono::addModem(const QString modemPath)
{
    m_modemPaths.insert(modemPath, OfonoInfo(this));
    if (!QDBusConnection::systemBus().registerObject(modemPath, m_modemPaths[modemPath].dbusParentObject,
                                                     QDBusConnection::ExportAdaptors)) {
        qDebug() << Q_FUNC_INFO << "Failed to register object (adaptors) at " << modemPath;
        return;
    }
    emit ModemAdded(QDBusObjectPath(modemPath), QVariantMap());
}

void FakeOfono::enableInterfaces(const QString modemPath)
{
    if (!m_modemPaths.contains(modemPath)) {
        qWarning() << Q_FUNC_INFO << "invalid modem:" << modemPath << ", have:" << m_modemPaths.keys();
        return;
    }

    m_modemPaths[modemPath].modem->addInterface(OfonoConstants::OFONO_NETWORKREGISTRATION_INTERFACE);
    m_modemPaths[modemPath].modem->addInterface(OfonoConstants::OFONO_NETWORKTIME_INTERFACE);
}

FakeOfono::OfonoModemList FakeOfono::GetModems()
{
    OfonoModemList modemList;
    if (m_modemPaths.isEmpty())
        return modemList;

    Q_FOREACH (const QString &modemPath, m_modemPaths.keys()) {
        OfonoModemProperties props;
        props.name.setPath(modemPath);
        props.dict.insert("Interfaces", QVariant(QStringList()));
        modemList.append(props);
    }

    return modemList;
}

void FakeOfono::emulateNetworkRegistration(const QString modem, const QString mnc, const QString mcc)
{
    if (!m_modemPaths.contains(modem)) {
        qWarning() << Q_FUNC_INFO << "invalid modem:" << modem << ", have:" << m_modemPaths.keys();
        return;
    }
    m_modemPaths[modem].networkRegistration->emulateNetworkRegistration(mnc, mcc);
}

void FakeOfono::emulateNetworkTimeChange(qlonglong utc, qlonglong received, int timezone,
                                  uint dst, QString mcc, QString mnc, QString modem)
{
    if (!m_modemPaths.contains(modem)) {
        qWarning() << Q_FUNC_INFO << "invalid modem:" << modem << ", have:" << m_modemPaths.keys();
        return;
    }
    m_modemPaths[modem].networkTimeInterface->emulateNetworkTimeChange(utc, received, timezone, dst, mcc, mnc, modem);
}


