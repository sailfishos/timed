/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2011 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
**   Author: Victor Portnov <ext-victor.portnov@nokia.com>                **
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
#ifndef INTERFACES_H
#define INTERFACES_H

#include <QDBusAbstractInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>

#include <dsme/dsme_dbus_if.h>

class DsmeReqInterface : public QDBusAbstractInterface
{
    Q_OBJECT;

public:
    DsmeReqInterface(QObject *parent = NULL)
        : QDBusAbstractInterface(dsme_service,
                                 dsme_req_path,
                                 dsme_req_interface,
                                 QDBusConnection::systemBus(),
                                 parent)
    {}

    QDBusMessage get_state_sync(void) { return call(dsme_get_state); }
    QDBusPendingCall get_state_async(void) { return asyncCall(dsme_get_state); }
};

#endif //INTERFACES_H
