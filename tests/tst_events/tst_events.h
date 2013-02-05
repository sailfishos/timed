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

namespace Maemo {
    namespace Timed {
        class Event;
    }
}

class tst_Events : public QObject
{
    Q_OBJECT
public:
    explicit tst_Events(QObject *parent = 0);

private slots:
    void initTestCase();
    void test_addEvent();
    void test_replaceEvent();
    void test_cancelEvent();
    void cleanupTestCase();
    void init();
    void cleanup();

private:
    QByteArray readFile(const QString path);
    Maemo::Timed::Event createEvent(const qint64 timestamp, const int dueInSeconds);
    bool addEventWithAction(const qint64 timestamp, const int dueInSeconds, uint &cookie);
    bool replaceEvent(const uint oldCookie, const qint64 timestamp, const int dueInSeconds, uint &cookie);
    bool verifyVolandDialog(const qint64 timestamp, const QList<QVariant> arguments, const uint cookie);
    bool dismissEvent(const uint cookie);
    bool cancelEvent(const uint cookie);
    void removeActionOutputFile();
    void cancelAllEvents();
    QList<uint> queryEvents();

    SimpleVolandAdaptor *m_simpleVolandAdaptor;
};
#endif // TST_EVENTS
