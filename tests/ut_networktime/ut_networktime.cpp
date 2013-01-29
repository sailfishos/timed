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

#include <QDateTime>
#include <QSignalSpy>
#include <QTest>
#include <time.h>

#include "ut_networktime.h"
#include "networktime.h"
#include "networkoperator.h"
#include "ofonoconstants.h"
#include "fakeofono.h"

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

    networkTimeInfo = NetworkTimeInfo(dateTime, daylightAdjustment, offsetFromUtc, timestampSeconds,
                                      timestampNanoSeconds, mnc, mcc);

    QVERIFY(networkTimeInfo.isValid());
    QVERIFY(networkTimeInfo.dateTime() == dateTime);
    QVERIFY(networkTimeInfo.offsetFromUtc() == offsetFromUtc);
    QVERIFY(networkTimeInfo.daylightAdjustment() == daylightAdjustment);
    QVERIFY(networkTimeInfo.mnc().compare(mnc) == 0);
    QVERIFY(networkTimeInfo.mcc().compare(mcc) == 0);

    struct timespec *timestamp = networkTimeInfo.timestamp();
    QVERIFY(timestamp->tv_sec == timestampSeconds);
    QVERIFY(timestamp->tv_nsec == timestampNanoSeconds);
}


bool ut_networktime::verifyNetworkTimeInfo(const NetworkTimeInfo timeInfo, const qlonglong utc,
                                           const qlonglong received, const int offsetFromUtc,
                                           const uint dst, const QString mcc, const QString mnc)
{
    return timeInfo.isValid()
            && timeInfo.dateTime() == QDateTime::fromMSecsSinceEpoch(utc*1000)
            && timeInfo.timestamp()->tv_sec == received
            && timeInfo.timestamp()->tv_nsec == 0
            && timeInfo.offsetFromUtc() == offsetFromUtc
            && timeInfo.daylightAdjustment() == (int) dst
            && timeInfo.mcc().compare(mcc) == 0
            && timeInfo.mnc().compare(mnc) == 0;
}

void ut_networktime::test_networktime()
{
    FakeOfono fakeOfono;
    NetworkTime networkTime;
    QSignalSpy spy(&networkTime, SIGNAL(timeInfoChanged(NetworkTimeInfo)));
    QVERIFY(!networkTime.isValid());
    fakeOfono.addModem("/fakemodem");
    fakeOfono.enableInterfaces();
    QTest::qWait(500);

    QVERIFY(!networkTime.isValid());
    QVERIFY(spy.count() == 0);

    qlonglong utc = 1356998400; // 1.1.2013 00:00:00 GMT, in seconds since Unix epoch
    qlonglong received = 12345; // represents value of monotonic clock
    int offsetFromUtc = 7200; // GMT+2, eg. EET
    uint dst = 0;
    QString mcc = "fakemcc";
    QString mnc = "fakemnc";

    fakeOfono.emulateNetworkTimeChange(utc, received, offsetFromUtc, dst, mcc, mnc);
    QTest::qWait(500); // Give some time to DBus messaging

    QVERIFY(networkTime.isValid());
    QVERIFY(spy.count() == 1);
    QVERIFY2(verifyNetworkTimeInfo(networkTime.timeInfo(), utc, received, offsetFromUtc, dst, mcc, mnc),
             "NetworkTime::timeInfo() returned an erroneous NetworkTimeInfo object");

    QSignalSpy querySpy(&networkTime, SIGNAL(timeInfoQueryCompleted(NetworkTimeInfo)));
    networkTime.queryTimeInfo();
    QTest::qWait(500); // NetworkTime::queryTimeInfo() is async DBus, wait for reply
    QVERIFY(querySpy.count() == 1);
    QList<QVariant> arguments = querySpy.takeFirst();
    QVERIFY(arguments.count() == 1);
    QVariant variant = arguments.takeFirst();
    QVERIFY(variant.canConvert<NetworkTimeInfo>());
    NetworkTimeInfo timeInfo = qvariant_cast<NetworkTimeInfo>(variant);
    QVERIFY2(verifyNetworkTimeInfo(timeInfo, utc, received, offsetFromUtc, dst, mcc, mnc),
             "NetworkTime::timeInfoQueryCompleted() signal provided an erronoeus NetworkTimeInfo object");

    // Change time again
    QVERIFY(networkTime.isValid());
    utc = 1388448000; // 31.12.2013 00:00:00 GMT, in seconds since Unix epoch
    received = 56789; // represents value of monotonic clock
    offsetFromUtc = 3600; // GMT+1
    dst = 1;
    mcc = "fakemcc2";
    mnc = "fakemnc2";
    fakeOfono.emulateNetworkTimeChange(utc, received, offsetFromUtc, dst, mcc, mnc);
    QTest::qWait(500); // Give some time to DBus messaging

    QVERIFY(spy.count() == 2);
    QVERIFY2(verifyNetworkTimeInfo(networkTime.timeInfo(), utc, received, offsetFromUtc, dst, mcc, mnc),
            "NetworkTime::timeInfo() returned an erroneous NetworkTimeInfo object");

    QVERIFY(querySpy.count() == 0);
    networkTime.queryTimeInfo();
    QTest::qWait(500); // NetworkTime::queryTimeInfo() is async DBus, wait for reply
    QVERIFY(querySpy.count() == 1);
    arguments = querySpy.takeFirst();
    QVERIFY(arguments.count() == 1);
    variant = arguments.takeFirst();
    QVERIFY(variant.canConvert<NetworkTimeInfo>());
    timeInfo = qvariant_cast<NetworkTimeInfo>(variant);
    QVERIFY2(verifyNetworkTimeInfo(timeInfo, utc, received, offsetFromUtc, dst, mcc, mnc),
             "NetworkTime::timeInfoQueryCompleted() signal provided an erronoeus NetworkTimeInfo object");
}

void ut_networktime::test_networkoperator()
{
    FakeOfono fakeOfono;
    NetworkOperator networkOperator;
    QSignalSpy spy(&networkOperator, SIGNAL(operatorChanged(QString, QString)));
    QVERIFY(!networkOperator.isValid());
    fakeOfono.addModem("/fakemodem");
    fakeOfono.enableInterfaces();
    QTest::qWait(500);
    QString mcc = "fakemcc";
    QString mnc = "fakemnc";
    QVERIFY(spy.count() == 0);
    fakeOfono.emulateNetworkRegistration(mnc, mcc);
    QTest::qWait(500);
    QVERIFY(networkOperator.isValid());

    QVERIFY(spy.count() == 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.count() == 2);
    QCOMPARE(arguments.at(0).toString(), mnc);
    QCOMPARE(arguments.at(1).toString(), mcc);
    QCOMPARE(mnc, networkOperator.mnc());
    QCOMPARE(mcc, networkOperator.mcc());

    // Make oFono send a duplicate network registration signal
    // this should not make NetworkOperator to react
    fakeOfono.emulateNetworkRegistration(mnc, mcc);
    QTest::qWait(500);
    QVERIFY(networkOperator.isValid());
    QVERIFY(spy.count() == 1);
    QCOMPARE(mnc, networkOperator.mnc());
    QCOMPARE(mcc, networkOperator.mcc());

    // Emulate operator change
    mcc = "fakemcc2";
    mnc = "fakemnc2";
    fakeOfono.emulateNetworkRegistration(mnc, mcc);
    QTest::qWait(500);
    QVERIFY(networkOperator.isValid());

    QVERIFY(spy.count() == 2);
    arguments = spy.at(1);
    QVERIFY(arguments.count() == 2);
    QCOMPARE(arguments.at(0).toString(), mnc);
    QCOMPARE(arguments.at(1).toString(), mcc);
    QCOMPARE(mnc, networkOperator.mnc());
    QCOMPARE(mcc, networkOperator.mcc());
}

QTEST_MAIN(ut_networktime)
