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

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QDateTime>
#include <QFile>
#include <QMap>
#include <QSignalSpy>
#include <QTest>

#include "../../src/lib/event-declarations.h"
#include "../../src/lib/interface.h"

#include "simplevolandadaptor.h"
#include "tst_events.h"

#define APPNAME "tst_events"
#define CLOCKAPP "nemoalarms"
#define CALENDARAPP "libextendedkcal"
#define PATH "/var/tmp/" APPNAME
#define ACTIONSTRING "timestamp: %1, cookie: %2"

// Wait until __expr is true, or until the total waiting time reaches __timeout.
// The status of __expr and timeout condition is checked with an interval of 100 ms.
// Finally, wait for an additional 100 ms after __expr is true or __timeout is reached
// to provide time for actions depending on/related to __expr to execute.
#define WAITFOR(__expr, __timeout) \
    do { \
        const int __timeDelta = 100; \
        for (int __i = 0; __i < __timeout && !(__expr); __i += __timeDelta) \
            QTest::qWait(__timeDelta); \
        QTest::qWait(__timeDelta); \
    } while (0);

tst_Events::tst_Events(QObject *parent)
    : QObject(parent)
    , m_simpleVolandAdaptor(new SimpleVolandAdaptor(this))
    , m_simpleAlarmUiAdaptor(new SimpleAlarmUiAdaptor(this))
{}

void tst_Events::initTestCase()
{
    if (!Maemo::Timed::Voland::bus().registerService(Maemo::Timed::Voland::service()))
        qWarning() << "Cannot register voland service";

    if (!Maemo::Timed::Voland::bus().registerObject(Maemo::Timed::Voland::objpath(), this))
        qWarning() << "Cannot register voland object";

    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("com.jolla.alarm.ui")))
        qWarning() << "Cannot register alarm-ui service";

    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/com/jolla/alarm/ui"), this))
        qWarning() << "Cannot register alarm-ui object";
}

QByteArray tst_Events::readFile(const QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to read file" << path;
        return QByteArray();
    }

    QByteArray byteArray;
    while (!file.atEnd())
        byteArray.append(file.readLine());

    return byteArray;
}

Maemo::Timed::Event tst_Events::createEvent(const qint64 timestamp, const int dueInSeconds)
{
    // Create an event, which will go off in dueInSeconds
    Maemo::Timed::Event event;
    event.setAttribute("APPLICATION", APPNAME);
    event.setAttribute("TEST", APPNAME);
    event.setAttribute("TITLE", QString("%1").arg(timestamp));
    event.setAlarmFlag();
    event.setReminderFlag();
    QDateTime dateTime;
    dateTime.setMSecsSinceEpoch(timestamp);
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    event.setTicker(dateTime.toSecsSinceEpoch() + dueInSeconds);
#else
    event.setTicker(dateTime.toTime_t() + dueInSeconds);
#endif

    // Add an action to the event, the action writes a message to a file when the event is triggered
    Maemo::Timed::Event::Action &act = event.addAction();
    act.setSendCookieFlag();
    // Timed will replace <COOKIE> with the cookie of the event that triggers this action
    QString message = QString(ACTIONSTRING).arg(timestamp).arg("<COOKIE>");
    act.runCommand(QString("echo -n %1 > %2").arg(message).arg(PATH));
    act.whenTriggered();

    return event;
}

Maemo::Timed::Event tst_Events::createClockEvent(const int dueInSeconds, const int snoozeSeconds)
{
    Maemo::Timed::Event event;
    event.setAttribute("APPLICATION", CLOCKAPP);
    event.setAttribute("TEST", APPNAME);
    event.setAttribute("type", "clock");
    event.setAttribute("TITLE", "Clock");
    event.setAlarmFlag();
    event.setTimeoutSnooze(snoozeSeconds);
    const QDateTime dateTime = QDateTime::currentDateTime();
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    event.setTicker(dateTime.toSecsSinceEpoch() + dueInSeconds);
#else
    event.setTicker(dateTime.toTime_t() + dueInSeconds);
#endif
    return event;
}

Maemo::Timed::Event tst_Events::createCalendarEvent(const int dueInSeconds, const int snoozeSeconds)
{
    Maemo::Timed::Event event;
    event.setAttribute("APPLICATION", CALENDARAPP);
    event.setAttribute("TEST", APPNAME);
    event.setAttribute("PLUGIN", "libCalendarReminder");
    event.setAttribute("type", "event");
    event.setAttribute("TITLE", "Calendar");
    event.setReminderFlag();
    event.setTimeoutSnooze(snoozeSeconds);
    const QDateTime dateTime = QDateTime::currentDateTime();
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    event.setTicker(dateTime.toSecsSinceEpoch() + dueInSeconds);
#else
    event.setTicker(dateTime.toTime_t() + dueInSeconds);
#endif
    return event;
}

bool tst_Events::addEvent(const qint64 timestamp, const int dueInSeconds, uint &cookie)
{
    Maemo::Timed::Event event = createEvent(timestamp, dueInSeconds);

    // Open a connection to timed and add the event
    Maemo::Timed::Interface timedIface;
    if (!timedIface.isValid()) {
        qWarning() << "Invalid timed interface:" << timedIface.lastError().message();
        return false;
    }

    QDBusReply<uint> reply = timedIface.add_event_sync(event);
    if (!reply.isValid()) {
        qWarning() << "Adding event failed: %1" << reply.error().message();
        return false;
    }
    cookie = reply.value();
    return true;
}

bool tst_Events::addClockEvent(const int dueInSeconds, uint &cookie, const int snoozeSeconds)
{
    Maemo::Timed::Event event = createClockEvent(dueInSeconds, snoozeSeconds);

    Maemo::Timed::Interface timedIface;
    if (!timedIface.isValid()) {
        qWarning() << "Invalid timed interface:" << timedIface.lastError().message();
        return false;
    }

    QDBusReply<uint> reply = timedIface.add_event_sync(event);
    if (!reply.isValid()) {
        qWarning() << "Adding clock event failed:" << reply.error().message();
        return false;
    }

    cookie = reply.value();
    return true;
}

bool tst_Events::addCalendarEvent(const int dueInSeconds, uint &cookie, const int snoozeSeconds)
{
    Maemo::Timed::Event event = createCalendarEvent(dueInSeconds, snoozeSeconds);

    Maemo::Timed::Interface timedIface;
    if (!timedIface.isValid()) {
        qWarning() << "Invalid timed interface:" << timedIface.lastError().message();
        return false;
    }

    QDBusReply<uint> reply = timedIface.add_event_sync(event);
    if (!reply.isValid()) {
        qWarning() << "Adding calendar event failed:" << reply.error().message();
        return false;
    }

    cookie = reply.value();
    return true;
}

bool tst_Events::replaceEvent(const uint oldCookie,
                              const qint64 timestamp,
                              const int dueInSeconds,
                              uint &cookie)
{
    Maemo::Timed::Event event = createEvent(timestamp, dueInSeconds);

    // Open a connection to timed and replace the event
    Maemo::Timed::Interface timedIface;
    if (!timedIface.isValid()) {
        qWarning() << "Invalid timed interface:" << timedIface.lastError().message();
        return false;
    }

    QDBusReply<uint> reply = timedIface.replace_event_sync(event, oldCookie);
    if (!reply.isValid()) {
        qWarning() << "Replace event failed: %1" << reply.error().message();
        return false;
    }
    cookie = reply.value();
    return true;
}

bool tst_Events::verifyVolandDialog(const qint64 timestamp,
                                    const QList<QVariant> arguments,
                                    const uint cookie)
{
    return (arguments.count() == 3) && arguments.at(0).toUInt() == cookie
           && arguments.at(1).toString().compare(QString(APPNAME)) == 0
           && arguments.at(2).toString().compare(QString("%1").arg(timestamp)) == 0;
}

bool tst_Events::dismissEvent(const uint cookie)
{
    Maemo::Timed::Interface timedIface;
    QDBusReply<bool> dialogResponseReply = timedIface.dialog_response_sync(cookie, -2);
    if (!dialogResponseReply.isValid()) {
        qWarning() << "Dismissing event" << cookie << "failed, DBus error"
                   << dialogResponseReply.error().message();
        return false;
    }
    bool retval = dialogResponseReply;
    return retval;
}

bool tst_Events::cancelEvent(const uint cookie)
{
    Maemo::Timed::Interface timedIface;
    QDBusReply<bool> cancelReply = timedIface.cancel_sync(cookie);
    if (!cancelReply.isValid()) {
        qWarning() << "Failed to cancel event, DBus error:" << timedIface.lastError();
        return false;
    }

    bool retval = cancelReply;
    return retval;
}

bool tst_Events::triggerForCookie(const uint cookie, uint &trigger)
{
    Maemo::Timed::Interface timedIface;
    QDBusReply<Maemo::Timed::Event::Triggers> triggerReply = timedIface.get_alarm_triggers_sync();
    if (!triggerReply.isValid()) {
        qWarning() << "Failed to get alarm triggers:" << timedIface.lastError();
        return false;
    }

    const Maemo::Timed::Event::Triggers triggers = triggerReply.value();
    if (!triggers.contains(cookie)) {
        qWarning() << "Alarm trigger not found for cookie" << cookie;
        return false;
    }

    trigger = triggers.value(cookie);
    return true;
}

void tst_Events::removeActionOutputFile()
{
    QFile file(PATH);
    if (!file.exists())
        return;

    if (!file.remove())
        qWarning() << "Unable to remove file" << PATH;
}

void tst_Events::cancelAllEvents()
{
    foreach (const uint &cookie, queryEvents())
        if (!cancelEvent(cookie))
            qWarning() << "Failed to cancel event";

    foreach (const uint &cookie, queryEvents(CLOCKAPP))
        if (!cancelEvent(cookie))
            qWarning() << "Failed to cancel clock event";

    foreach (const uint &cookie, queryEvents(CALENDARAPP))
        if (!cancelEvent(cookie))
            qWarning() << "Failed to cancel calendar event";
}

QList<uint> tst_Events::queryEvents()
{
    return queryEvents(QString(APPNAME));
}

QList<uint> tst_Events::queryEvents(const QString &application)
{
    QMap<QString, QVariant> parameters;
    parameters.insert("APPLICATION", QVariant(application));
    parameters.insert("TEST", QVariant(QString(APPNAME)));

    Maemo::Timed::Interface timedIface;
    QDBusReply<QList<QVariant>> queryReply = timedIface.query_sync(parameters);
    QList<uint> cookieList;
    if (!queryReply.isValid()) {
        qWarning() << "query call failed" << timedIface.lastError();
        return cookieList;
    }
    const QList<QVariant> &result = queryReply.value();
    bool ok;
    foreach (const QVariant &variant, result) {
        cookieList.append(variant.toUInt(&ok));
        if (!ok) {
            qWarning() << "Failed to cast QVariant to uint";
            return QList<uint>();
        }
    }

    return cookieList;
}

void tst_Events::test_addEvent()
{
    qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QSignalSpy spy(m_simpleVolandAdaptor, SIGNAL(openAlarmDialog(uint, QString, QString)));
    uint cookie = 0;
    QVERIFY2(addEvent(timestamp, 2, cookie), "Adding event failed, DBus error");
    QVERIFY2(cookie != 0, "Adding event failed, cookie is 0");

    QList<uint> cookieList = queryEvents();
    QVERIFY(cookieList.count() == 1 && cookieList.contains(cookie));

    // The event should be triggered while waiting
    WAITFOR(spy.count() >= 1, 2200);

    // check action output, the action should have been triggered when event got triggered
    QByteArray byteArray = readFile(PATH);
    QVERIFY(byteArray == QString(ACTIONSTRING).arg(timestamp).arg(cookie));

    // check that timed tells the voland service to display a dialog showing the event
    QVERIFY(spy.count() == 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(verifyVolandDialog(timestamp, arguments, cookie));

    QVERIFY2(dismissEvent(cookie), "Failed to dismiss event");

    QVERIFY(queryEvents().count() == 0);
}

void tst_Events::test_upcomingAlarmPreAlerts()
{
    uint cookie = 0;
    QSignalSpy spy(m_simpleAlarmUiAdaptor,
                   SIGNAL(upcomingAlarm(uint,uint,QString,int,QString)));

    QVERIFY2(addClockEvent(11 * 60, cookie), "Adding clock event failed");
    QVERIFY2(cookie != 0, "Adding clock event failed, cookie is 0");

    WAITFOR(spy.count() >= 1, 1000);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toUInt(), cookie);
    QCOMPARE(spy.at(0).at(3).toInt(), 1);
    QCOMPARE(spy.at(0).at(4).toString(), QStringLiteral("clock"));

    QVERIFY(cancelEvent(cookie));

    cookie = 0;
    spy.clear();

    QVERIFY2(addClockEvent(9 * 60, cookie), "Adding clock event failed");
    QVERIFY2(cookie != 0, "Adding clock event failed, cookie is 0");

    WAITFOR(spy.count() >= 1, 1000);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toUInt(), cookie);
    QCOMPARE(spy.at(0).at(3).toInt(), 2);
    QCOMPARE(spy.at(0).at(4).toString(), QStringLiteral("clock"));

    QVERIFY(cancelEvent(cookie));

    cookie = 0;
    spy.clear();

    QVERIFY2(addCalendarEvent(9 * 60, cookie), "Adding calendar event failed");
    QVERIFY2(cookie != 0, "Adding calendar event failed, cookie is 0");

    WAITFOR(spy.count() >= 1, 1000);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toUInt(), cookie);
    QCOMPARE(spy.at(0).at(3).toInt(), 2);
    QCOMPARE(spy.at(0).at(4).toString(), QStringLiteral("calendar"));

    QVERIFY(cancelEvent(cookie));
}

void tst_Events::test_alarmOccurrenceActions()
{
    const int snoozeSeconds = 120;
    uint cookie = 0;
    QSignalSpy spy(m_simpleAlarmUiAdaptor,
                   SIGNAL(upcomingAlarm(uint,uint,QString,int,QString)));

    QVERIFY2(addClockEvent(60, cookie, snoozeSeconds), "Adding clock event failed");
    QVERIFY2(cookie != 0, "Adding clock event failed, cookie is 0");

    WAITFOR(spy.count() >= 1, 1000);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toUInt(), cookie);
    QCOMPARE(spy.at(0).at(3).toInt(), 2);
    QCOMPARE(spy.at(0).at(4).toString(), QStringLiteral("clock"));

    uint trigger = 0;
    QVERIFY(triggerForCookie(cookie, trigger));

    Maemo::Timed::Interface timedIface;
    QDBusReply<bool> staleReply = timedIface.skip_alarm_occurrence_sync(cookie, trigger + 1);
    QVERIFY(staleReply.isValid());
    QCOMPARE(staleReply.value(), false);
    QVERIFY(queryEvents(CLOCKAPP).contains(cookie));

    QDBusReply<bool> snoozeReply = timedIface.snooze_alarm_occurrence_sync(cookie, trigger);
    QVERIFY(snoozeReply.isValid());
    QCOMPARE(snoozeReply.value(), true);

    WAITFOR(spy.count() >= 2, 1000);

    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toUInt(), cookie);
    QCOMPARE(spy.at(1).at(1).toUInt(), trigger + snoozeSeconds);
    QCOMPARE(spy.at(1).at(3).toInt(), 3);
    QCOMPARE(spy.at(1).at(4).toString(), QStringLiteral("clock"));

    QDBusReply<bool> skipReply = timedIface.skip_alarm_occurrence_sync(cookie,
                                                                       trigger + snoozeSeconds);
    QVERIFY(skipReply.isValid());
    QCOMPARE(skipReply.value(), true);

    WAITFOR(!queryEvents(CLOCKAPP).contains(cookie), 1000);
    QVERIFY(!queryEvents(CLOCKAPP).contains(cookie));
}

// Add three events, modify the middle one, and verify that all events go off
void tst_Events::test_replaceEvent()
{
    qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 timestamp2 = QDateTime::currentDateTime().toMSecsSinceEpoch() + 2;
    qint64 timestamp3 = QDateTime::currentDateTime().toMSecsSinceEpoch() + 3;
    uint cookie = 0;
    uint cookie2 = 0;
    uint cookie3 = 0;

    QSignalSpy spy(m_simpleVolandAdaptor, SIGNAL(openAlarmDialog(uint, QString, QString)));

    QVERIFY2(addEvent(timestamp, 1, cookie), "Adding event failed, DBus error");
    QVERIFY2(addEvent(timestamp2, 2, cookie2), "Adding event failed, DBus error");
    QVERIFY2(addEvent(timestamp3, 3, cookie3), "Adding event failed, DBus error");
    QVERIFY2(cookie != 0, "Adding event failed, cookie is 0");
    QVERIFY2(cookie2 != 0, "Adding event failed, cookie is 0");
    QVERIFY2(cookie3 != 0, "Adding event failed, cookie is 0");

    QVERIFY(spy.count() == 0);

    QList<uint> cookieList = queryEvents();
    QVERIFY(cookieList.count() == 3 && cookieList.contains(cookie) && cookieList.contains(cookie2)
            && cookieList.contains(cookie3));

    qint64 newTimestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    uint newCookie = 0;

    QVERIFY(replaceEvent(cookie2, newTimestamp, 2, newCookie));

    cookieList = queryEvents();
    QVERIFY(cookieList.count() == 3 && cookieList.contains(cookie) && cookieList.contains(cookie3)
            && cookieList.contains(newCookie));

    QVERIFY(spy.count() == 0); // make sure that the events have not been triggered yet

    WAITFOR(spy.count() >= 1, 1500);

    QVERIFY(spy.count() == 1);
    QVERIFY(verifyVolandDialog(timestamp, spy.at(0), cookie));
    QByteArray byteArray = readFile(PATH);
    QVERIFY(byteArray == QString(ACTIONSTRING).arg(timestamp).arg(cookie));

    WAITFOR(spy.count() >= 2, 2200);

    QVERIFY(spy.count() == 2);
    QVERIFY(verifyVolandDialog(newTimestamp, spy.at(1), newCookie));
    byteArray = readFile(PATH);
    QVERIFY(byteArray == QString(ACTIONSTRING).arg(newTimestamp).arg(newCookie));

    WAITFOR(spy.count() >= 3, 2200);

    QVERIFY(spy.count() == 3);
    QVERIFY(verifyVolandDialog(timestamp3, spy.at(2), cookie3));
    byteArray = readFile(PATH);
    QVERIFY(byteArray == QString(ACTIONSTRING).arg(timestamp3).arg(cookie3));

    cookieList = queryEvents();
    QVERIFY(cookieList.count() == 3 && cookieList.contains(cookie) && cookieList.contains(cookie3)
            && cookieList.contains(newCookie));

    QVERIFY2(dismissEvent(cookie), "Failed to dismiss event");
    QVERIFY2(dismissEvent(cookie3), "Failed to dismiss event");
    QVERIFY2(dismissEvent(newCookie), "Failed to dismiss event");
}

// Adds three events, cancels one, and verifies that the two remaining events are triggered
void tst_Events::test_cancelEvent()
{
    qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 timestamp2 = QDateTime::currentDateTime().toMSecsSinceEpoch() + 2;
    qint64 timestamp3 = QDateTime::currentDateTime().toMSecsSinceEpoch() + 3;
    uint cookie = 0;
    uint cookie2 = 0;
    uint cookie3 = 0;

    QSignalSpy spy(m_simpleVolandAdaptor, SIGNAL(openAlarmDialog(uint, QString, QString)));

    QVERIFY2(addEvent(timestamp, 1, cookie), "Adding event failed, DBus error");
    QVERIFY2(addEvent(timestamp2, 2, cookie2), "Adding event failed, DBus error");
    QVERIFY2(addEvent(timestamp3, 3, cookie3), "Adding event failed, DBus error");
    QVERIFY2(cookie != 0, "Adding event failed, cookie is 0");
    QVERIFY2(cookie2 != 0, "Adding event failed, cookie is 0");
    QVERIFY2(cookie3 != 0, "Adding event failed, cookie is 0");

    QVERIFY(spy.count() == 0);

    QList<uint> cookieList = queryEvents();
    QVERIFY(cookieList.count() == 3 && cookieList.contains(cookie) && cookieList.contains(cookie2)
            && cookieList.contains(cookie3));

    QVERIFY(cancelEvent(cookie2));

    cookieList = queryEvents();
    QVERIFY(cookieList.count() == 2 && cookieList.contains(cookie) && cookieList.contains(cookie3));

    QVERIFY(spy.count() == 0); // make sure that the events have not been triggered yet

    WAITFOR(spy.count() >= 1, 1200); // let the first event trigger

    QVERIFY(spy.count() == 1);
    QVERIFY(verifyVolandDialog(timestamp, spy.at(0), cookie));
    QByteArray byteArray = readFile(PATH);
    QVERIFY(byteArray == QString(ACTIONSTRING).arg(timestamp).arg(cookie));

    WAITFOR(spy.count() >= 2, 2200); // wait for the remaining event

    QVERIFY(spy.count() == 2);
    QVERIFY(verifyVolandDialog(timestamp, spy.at(0), cookie));
    QVERIFY(verifyVolandDialog(timestamp3, spy.at(1), cookie3));
    byteArray = readFile(PATH);
    QVERIFY(byteArray == QString(ACTIONSTRING).arg(timestamp3).arg(cookie3));

    cookieList = queryEvents();
    QVERIFY(cookieList.count() == 2 && cookieList.contains(cookie) && cookieList.contains(cookie3));

    QVERIFY2(dismissEvent(cookie), "Failed to dismiss event");
    QVERIFY2(dismissEvent(cookie3), "Failed to dismiss event");
}

void tst_Events::cleanupTestCase()
{
    cancelAllEvents();

    if (!Maemo::Timed::Voland::bus().unregisterService(Maemo::Timed::Voland::service()))
        qWarning() << "Cannot unregister voland service";

    Maemo::Timed::Voland::bus().unregisterObject(Maemo::Timed::Voland::objpath());
    QDBusConnection::sessionBus().unregisterObject(QStringLiteral("/com/jolla/alarm/ui"));
    QDBusConnection::sessionBus().unregisterService(QStringLiteral("com.jolla.alarm.ui"));

    removeActionOutputFile();
}

void tst_Events::init()
{
    // Remove events from previous test runs/cases
    cancelAllEvents();

    // Remove file that may have been left over from previous test run/case
    removeActionOutputFile();
}

void tst_Events::cleanup()
{
    // Cancel all events programmed by this application,
    // there may be leftovers if tests fail
    cancelAllEvents();

    removeActionOutputFile();
}
QTEST_MAIN(tst_Events)
