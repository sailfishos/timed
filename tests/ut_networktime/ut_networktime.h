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

#ifndef UT_NETWORKTIME
#define UT_NETWORKTIME

#include <QObject>

#include "../../src/server/networktimeinfo.h"

class ut_networktime : public QObject
{
    Q_OBJECT

public:
    ut_networktime();

private Q_SLOTS:
    void test_networktime();
    void test_networktimeinfo();
    void test_networkoperator();

private:
    bool verifyNetworkTimeInfo(const NetworkTimeInfo timeInfo, const qlonglong utc,
                               const qlonglong received, const int offsetFromUtc, const uint dst,
                               const QString mcc, const QString mnc);
};
#endif //UT_NETWORKTIME
