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

#ifndef FAKEOFONO_H
#define FAKEOFONO_H

#include <QObject>
#include <QDBusObjectPath>
#include <QVariant>

#include "ofonoconstants.h"

class QDBusInterface;
struct OfonoModemProperties;
class ModemInterface;
class NetworkTimeInterface;
class NetworkRegistrationInterface;

class FakeOfono : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.fakeofono.Manager")

public:
    explicit FakeOfono(QObject *parent = 0);

    struct OfonoModemProperties
    {
        QDBusObjectPath name;
        QMap<QString, QVariant> dict;
    };

    typedef QList<OfonoModemProperties> OfonoModemList;

    void addModem(const QString modemPath);
    void enableInterfaces();
    void emulateNetworkRegistration(const QString mnc, const QString mcc);
    void emulateNetworkTimeChange(qlonglong utc, qlonglong received, int timezone, uint dst,
                                  QString mcc, QString mnc);
signals:
    void ModemAdded(QDBusObjectPath path, QVariantMap properties);
    void ModemRemoved(QDBusObjectPath path);

public slots:
    OfonoModemList GetModems();

private:
    NetworkRegistrationInterface *m_networkRegistration;
    NetworkTimeInterface *m_networkTimeInterface;
    ModemInterface *m_modem;
    QString m_modemPath;
};

Q_DECLARE_METATYPE(FakeOfono::OfonoModemProperties)
Q_DECLARE_METATYPE(FakeOfono::OfonoModemList)
#endif // FAKEOFONO_H
