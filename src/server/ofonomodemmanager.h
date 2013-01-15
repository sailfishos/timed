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

#ifndef OFONOMODEMMANAGER_H
#define OFONOMODEMMANAGER_H

#include <QDBusObjectPath>
#include <QObject>
#include <QStringList>
#include <QVariant>

class ModemWatcher;

class OfonoModemManager : public QObject
{
    Q_OBJECT

public:
    explicit OfonoModemManager(QObject *parent = 0);
    ~OfonoModemManager();

    QStringList getModems();

signals:
    void modemAdded(QString objectPath);
    void modemRemoved(QString objectPath);

private slots:
    void onModemAdded(QDBusObjectPath objectPath, QVariantMap map);
    void onModemRemoved(QDBusObjectPath objectPath);

private:
    QStringList m_modemList;
    bool addModem(QString objectPath);
};

#endif // OFONOMODEMMANAGER_H
