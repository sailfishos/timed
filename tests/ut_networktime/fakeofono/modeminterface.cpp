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

#include "modeminterface.h"

ModemInterface::ModemInterface(QObject *parent) : QDBusAbstractAdaptor(parent)
{
}

void ModemInterface::addInterface(const QString interface)
{
    m_interfaceList.append(interface);
    emit PropertyChanged("Interfaces", QDBusVariant(QVariant(m_interfaceList)));
}

QVariantMap ModemInterface::GetProperties()
{
    QVariantMap props;
    props.insert("Interfaces",QVariant(m_interfaceList));
    return props;
}
