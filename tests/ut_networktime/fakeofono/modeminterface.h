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

#ifndef MODEMINTERFACE_H
#define MODEMINTERFACE_H

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QStringList>
#include <QVariant>

class ModemInterface : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.fakeofono.Modem")

public:
    explicit ModemInterface(QObject *parent = 0);

    void addInterface(const QString interface);

signals:
    void PropertyChanged(QString name, QDBusVariant value);

public slots:
    QVariantMap GetProperties();

private:
    QStringList m_interfaceList;
};

#endif // MODEMINTERFACE_H
