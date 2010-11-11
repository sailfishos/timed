/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
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

#include <QMap>
#include <QString>
#include <QVariant>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusInterface>

#include <timed/event>
#include <timed/wallclock>
#include <timed/qmacro.h>

namespace Maemo
{
  namespace Timed
  {

    inline const char *service()
    {
      return "com.nokia.time" ;
    }
    inline const char *interface()
    {
      return "com.nokia.time" ;
    }
    inline const char *objpath()
    {
      return "/com/nokia/time" ;
    }
    inline QDBusConnection bus()
    {
      return QDBusConnection::sessionBus() ;
    }
    // struct signal_receiver ;
    class Interface : public QDBusInterface
    {
      Q_OBJECT ;
      // struct signal_receiver *receiver ;
    private slots:
      // void xxx(bool yyy) { /* emit settings_changed_1(yyy) ; */ }
    signals:
       void settings_changed(const Maemo::Timed::WallClock::Info &info, bool time_changed) ;
      // void settings_changed_1(bool time_changed) ;
    public:
      Interface(QObject *parent=NULL) ;
      // -- dbus signals -- //
      bool settings_changed_connect(QObject *object, const char *slot) ;
      bool settings_changed_disconnect(QObject *object, const char *slot) ;
      // -- application methods -- //
      qtdbus_method(get_wall_clock_info, (void)) ;
      qtdbus_method(wall_clock_settings, (const Maemo::Timed::WallClock::Settings &s), s.dbus_output(__PRETTY_FUNCTION__)) ;
      qtdbus_method(add_event, (const Maemo::Timed::Event &e), e.dbus_output(__PRETTY_FUNCTION__)) ;
      qtdbus_method(add_events, (const Maemo::Timed::Event::List &ee), ee.dbus_output()) ;
      qtdbus_method(cancel, (uint32_t cookie), cookie) ;
      qtdbus_method(replace_event, (const Maemo::Timed::Event &e, uint32_t cookie), e.dbus_output(__PRETTY_FUNCTION__), cookie) ;
      qtdbus_method(query, (const QMap<QString,QVariant> &attr), QVariant::fromValue(attr)) ;
      qtdbus_method(query_attributes, (uint32_t cookie), cookie) ;
      qtdbus_method(enable_alarms, (bool enable), enable) ;
      qtdbus_method(alarms_enabled, (void)) ;
      qtdbus_method(set_default_snooze, (int value), value) ;
      qtdbus_method(get_default_snooze, (void)) ;
      // -- to be called by voland -- //
      qtdbus_method(dialog_response, (uint32_t cookie, int button), cookie, button) ;
      // -- not documented stuff -- //
      qtdbus_method(ping, (void)) ;
      qtdbus_method(pid, (void)) ;
      qtdbus_method(parse, (const QString &text), text) ;
      qtdbus_method(customization_values, (void)) ;
      qtdbus_method(fake_nitz_signal, (int mcc, int offset, int time, int dst), mcc, offset, time, dst) ;
    } ;
  }
}

#endif
