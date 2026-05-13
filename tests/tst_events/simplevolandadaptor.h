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

#ifndef SIMPLEVOLANDADAPTOR_H
#define SIMPLEVOLANDADAPTOR_H

#include "../../src/voland/interface.h"

#include <QDBusAbstractAdaptor>

/** Minimalistic Voland service
  *
  * Emits the openAlarmDialog() signal when timed asks the service to
  * show event data, and emits the close() signal when timed instructs
  * to close a dialog.
  */
class SimpleVolandAdaptor : public Maemo::Timed::Voland::AbstractAdaptor
{
    Q_OBJECT
public:
    SimpleVolandAdaptor(QObject *parent);

    virtual bool open(const Maemo::Timed::Voland::Reminder &data);
    virtual bool open(const QList<QVariant> &data);
    virtual bool close(uint cookie);

signals:
    void openAlarmDialog(uint cookie, QString application, QString title);
    void closeAlarmDialog(uint cookie);
};

class SimpleAlarmUiAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.jolla.alarm.ui")

public:
    SimpleAlarmUiAdaptor(QObject *parent);

public slots:
    bool publishUpcomingClockAlarm(uint cookie, uint trigger, const QString &title, int stage);
    bool publishUpcomingAlarm(uint cookie,
                              uint trigger,
                              const QString &title,
                              int stage,
                              const QString &alarmType);

signals:
    void upcomingClockAlarm(uint cookie, uint trigger, QString title, int stage);
    void upcomingAlarm(uint cookie, uint trigger, QString title, int stage, QString alarmType);
};

#endif // SIMPLEVOLANDADAPTOR_H
