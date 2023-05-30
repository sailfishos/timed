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

#include "networkregistrationinterface.h"

NetworkRegistrationInterface::NetworkRegistrationInterface(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{}

void NetworkRegistrationInterface::emulateNetworkRegistration(const QString mnc, const QString mcc)
{
    m_mcc = mcc;
    m_mnc = mnc;
    emit PropertyChanged("MobileCountryCode", QDBusVariant(QVariant(m_mcc)));
    emit PropertyChanged("MobileNetworkCode", QDBusVariant(QVariant(m_mnc)));
}

QVariantMap NetworkRegistrationInterface::GetProperties()
{
    QVariantMap map;
    if (!m_mnc.isEmpty() && !m_mcc.isEmpty()) {
        map.insert("MobileNetworkCode", QVariant(m_mnc));
        map.insert("MobileCountryCode", QVariant(m_mcc));
    }
    return map;
}
