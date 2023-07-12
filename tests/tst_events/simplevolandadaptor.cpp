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

#include "simplevolandadaptor.h"

SimpleVolandAdaptor::SimpleVolandAdaptor(QObject *parent)
    : Maemo::Timed::Voland::AbstractAdaptor(parent)
{}

bool SimpleVolandAdaptor::open(const Maemo::Timed::Voland::Reminder &data)
{
    QString application, title;
    QMap<QString, QString> attributes = data.attributes();

    foreach (const QString &key, attributes.keys()) {
        if (key.compare("APPLICATION") == 0)
            application = attributes.value(key);
        else if (key.compare("TITLE") == 0)
            title = attributes.value(key);
    }

    emit openAlarmDialog(data.cookie(), application, title);

    return true;
}

bool SimpleVolandAdaptor::open(const QList<QVariant> &data)
{
    foreach (const QVariant &variant, data) {
        QDBusArgument argument = variant.value<QDBusArgument>();
        Maemo::Timed::Voland::Reminder reminder;
        argument >> reminder;
        open(reminder);
    }

    return true;
}

bool SimpleVolandAdaptor::close(uint cookie)
{
    emit closeAlarmDialog(cookie);
    return true;
}
