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

#ifndef NETWORKTIMEINTERFACE_H
#define NETWORKTIMEINTERFACE_H

#include <QDBusAbstractAdaptor>
#include <QVariant>

class NetworkTimeInterface : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.fakeofono.NetworkTime")

public:
    explicit NetworkTimeInterface(QObject *parent = 0);

    void emulateNetworkTimeChange(qlonglong utc, qlonglong received, int timezone,
                                  uint dst, QString mcc, QString mnc, QString modem);

public slots:
    QVariantMap GetNetworkTime();

signals:
    void NetworkTimeChanged(QVariantMap time);

private:
    qlonglong m_utc;
    qlonglong m_received;
    int m_timezone;
    uint m_dst;
    QString m_mcc;
    QString m_mnc;
    QString m_modem;

    QVariantMap encode();
};
#endif // NETWORKTIMEINTERFACE_H
