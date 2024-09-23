/***************************************************************************
**                                                                        **
**   Copyright (c) 2009-2011 Nokia Corporation.                           **
**   Copyright (C) 2013-2019 Jolla Ltd.                                   **
**   Copyright (c) 2019 Open Mobile Platform LLC.                         **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
**   Author: Victor Portnov <ext-victor.portnov@nokia.com>                **
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
#ifndef MAEMO_TIMED_INTERFACE_H
#define MAEMO_TIMED_INTERFACE_H

#include <stdint.h>

#include <QDBusAbstractInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QMap>
#include <QString>
#include <QVariant>

#include "event-declarations.h"
#include "qmacro.h"
#include "wall-declarations.h"

typedef QList<uint> Q_List_uint;
typedef QMap<QString, QString> Q_Map_String_String;
typedef QMap<uint, QMap<QString, QString>> Q_Map_uint_String_String;

Q_DECLARE_METATYPE(Q_List_uint);
Q_DECLARE_METATYPE(Q_Map_String_String);
Q_DECLARE_METATYPE(Q_Map_uint_String_String);

register_qtdbus_metatype(Maemo::Timed::Event::Triggers, 11);

namespace Maemo {
namespace Timed {

inline const char *service()
{
    return "com.nokia.time";
}
inline const char *interface()
{
    return "com.nokia.time";
}
inline const char *objpath()
{
    return "/com/nokia/time";
}
inline QDBusConnection bus()
{
    return QDBusConnection::systemBus();
}
// struct signal_receiver ;
class Interface : public QDBusAbstractInterface
{
    Q_OBJECT;
signals:
    void settings_changed(const Maemo::Timed::WallClock::Info &info, bool time_changed);
    void alarm_present_changed(bool present);
    void alarm_triggers_changed(Maemo::Timed::Event::Triggers);
    // void settings_changed_1(bool time_changed) ;
public:
    Interface(QObject *parent = NULL);
    // -- dbus signals -- //
    bool settings_changed_connect(QObject *object, const char *slot);
    bool settings_changed_disconnect(QObject *object, const char *slot);
    bool alarm_present_changed_connect(QObject *object, const char *slot);
    bool alarm_present_changed_disconnect(QObject *object, const char *slot);
    bool alarm_triggers_changed_connect(QObject *object, const char *slot);
    bool alarm_triggers_changed_disconnect(QObject *object, const char *slot);
    // -- application methods -- //
    qtdbus_method(get_wall_clock_info, (void) );
    qtdbus_method(wall_clock_settings,
                  (const Maemo::Timed::WallClock::Settings &s),
                  s.dbus_output(__PRETTY_FUNCTION__));
    qtdbus_method(add_event, (const Maemo::Timed::Event &e), e.dbus_output(__PRETTY_FUNCTION__));
    qtdbus_method(add_events, (const Maemo::Timed::Event::List &ee), ee.dbus_output());
    qtdbus_method(get_event, (uint32_t cookie), cookie);
    qtdbus_method(get_events, (const QList<uint> &cookies), QVariant::fromValue(cookies));
    qtdbus_method(cancel, (uint32_t cookie), cookie);
    qtdbus_method(cancel_events, (const QList<uint> &cookies), QVariant::fromValue(cookies));
    qtdbus_method(dismiss, (uint32_t cookie), cookie);
    qtdbus_method(replace_event,
                  (const Maemo::Timed::Event &e, uint32_t cookie),
                  e.dbus_output(__PRETTY_FUNCTION__),
                  cookie);
    qtdbus_method(query, (const QMap<QString, QVariant> &attr), QVariant::fromValue(attr));
    qtdbus_method(query_attributes, (uint32_t cookie), cookie);
    qtdbus_method(get_attributes_by_cookie, (uint32_t cookie), cookie);
    qtdbus_method(get_attributes_by_cookies,
                  (const QList<uint> cookies),
                  QVariant::fromValue(cookies));
    qtdbus_method(get_cookies_by_attributes, (const Q_Map_String_String &a), QVariant::fromValue(a));
    qtdbus_method(enable_alarms, (bool enable), enable);
    qtdbus_method(alarms_enabled, (void) );
    qtdbus_method(set_default_snooze, (int value), value);
    qtdbus_method(get_default_snooze, (void) );
    qtdbus_method(set_app_snooze, (const QString &name, int value), name, value);
    qtdbus_method(get_app_snooze, (const QString &name), name);
    qtdbus_method(remove_app_snooze, (const QString &name), name);
    qtdbus_method(get_alarm_present, (void) );
    qtdbus_method(get_alarm_triggers, (void) );
    // -- to be called by voland -- //
    qtdbus_method(dialog_response, (uint32_t cookie, int button), cookie, button);
    // -- not documented stuff -- //
    qtdbus_method(mode, (const QString &mode), mode);
    qtdbus_method(ping, (void) );
    qtdbus_method(pid, (void) );
    qtdbus_method(parse, (const QString &text), text);
    qtdbus_method(customization_values, (void) );
#if __ENABLE_TEST_DBUS_INTERFACE__
    qtdbus_method(fake_nitz_signal, (int mcc, int offset, int time, int dst), mcc, offset, time, dst);
    qtdbus_method(fake_operator_signal, (const QString &mcc, const QString &mnc), mcc, mnc);
    qtdbus_method(fake_csd_time_signal,
                  (const QString &mcc,
                   const QString &mnc,
                   int offset,
                   int time,
                   int dst,
                   int seconds,
                   int nano_seconds),
                  mcc,
                  mnc,
                  offset,
                  time,
                  dst,
                  seconds,
                  nano_seconds);
    qtdbus_method(fake_csd_time_signal_now,
                  (const QString &mcc, const QString &mnc, int offset, int time, int dst),
                  mcc,
                  mnc,
                  offset,
                  time,
                  dst);
#endif // __ENABLE_TEST_DBUS_INTERFACE__
};
} // namespace Timed
} // namespace Maemo

#endif
