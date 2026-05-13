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

#ifndef TST_EVENTS
#define TST_EVENTS

#include <QObject>

class SimpleVolandAdaptor;
class SimpleAlarmUiAdaptor;

namespace Maemo {
namespace Timed {
class Event;
}
} // namespace Maemo

class tst_Events : public QObject
{
    Q_OBJECT
public:
    explicit tst_Events(QObject *parent = 0);

private slots:
    void initTestCase();
    void test_addEvent();
    void test_upcomingAlarmPreAlerts();
    void test_alarmOccurrenceActions();
    void test_replaceEvent();
    void test_cancelEvent();
    void cleanupTestCase();
    void init();
    void cleanup();

private:
    QByteArray readFile(const QString path);
    Maemo::Timed::Event createEvent(const qint64 timestamp, const int dueInSeconds);
    Maemo::Timed::Event createClockEvent(const int dueInSeconds, const int snoozeSeconds = 120);
    Maemo::Timed::Event createCalendarEvent(const int dueInSeconds, const int snoozeSeconds = 120);
    bool addEvent(const qint64 timestamp, const int dueInSeconds, uint &cookie);
    bool addClockEvent(const int dueInSeconds, uint &cookie, const int snoozeSeconds = 120);
    bool addCalendarEvent(const int dueInSeconds, uint &cookie, const int snoozeSeconds = 120);
    bool replaceEvent(const uint oldCookie,
                      const qint64 timestamp,
                      const int dueInSeconds,
                      uint &cookie);
    bool verifyVolandDialog(const qint64 timestamp,
                            const QList<QVariant> arguments,
                            const uint cookie);
    bool dismissEvent(const uint cookie);
    bool cancelEvent(const uint cookie);
    bool triggerForCookie(const uint cookie, uint &trigger);
    void removeActionOutputFile();
    void cancelAllEvents();
    QList<uint> queryEvents();
    QList<uint> queryEvents(const QString &application);

    SimpleVolandAdaptor *m_simpleVolandAdaptor;
    SimpleAlarmUiAdaptor *m_simpleAlarmUiAdaptor;
};
#endif // TST_EVENTS
