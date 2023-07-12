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

#ifndef NETWORKTIMEWATCHER_H
#define NETWORKTIMEWATCHER_H

#include <QDBusVariant>
#include <QObject>
#include <QVariantMap>

#include "modemwatcher.h"

class QDBusInterface;
class QDBusPendingCallWatcher;

class NetworkTimeWatcher : public ModemWatcher
{
    Q_OBJECT

public:
    explicit NetworkTimeWatcher(const QString objectPath, QObject *parent = 0);
    ~NetworkTimeWatcher();

public slots:
    void queryNetworkTime();

signals:
    void networkTimeChanged(QVariantMap map);
    void networkTimeQueryCompleted(QVariantMap map);

private slots:
    void queryNetworkTimeCallback(QDBusPendingCallWatcher *watcher);
    void onNetworkTimeChanged(QVariantMap map);
};
#endif // NETWORKTIMEWATCHER_H
