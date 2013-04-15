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
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QList>

#include "../common/log.h"

#include "ofonomodemmanager.h"
#include "ofonoconstants.h"
#include "modemwatcher.h"

struct OfonoModemProperties
{
    QDBusObjectPath name;
    QMap<QString, QVariant> dict;
};

typedef QList<OfonoModemProperties> OfonoModemList;

QDBusArgument &operator<<(QDBusArgument &argument, const OfonoModemProperties &modemProperties)
{
    argument.beginStructure();
    argument << modemProperties.name << modemProperties.dict;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, OfonoModemProperties &modemProperties)
{
    argument.beginStructure();
    argument >> modemProperties.name >> modemProperties.dict;
    argument.endStructure();
    return argument;
}

Q_DECLARE_METATYPE(OfonoModemProperties)
Q_DECLARE_METATYPE(OfonoModemList)

OfonoModemManager::OfonoModemManager(QObject *parent) :
    QObject(parent)
{
    qDBusRegisterMetaType<OfonoModemProperties>();
    qDBusRegisterMetaType<OfonoModemList>();

    QDBusConnection::systemBus().connect(OfonoConstants::OFONO_SERVICE,
                                         OfonoConstants::OFONO_MANAGER_PATH,
                                         OfonoConstants::OFONO_MANAGER_INTERFACE, "ModemAdded",
                                         this, SLOT(onModemAdded(QDBusObjectPath, QVariantMap)));
    QDBusConnection::systemBus().connect(OfonoConstants::OFONO_SERVICE,
                                         OfonoConstants::OFONO_MANAGER_PATH,
                                         OfonoConstants::OFONO_MANAGER_INTERFACE, "ModemRemoved",
                                         this, SLOT(onModemRemoved(QDBusObjectPath)));

    QDBusMessage request = QDBusMessage::createMethodCall(OfonoConstants::OFONO_SERVICE,
                                                          OfonoConstants::OFONO_MANAGER_PATH,
                                                          OfonoConstants::OFONO_MANAGER_INTERFACE,
                                                          "GetModems");

    QDBusReply<OfonoModemList> reply = QDBusConnection::systemBus().call(request);
    if (reply.error().isValid()) {
        log_error("DBus call to interface %s function GetModems of path %s failed: %s",
                  OfonoConstants::OFONO_MANAGER_INTERFACE,
                  OfonoConstants::OFONO_MANAGER_PATH,
                  reply.error().message().toStdString().c_str());
    } else {
        OfonoModemList list = reply;
        for (int i = 0; i < list.count(); i++)
            addModem(list.at(i).name.path());
    }
}

OfonoModemManager::~OfonoModemManager()
{
    QDBusConnection::systemBus().disconnect(OfonoConstants::OFONO_SERVICE,
                                            OfonoConstants::OFONO_MANAGER_PATH,
                                            OfonoConstants::OFONO_MANAGER_INTERFACE, "ModemAdded",
                                            this, SLOT(onModemAdded(QDBusObjectPath, QVariantMap)));
    QDBusConnection::systemBus().disconnect(OfonoConstants::OFONO_SERVICE,
                                            OfonoConstants::OFONO_MANAGER_PATH,
                                            OfonoConstants::OFONO_MANAGER_INTERFACE, "ModemRemoved",
                                            this, SLOT(onModemRemoved(QDBusObjectPath)));
}

bool OfonoModemManager::addModem(QString objectPath)
{
    if (!m_modemList.contains(objectPath)) {
        m_modemList.append(objectPath);
        return true;
    }
    return false;
}

QStringList OfonoModemManager::getModems()
{
    return m_modemList;
}

void OfonoModemManager::onModemAdded(QDBusObjectPath objectPath, QVariantMap map)
{
    Q_UNUSED(map)
    QString path = objectPath.path();
    if (addModem(path))
        emit modemAdded(path);
}

void OfonoModemManager::onModemRemoved(QDBusObjectPath objectPath)
{
    QString path = objectPath.path();
    if (m_modemList.contains(path)) {
        m_modemList.removeAll(path);
        emit modemRemoved(path);
    }
}
