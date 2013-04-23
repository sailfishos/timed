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

#include <QDBusInterface>
#include <QDBusReply>

#include "../common/log.h"

#include "ofonoconstants.h"
#include "modemwatcher.h"

ModemWatcher::ModemWatcher(const QString objectPath, const QString interface, QObject *parent) :
    QObject(parent), m_objectPath(objectPath), m_interface(interface)
{
    QDBusConnection::systemBus().connect(OfonoConstants::OFONO_SERVICE, m_objectPath,
                                         OfonoConstants::OFONO_MODEM_INTERFACE, "PropertyChanged",
                                         this, SLOT(onModemPropertyChanged(QString, QDBusVariant)));

    QDBusMessage request = QDBusMessage::createMethodCall(OfonoConstants::OFONO_SERVICE,
                                                          m_objectPath,
                                                          OfonoConstants::OFONO_MODEM_INTERFACE,
                                                          "GetProperties");

    QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(request);
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
    QDBusConnection::systemBus().disconnect(OfonoConstants::OFONO_SERVICE, m_objectPath,
                                            OfonoConstants::OFONO_MODEM_INTERFACE, "PropertyChanged",
                                            this, SLOT(onModemPropertyChanged(QString, QDBusVariant)));
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
// the list: true indicates that the interface is in the list, false
// indicates that it is not. If m_interfaceAvailable was changed,
// then returns true, otherwise returns false.
bool ModemWatcher::checkInterfaceAvailability(QVariant variant)
{
    if (variant.type() != QVariant::StringList)
        return false;

    QStringList list = variant.toStringList();
    bool available = list.contains(m_interface);
    if (available != m_interfaceAvailable) {
        m_interfaceAvailable = available;
        return true;
    }

    return false;
}

void ModemWatcher::onModemPropertyChanged(QString name, QDBusVariant value)
{
    if (name.compare("Interfaces") != 0)
        return;

    if (checkInterfaceAvailability(value.variant())) {
        log_debug("interface %s availability changed: %s",
                  m_interface.toStdString().c_str(),
                  m_interfaceAvailable ? "available" : "not available");
        emit interfaceAvailableChanged(m_interfaceAvailable);
    }
}
