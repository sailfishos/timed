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

#ifndef NTPCONTROLLER_H
#define NTPCONTROLLER_H

#include <QObject>

class QDBusServiceWatcher;
class QDBusPendingCallWatcher;

class NtpController : public QObject
{
    Q_OBJECT

public:
    explicit NtpController(bool enable, QObject *parent = 0);
    void enableNtpTimeAdjustment(bool enable);

public slots:
    void serviceRegistered();
    void propertiesReply(QDBusPendingCallWatcher *call);

private:
    void setConnmanProperty(QString key, QString value);

    bool m_enable;
    QDBusServiceWatcher *m_connmanWatcher;
};

#endif // NTPCONTROLLER_H
