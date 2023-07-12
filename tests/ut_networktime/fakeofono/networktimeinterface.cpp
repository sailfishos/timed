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

#include "networktimeinterface.h"

NetworkTimeInterface::NetworkTimeInterface(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{}

void NetworkTimeInterface::emulateNetworkTimeChange(qlonglong utc,
                                                    qlonglong received,
                                                    int timezone,
                                                    uint dst,
                                                    QString mcc,
                                                    QString mnc,
                                                    QString modem)
{
    m_utc = utc;
    m_received = received;
    m_timezone = timezone;
    m_dst = dst;
    m_mcc = mcc;
    m_mnc = mnc;
    m_modem = modem;
    emit NetworkTimeChanged(encode());
}

QVariantMap NetworkTimeInterface::GetNetworkTime()
{
    return encode();
}

QVariantMap NetworkTimeInterface::encode()
{
    QVariantMap map;
    if (m_mcc.isEmpty())
        return map;

    map.insert("UTC", QVariant(m_utc));
    map.insert("Received", QVariant(m_received));
    map.insert("Timezone", QVariant(m_timezone));
    map.insert("DST", QVariant(m_dst));
    map.insert("MobileCountryCode", QVariant(m_mcc));
    map.insert("MobileNetworkCode", QVariant(m_mnc));
    map.insert("ModemPath", QVariant(m_modem));
    return map;
}
