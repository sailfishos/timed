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

#include <QTest>
#include <QDBusReply>
#include <QDBusError>
#include <QDateTime>
#include <QMap>
#include <QSignalSpy>

#include <timed/interface>
#include <timed/event>

#include "tst_events.h"
#include "simplevolandadaptor.h"

#define APPNAME "tst_events"
#define PATH "/var/tmp/" APPNAME
#define ACTIONSTRING "timestamp: %1, cookie: %2"

tst_Events::tst_Events(QObject *parent) :
    QObject(parent), m_simpleVolandAdaptor(new SimpleVolandAdaptor(this))
{
}

void tst_Events::initTestCase()
{
    if (!Maemo::Timed::Voland::bus().registerService(Maemo::Timed::Voland::service()))
        qWarning() << "Cannot register voland service";

    if (!Maemo::Timed::Voland::bus().registerObject(Maemo::Timed::Voland::objpath(), this))
        qWarning() << "Cannot register voland object";
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

bool tst_Events::addEventWithAction(const qint64 timestamp, const int dueInSeconds, uint &cookie)
{
    // Create an alarm, event in timed lingo, which will go off in 2 seconds
    Maemo::Timed::Event event;
    event.setAttribute("APPLICATION", APPNAME);
    event.setAttribute("TITLE", QString("%1").arg(timestamp));
    event.setAlarmFlag();
    event.setReminderFlag();
    QDateTime dateTime;
    dateTime.setMSecsSinceEpoch(timestamp);
    event.setTicker(dateTime.toTime_t() + dueInSeconds);

    // Add an action to the event, the action writes a message to a file when the event is triggered
    Maemo::Timed::Event::Action &act = event.addAction();
    act.setSendCookieFlag();
    QString message = QString(ACTIONSTRING).arg(timestamp).arg("<COOKIE>");
    act.runCommand(QString("echo -n %1 > %2")
                   .arg(message)
                   .arg(PATH));
    act.whenTriggered();

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

bool tst_Events::verifyVolandDialog(const qint64 timestamp, const QList<QVariant> arguments,
                                        const uint cookie)
{
    return (arguments.count() == 3)
            && arguments.at(0).toUInt() == cookie
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
}

QList<uint> tst_Events::queryEvents()
{
    QMap<QString,QVariant> parameters;
    parameters.insert("APPLICATION", QVariant(QString(APPNAME)));

    Maemo::Timed::Interface timedIface;
    QDBusReply<QList<QVariant> > queryReply = timedIface.query_sync(parameters);
    QList<uint> cookieList;
    if(!queryReply.isValid()) {
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
    QVERIFY2(addEventWithAction(timestamp, 2, cookie),
             "Adding event failed, DBus error");
    QVERIFY2(cookie != 0, "Adding event failed, cookie is 0");

    QList<uint> cookieList = queryEvents();
    QVERIFY(cookieList.count() == 1
            && cookieList.contains(cookie));

    // The event should be triggered while waiting
    QTest::qWait(2200);

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

    QVERIFY2(addEventWithAction(timestamp, 1, cookie), "Adding event failed, DBus error");
    QVERIFY2(addEventWithAction(timestamp2, 2, cookie2), "Adding event failed, DBus error");
    QVERIFY2(addEventWithAction(timestamp3, 3, cookie3), "Adding event failed, DBus error");
    QVERIFY2(cookie != 0, "Adding event failed, cookie is 0");
    QVERIFY2(cookie2 != 0, "Adding event failed, cookie is 0");
    QVERIFY2(cookie3 != 0, "Adding event failed, cookie is 0");

    QVERIFY(spy.count() == 0);

    QList<uint> cookieList = queryEvents();
    QVERIFY(cookieList.count() == 3
            && cookieList.contains(cookie)
            && cookieList.contains(cookie2)
            && cookieList.contains(cookie3));

    QVERIFY(cancelEvent(cookie2));

    cookieList = queryEvents();
    QVERIFY(cookieList.count() == 2
            && cookieList.contains(cookie)
            && cookieList.contains(cookie3));

    QVERIFY(spy.count() == 0); // make sure that the events have not been triggered yet

    QTest::qWait(1200); // let the first event trigger

    QVERIFY(spy.count() == 1);
    QVERIFY(verifyVolandDialog(timestamp, spy.at(0), cookie));
    QByteArray byteArray = readFile(PATH);
    QVERIFY(byteArray == QString(ACTIONSTRING).arg(timestamp).arg(cookie));

    QTest::qWait(2200); // wait for the remaining event

    QVERIFY(spy.count() == 2);
    QVERIFY(verifyVolandDialog(timestamp, spy.at(0), cookie));
    QVERIFY(verifyVolandDialog(timestamp3, spy.at(1), cookie3));
    byteArray = readFile(PATH);
    QVERIFY(byteArray == QString(ACTIONSTRING).arg(timestamp3).arg(cookie3));

    cookieList = queryEvents();
    QVERIFY(cookieList.count() == 2
            && cookieList.contains(cookie)
            && cookieList.contains(cookie3));

    QVERIFY2(dismissEvent(cookie), "Failed to dismiss event");
    QVERIFY2(dismissEvent(cookie3), "Failed to dismiss event");
}

void tst_Events::cleanupTestCase()
{
    cancelAllEvents();

    if (!Maemo::Timed::Voland::bus().unregisterService(Maemo::Timed::Voland::service()))
        qWarning() << "Cannot unregister voland service";

    Maemo::Timed::Voland::bus().unregisterObject(Maemo::Timed::Voland::objpath());

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
