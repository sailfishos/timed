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

#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusServiceWatcher>

#include "../common/log.h"

#include "modemwatcher.h"
#include "ofonoconstants.h"

ModemWatcher::ModemWatcher(const QString objectPath, const QString interface, QObject *parent)
    : QObject(parent)
    , m_objectPath(objectPath)
    , m_interface(interface)
    , m_interfaceAvailable(false)
{
    QDBusConnection::systemBus().connect(OfonoConstants::OFONO_SERVICE,
                                         m_objectPath,
                                         OfonoConstants::OFONO_MODEM_INTERFACE,
                                         "PropertyChanged",
                                         this,
                                         SLOT(onModemPropertyChanged(QString, QDBusVariant)));

    m_ofonoWatcher = new QDBusServiceWatcher(OfonoConstants::OFONO_SERVICE,
                                             QDBusConnection::systemBus(),
                                             QDBusServiceWatcher::WatchForRegistration,
                                             this);
    connect(m_ofonoWatcher, SIGNAL(serviceRegistered(QString)), this, SLOT(getProperties()));

    if (QDBusConnection::systemBus().interface()->isServiceRegistered(OfonoConstants::OFONO_SERVICE))
        getProperties();
}

void ModemWatcher::getProperties()
{
    QDBusMessage request = QDBusMessage::createMethodCall(OfonoConstants::OFONO_SERVICE,
                                                          m_objectPath,
                                                          OfonoConstants::OFONO_MODEM_INTERFACE,
                                                          "GetProperties");
    QDBusPendingReply<QVariantMap> reply = QDBusConnection::systemBus().asyncCall(request);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    QObject::connect(watcher,
                     SIGNAL(finished(QDBusPendingCallWatcher *)),
                     this,
                     SLOT(getPropertiesReply(QDBusPendingCallWatcher *)));
}

void ModemWatcher::getPropertiesReply(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QVariantMap> reply = *call;
    call->deleteLater();
    if (reply.error().isValid()) {
        log_error("DBus call to interface %s function GetProperties of path %s failed: %s",
                  m_interface.toStdString().c_str(),
                  m_objectPath.toStdString().c_str(),
                  reply.error().message().toStdString().c_str());
    } else {
        QVariantMap properties = reply;

        checkInterfaceAvailability(properties.value("Interfaces"));
    }
}

ModemWatcher::~ModemWatcher()
{
    QDBusConnection::systemBus().disconnect(OfonoConstants::OFONO_SERVICE,
                                            m_objectPath,
                                            OfonoConstants::OFONO_MODEM_INTERFACE,
                                            "PropertyChanged",
                                            this,
                                            SLOT(onModemPropertyChanged(QString, QDBusVariant)));
}

bool ModemWatcher::interfaceAvailable() const
{
    return m_interfaceAvailable;
}

QString ModemWatcher::objectPath() const
{
    return m_objectPath;
}

QString ModemWatcher::interface() const
{
    return m_interface;
}

// Checks if the interface stored in m_interface is present in
// list of interfaces stored in parameter variant.
// Changes m_interfaceAvailable to reflect interface presence in
// the list and emits the change signal interfaceAvailableChanged
void ModemWatcher::checkInterfaceAvailability(QVariant variant)
{
    if (variant.type() != QVariant::StringList)
        return;

    QStringList list = variant.toStringList();
    bool available = list.contains(m_interface);
    if (available != m_interfaceAvailable) {
        m_interfaceAvailable = available;
        emit interfaceAvailableChanged(m_interfaceAvailable);
        log_debug("interface %s availability changed: %s",
                  m_interface.toStdString().c_str(),
                  m_interfaceAvailable ? "available" : "not available");
    }
}

void ModemWatcher::onModemPropertyChanged(QString name, QDBusVariant value)
{
    if (name.compare("Interfaces") != 0)
        return;

    checkInterfaceAvailability(value.variant());
}
