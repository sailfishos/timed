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

#ifndef NETWORKREGISTRATIONRWATCHER_H
#define NETWORKREGISTRATIONRWATCHER_H

#include <QObject>
#include <QVariantMap>
#include <QDBusVariant>

#include "modemwatcher.h"

class QDBusInterface;
class QDBusPendingCallWatcher;

class NetworkRegistrationWatcher : public ModemWatcher
{
    Q_OBJECT
public:
    explicit NetworkRegistrationWatcher(const QString objectPath, QObject *parent = 0);
    ~NetworkRegistrationWatcher();

public slots:
    void getProperties();

signals:
    void propertyChanged(QString objectPath, QString name, QVariant value);

private slots:
    void onPropertyChanged(QString name, QDBusVariant value);
    void getPropertiesCallback(QDBusPendingCallWatcher *watcher);
};
#endif // NETWORKREGISTRATIONRWATCHER_H
