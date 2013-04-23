/***************************************************************************
**
** Copyright (C) 2013 Jolla Ltd.
** Contact: Petri M. Gerdt <petri.gerdt@jollamobile.com>
**
** This file is part of Timed.
**
** Timed is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License
** version 2.1 as published by the Free Software Foundation.
**
** Timed is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY;  without even the implied warranty  of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU Lesser General Public License  for more details.
**
** You should have received a copy of the GNU  Lesser General Public
** License along with Timed. If not, see http://www.gnu.org/licenses/
**
***************************************************************************/

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include "../common/log.h"

#include "ofonoconstants.h"
#include "networktimewatcher.h"

NetworkTimeWatcher::NetworkTimeWatcher(const QString path, QObject *parent) :
    ModemWatcher(path, OfonoConstants::OFONO_NETWORKTIME_INTERFACE, parent)
{
    QDBusConnection::systemBus().connect(OfonoConstants::OFONO_SERVICE, objectPath(),
                                         interface(), "NetworkTimeChanged",
                                         this, SLOT(onNetworkTimeChanged(QVariantMap)));

    QObject::connect(this, SIGNAL(interfaceAvailableChanged(bool)), this, SLOT(queryNetworkTime()));
}

NetworkTimeWatcher::~NetworkTimeWatcher()
{
    QDBusConnection::systemBus().disconnect(OfonoConstants::OFONO_SERVICE, objectPath(),
                                            interface(), "NetworkTimeChanged",
                                            this, SLOT(onNetworkTimeChanged(QVariantMap)));
}

void NetworkTimeWatcher::queryNetworkTime()
{
    if (!interfaceAvailable())
        return;

    QDBusInterface dbusInterface(OfonoConstants::OFONO_SERVICE, objectPath(), interface(),
                                 QDBusConnection::systemBus());
    if (!dbusInterface.isValid()) {
        log_error("Dbus interface %s of path %s is invalid.",
                  interface().toStdString().c_str(),
                  objectPath().toStdString().c_str());
        return;
    }

    QDBusPendingCall async = dbusInterface.asyncCall("GetNetworkTime");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(queryNetworkTimeCallback(QDBusPendingCallWatcher*)));
}

void NetworkTimeWatcher::queryNetworkTimeCallback(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QVariantMap> reply = *watcher;
    if (reply.error().isValid()) {
        log_error("DBus call to interface %s function GetNetworkTime of path %s failed: %s",
                  interface().toStdString().c_str(),
                  objectPath().toStdString().c_str(),
                  reply.error().message().toStdString().c_str());
        return;
    }

    QVariantMap map = reply.argumentAt<0>();
    emit networkTimeQueryCompleted(map);
}

void NetworkTimeWatcher::onNetworkTimeChanged(QVariantMap map)
{
    emit networkTimeChanged(map);
}

