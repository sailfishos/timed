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

#include "ntpcontroller.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDBusServiceWatcher>
#include <QtDBus/QDBusPendingCallWatcher>

#include "../common/log.h"

const QLatin1String CONNMAN_SERVICE("net.connman");
const QLatin1String CONNMAN_INTERFACE("net.connman.Clock");
const QLatin1String CONNMAN_METHOD("SetProperty");

NtpController::NtpController(bool enable, QObject *parent) :
    QObject(parent), m_enable(enable)
{
    m_connmanWatcher = new QDBusServiceWatcher(CONNMAN_SERVICE,
                                               QDBusConnection::systemBus(),
                                               QDBusServiceWatcher::WatchForRegistration,
                                               this);
    connect(m_connmanWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(serviceRegistered()));
    QDBusInterface connmanInterface(CONNMAN_SERVICE, "/",  CONNMAN_INTERFACE, QDBusConnection::systemBus());
    if (connmanInterface.isValid())
        enableNtpTimeAdjustment(m_enable);
}

void NtpController::enableNtpTimeAdjustment(bool enable)
{
    m_enable = enable;

    QString parameter;
    if (m_enable)
        parameter = "auto";
    else
        parameter = "manual";

    setConnmanProperty("TimeUpdates", parameter);
    setConnmanProperty("TimezoneUpdates", parameter);
}

void NtpController::setConnmanProperty(QString key, QString value)
{
    QDBusMessage request = QDBusMessage::createMethodCall(CONNMAN_SERVICE,
                                                          "/",
                                                          CONNMAN_INTERFACE,
                                                          CONNMAN_METHOD);
    QList<QVariant> arguments;
    arguments << key << QVariant::fromValue(QDBusVariant(value));
    request.setArguments(arguments);

    QDBusPendingReply<void> reply = QDBusConnection::systemBus().asyncCall(request);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    QObject::connect(watcher,SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(propertiesReply(QDBusPendingCallWatcher*)));
}

void NtpController::serviceRegistered()
{
    enableNtpTimeAdjustment(m_enable);
}

void NtpController::propertiesReply(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<void> reply = *call;
    call->deleteLater();

    if (reply.error().isValid()) {
        log_warning("Failed to call %s.%s: %s",
                    QString(CONNMAN_INTERFACE).toStdString().c_str(),
                    QString(CONNMAN_METHOD).toStdString().c_str(),
                    reply.error().message().toStdString().c_str());
    } else {
        log_debug("Set %s property %s to value %s",
                  QString(CONNMAN_INTERFACE).toStdString().c_str(),
                  reply.argumentAt(0).toString().toStdString().c_str(),
                  reply.argumentAt(1).toString().toStdString().c_str());
    }
}
