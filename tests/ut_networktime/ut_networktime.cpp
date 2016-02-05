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

#include <QtDebug>
#include <QDateTime>
#include <QSignalSpy>
#include <QTest>
#include <time.h>

#include "ut_networktime.h"
#include "../../src/server/networktime.h"
#include "../../src/server/networkoperator.h"
#include "../../src/server/ofonoconstants.h"
#include "fakeofono/fakeofono.h"

const char *OfonoConstants::OFONO_SERVICE = "org.fakeofono";
const char *OfonoConstants::OFONO_MANAGER_INTERFACE = "org.fakeofono.Manager";
const char *OfonoConstants::OFONO_MANAGER_PATH = "/";
const char *OfonoConstants::OFONO_MODEM_INTERFACE = "org.fakeofono.Modem";
const char *OfonoConstants::OFONO_NETWORKREGISTRATION_INTERFACE = "org.fakeofono.NetworkRegistration";
const char *OfonoConstants::OFONO_NETWORKTIME_INTERFACE = "org.fakeofono.NetworkTime";

Q_DECLARE_METATYPE(NetworkTimeInfo)

ut_networktime::ut_networktime()
{
    qRegisterMetaType<NetworkTimeInfo>("NetworkTimeInfo");
}

void ut_networktime::test_networktimeinfo()
{
    NetworkTimeInfo networkTimeInfo;
    QVERIFY(!networkTimeInfo.isValid());

    QDateTime dateTime = QDateTime::currentDateTimeUtc();
    int daylightAdjustment = 0;
    int offsetFromUtc = 7200;
    qlonglong timestampSeconds = 1234;
    qlonglong timestampNanoSeconds = 5678;
    QString mnc = "1";
    QString mcc = "2";
    QString modem = "modem1";

    networkTimeInfo = NetworkTimeInfo(dateTime, daylightAdjustment, offsetFromUtc, timestampSeconds,
                                      timestampNanoSeconds, mnc, mcc, modem);

    QVERIFY(networkTimeInfo.isValid());
    QVERIFY(networkTimeInfo.dateTime() == dateTime);
    QVERIFY(networkTimeInfo.offsetFromUtc() == offsetFromUtc);
    QVERIFY(networkTimeInfo.daylightAdjustment() == daylightAdjustment);
    QVERIFY(networkTimeInfo.mnc().compare(mnc) == 0);
    QVERIFY(networkTimeInfo.mcc().compare(mcc) == 0);
    QVERIFY(networkTimeInfo.modem().compare(modem) == 0);

    const struct timespec *timestamp = networkTimeInfo.timestamp();
    QVERIFY(timestamp->tv_sec == timestampSeconds);
    QVERIFY(timestamp->tv_nsec == timestampNanoSeconds);
}


bool ut_networktime::verifyNetworkTimeInfo(const NetworkTimeInfo timeInfo, const qlonglong utc,
                                           const qlonglong received, const int offsetFromUtc,
                                           const uint dst, const QString mcc, const QString mnc,
                                           const QString modem)
{
    return timeInfo.isValid()
            && timeInfo.dateTime() == QDateTime::fromMSecsSinceEpoch(utc*1000)
            && timeInfo.timestamp()->tv_sec == received
            && timeInfo.timestamp()->tv_nsec == 0
            && timeInfo.offsetFromUtc() == offsetFromUtc
            && timeInfo.daylightAdjustment() == (int) dst
            && timeInfo.mcc().compare(mcc) == 0
            && timeInfo.mnc().compare(mnc) == 0
            && timeInfo.modem().compare(modem) == 0;
}

void ut_networktime::test_networktime()
{
    FakeOfono fakeOfono;
    NetworkTime networkTime;
    QSignalSpy spy(&networkTime, SIGNAL(timeInfoChanged(NetworkTimeInfo)));
    QVERIFY(!networkTime.isValid());
    QString modem1 = "/fakemodem";
    fakeOfono.addModem(modem1);
    fakeOfono.enableInterfaces(modem1);
    QString modem2 = "/fakemodem2";
    fakeOfono.addModem(modem2);
    fakeOfono.enableInterfaces(modem2);
    QTest::qWait(500);

    QVERIFY(!networkTime.isValid());
    QVERIFY(spy.count() == 0);

    qlonglong m1utc = 1356998400; // 1.1.2013 00:00:00 GMT, in seconds since Unix epoch
    qlonglong m1received = 12345; // represents value of monotonic clock
    int m1offsetFromUtc = 7200; // GMT+2, eg. EET
    uint m1dst = 0;
    QString m1mcc = "m1fakemcc";
    QString m1mnc = "m1fakemnc";

    fakeOfono.emulateNetworkTimeChange(m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1);
    QTest::qWait(500); // Give some time to DBus messaging

    QVERIFY(networkTime.isValid());
    QVERIFY(spy.count() == 1);
    QVERIFY2(verifyNetworkTimeInfo(networkTime.timeInfo(), m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1),
             "NetworkTime::timeInfo() returned an erroneous NetworkTimeInfo object");
    QVERIFY2(verifyNetworkTimeInfo(networkTime.timeInfo(modem1), m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1),
            "NetworkTime::timeInfo(modem1) returned an erroneous NetworkTimeInfo object");
    QCOMPARE(networkTime.defaultModem(), modem1); // the default modem shoudl have been set to modem1

    QSignalSpy querySpy(&networkTime, SIGNAL(timeInfoQueryCompleted(NetworkTimeInfo)));
    networkTime.queryTimeInfo(modem1); // query just for modem1
    QTest::qWait(500); // NetworkTime::queryTimeInfo() is async DBus, wait for reply
    QCOMPARE(querySpy.count(), 1);
    QList<QVariant> arguments = querySpy.takeFirst();
    QVERIFY(arguments.count() == 1);
    QVariant variant = arguments.takeFirst();
    QVERIFY(variant.canConvert<NetworkTimeInfo>());
    NetworkTimeInfo timeInfo = qvariant_cast<NetworkTimeInfo>(variant);
    QVERIFY2(verifyNetworkTimeInfo(timeInfo, m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1),
             "NetworkTime::timeInfoQueryCompleted() signal provided an erronoeus NetworkTimeInfo object");

    // Change time again
    QVERIFY(networkTime.isValid());
    m1utc = 1388448000; // 31.12.2013 00:00:00 GMT, in seconds since Unix epoch
    m1received = 56789; // represents value of monotonic clock
    m1offsetFromUtc = 3600; // GMT+1
    m1dst = 1;
    m1mcc = "m1fakemcc2";
    m1mnc = "m1fakemnc2";
    // modem will stay the same = "/fakemodem"
    fakeOfono.emulateNetworkTimeChange(m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1);
    QTest::qWait(500); // Give some time to DBus messaging

    QVERIFY(spy.count() == 2);
    QVERIFY2(verifyNetworkTimeInfo(networkTime.timeInfo(), m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1),
            "NetworkTime::timeInfo() returned an erroneous NetworkTimeInfo object");
    QVERIFY2(verifyNetworkTimeInfo(networkTime.timeInfo(modem1), m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1),
            "NetworkTime::timeInfo(modem1) returned an erroneous NetworkTimeInfo object");

    QVERIFY(querySpy.count() == 0);
    networkTime.queryTimeInfo(modem1);
    QTest::qWait(500); // NetworkTime::queryTimeInfo() is async DBus, wait for reply
    QVERIFY(querySpy.count() == 1);
    arguments = querySpy.takeFirst();
    QVERIFY(arguments.count() == 1);
    variant = arguments.takeFirst();
    QVERIFY(variant.canConvert<NetworkTimeInfo>());
    timeInfo = qvariant_cast<NetworkTimeInfo>(variant);
    QVERIFY2(verifyNetworkTimeInfo(timeInfo, m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1),
             "NetworkTime::timeInfoQueryCompleted() signal provided an erronoeus NetworkTimeInfo object");

    // check that we do not yet have a valid network time from the second modem
    QVERIFY(!networkTime.isValid(modem2));
    // receive a network time change from the second modem
    qlonglong m2utc = 1356999000; // 1.1.2013 00:10:00 GMT, in seconds since Unix epoch
    qlonglong m2received = 67898; // represents value of monotonic clock
    int m2offsetFromUtc = 36000; // GMT+10
    uint m2dst = 0;
    QString m2mcc = "m2fakemcc";
    QString m2mnc = "m2fakemnc";
    int currSpyCount = spy.count();
    fakeOfono.emulateNetworkTimeChange(m2utc, m2received, m2offsetFromUtc, m2dst, m2mcc, m2mnc, modem2);
    QTest::qWait(500); // Give some time to DBus messaging

    // we should now have a valid network time from the second modem
    QVERIFY(networkTime.isValid(modem2));
    QCOMPARE(spy.count(), currSpyCount+1);
    QVERIFY2(verifyNetworkTimeInfo(networkTime.timeInfo(modem2), m2utc, m2received, m2offsetFromUtc, m2dst, m2mcc, m2mnc, modem2),
             "NetworkTime::timeInfo() returned an erroneous NetworkTimeInfo object");
    // but the default modem should not have changed
    QCOMPARE(networkTime.defaultModem(), modem1);
    // nor should have the time values for modem1
    QVERIFY2(verifyNetworkTimeInfo(networkTime.timeInfo(modem1), m1utc, m1received, m1offsetFromUtc, m1dst, m1mcc, m1mnc, modem1),
            "NetworkTime::timeInfo(modem1) returned an erroneous NetworkTimeInfo object");
}

void ut_networktime::test_networkoperator()
{
    FakeOfono fakeOfono;
    NetworkOperator networkOperator;
    QSignalSpy spy(&networkOperator, SIGNAL(operatorChanged(QString, QString, QString)));
    QVERIFY(!networkOperator.isValid());
    QString modem1 = "/fakemodem1";
    QString m1mcc = "m1fakemcc";
    QString m1mnc = "m1fakemnc";
    fakeOfono.addModem(modem1);
    fakeOfono.enableInterfaces(modem1);
    QTest::qWait(500);
    QVERIFY(spy.count() == 0);
    fakeOfono.emulateNetworkRegistration(modem1, m1mnc, m1mcc);
    QTest::qWait(500);
    QVERIFY(networkOperator.isValid());

    QVERIFY(spy.count() == 1);
    QList<QVariant> arguments = spy.first();
    QVERIFY(arguments.count() == 3);
    QCOMPARE(arguments.at(0).toString(), modem1);
    QCOMPARE(arguments.at(1).toString(), m1mnc);
    QCOMPARE(arguments.at(2).toString(), m1mcc);
    QCOMPARE(modem1, networkOperator.defaultModem());
    QCOMPARE(m1mnc, networkOperator.mnc());
    QCOMPARE(m1mcc, networkOperator.mcc());

    // Make oFono send a duplicate network registration signal
    // this should not make NetworkOperator to react
    // TODO: what does this mean?  Currently, it will still emit the operatorChanged signal
    // because there is no detection of duplicate values in NetworkOperator.cpp
    fakeOfono.emulateNetworkRegistration(modem1, m1mnc, m1mcc);
    QTest::qWait(500);
    QVERIFY(networkOperator.isValid());
    QVERIFY(spy.count() == 2);
    QCOMPARE(modem1, networkOperator.defaultModem());
    QCOMPARE(m1mnc, networkOperator.mnc());
    QCOMPARE(m1mcc, networkOperator.mcc());

    // Emulate operator change
    m1mcc = "m1fakemcc2";
    m1mnc = "m1fakemnc2";
    fakeOfono.emulateNetworkRegistration(modem1, m1mnc, m1mcc);
    QTest::qWait(500);
    QVERIFY(networkOperator.isValid());

    QVERIFY(spy.count() == 3);
    arguments = spy.at(2);
    QVERIFY(arguments.count() == 3);
    QCOMPARE(arguments.at(0).toString(), modem1);
    QCOMPARE(arguments.at(1).toString(), m1mnc);
    QCOMPARE(arguments.at(2).toString(), m1mcc);
    QCOMPARE(modem1, networkOperator.defaultModem());
    QCOMPARE(m1mnc, networkOperator.mnc());
    QCOMPARE(m1mcc, networkOperator.mcc());

    // Register a new operator on a different modem
    QString modem2 = "/fakemodem2";
    QString m2mcc = "fm2fakemcc";
    QString m2mnc = "fm2fakemnc";
    fakeOfono.addModem(modem2);
    fakeOfono.enableInterfaces(modem2);
    fakeOfono.emulateNetworkRegistration(modem2, m2mnc, m2mcc);
    QTest::qWait(500);
    QVERIFY(networkOperator.isValid());
    QVERIFY(spy.count() == 4);
    // the default modem should not have changed, and info for the other modem shouldn't have been lost
    QVERIFY(networkOperator.isValid(modem1));
    QCOMPARE(modem1, networkOperator.defaultModem());
    QCOMPARE(m1mnc, networkOperator.mnc(modem1));
    QCOMPARE(m1mcc, networkOperator.mcc(modem1));
    // however information for the new modem should now be available
    QVERIFY(networkOperator.isValid(modem2));
    QCOMPARE(m2mnc, networkOperator.mnc(modem2));
    QCOMPARE(m2mcc, networkOperator.mcc(modem2));

    // Emulate operator change on modem2
    m2mcc = "m2fakemcc2";
    m2mnc = "m2fakemnc2";
    fakeOfono.emulateNetworkRegistration(modem2, m2mnc, m2mcc);
    QTest::qWait(500);
    QVERIFY(networkOperator.isValid());

    QVERIFY(spy.count() == 5);
    arguments = spy.at(4);
    QVERIFY(arguments.count() == 3);
    QCOMPARE(arguments.at(0).toString(), modem2);
    QCOMPARE(arguments.at(1).toString(), m2mnc);
    QCOMPARE(arguments.at(2).toString(), m2mcc);
    QCOMPARE(m2mnc, networkOperator.mnc(modem2));
    QCOMPARE(m2mcc, networkOperator.mcc(modem2));

    // should not have changed operator for modem1
    QCOMPARE(m1mnc, networkOperator.mnc(modem1));
    QCOMPARE(m1mcc, networkOperator.mcc(modem1));
}

QTEST_MAIN(ut_networktime)
