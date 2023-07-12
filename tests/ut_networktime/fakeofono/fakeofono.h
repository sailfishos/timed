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

#include <QDBusObjectPath>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

#include "../../../src/server/ofonoconstants.h"

#include "modeminterface.h"
#include "networkregistrationinterface.h"
#include "networktimeinterface.h"

class QDBusInterface;
struct OfonoModemProperties;

class FakeOfono : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.fakeofono.Manager")

public:
    explicit FakeOfono(QObject *parent = 0);
    ~FakeOfono()
    {
        Q_FOREACH (const QString &m, m_modemPaths.keys())
            m_modemPaths[m].cleanup();
    }

    struct OfonoModemProperties
    {
        QDBusObjectPath name;
        QMap<QString, QVariant> dict;
    };

    typedef QList<OfonoModemProperties> OfonoModemList;

    void addModem(const QString modemPath);
    void enableInterfaces(const QString modemPath);
    void emulateNetworkRegistration(const QString modem, const QString mnc, const QString mcc);
    void emulateNetworkTimeChange(qlonglong utc,
                                  qlonglong received,
                                  int timezone,
                                  uint dst,
                                  QString mcc,
                                  QString mnc,
                                  QString modem);
signals:
    void ModemAdded(QDBusObjectPath path, QVariantMap properties);
    void ModemRemoved(QDBusObjectPath path);

public slots:
    OfonoModemList GetModems();

private:
    struct OfonoInfo
    {
        OfonoInfo()
            : dbusParentObject(NULL)
            , networkRegistration(NULL)
            , networkTimeInterface(NULL)
            , modem(NULL)
        {}
        OfonoInfo(QObject *parent)
            : dbusParentObject(new QObject(parent))
            , networkRegistration(new NetworkRegistrationInterface(dbusParentObject))
            , networkTimeInterface(new NetworkTimeInterface(dbusParentObject))
            , modem(new ModemInterface(dbusParentObject))
        {}
        void cleanup()
        {
            delete networkRegistration;
            delete networkTimeInterface;
            delete modem;
            delete dbusParentObject;
        } // not dtor since copies will be destroyed.
        QObject *dbusParentObject;
        NetworkRegistrationInterface *networkRegistration;
        NetworkTimeInterface *networkTimeInterface;
        ModemInterface *modem;
    };
    QMap<QString, OfonoInfo> m_modemPaths;
};

Q_DECLARE_METATYPE(FakeOfono::OfonoModemProperties)
Q_DECLARE_METATYPE(FakeOfono::OfonoModemList)
#endif // FAKEOFONO_H
