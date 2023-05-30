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

#ifndef MODEMWATCHER_H
#define MODEMWATCHER_H

#include <QDBusVariant>
#include <QObject>
#include <QVariant>

class QDBusInterface;
class QDBusPendingCallWatcher;
class QDBusServiceWatcher;

class ModemWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ModemWatcher(const QString objectPath, const QString interface, QObject *parent = 0);
    ~ModemWatcher();
    bool interfaceAvailable() const;
    QString objectPath() const;
    QString interface() const;

signals:
    void interfaceAvailableChanged(bool available);

private:
    QString m_objectPath;
    QString m_interface;
    bool m_interfaceAvailable;
    QDBusServiceWatcher *m_ofonoWatcher;

    void checkInterfaceAvailability(QVariant variant);

private slots:
    void onModemPropertyChanged(QString objectPath, QDBusVariant value);
    void getProperties();
    void getPropertiesReply(QDBusPendingCallWatcher *call);
};
#endif // MODEMWATCHER_H
