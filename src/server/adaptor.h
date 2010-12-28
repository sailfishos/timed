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
#ifndef ADAPTOR_H
#define ADAPTOR_H

#include <sys/types.h>
#include <unistd.h>

#include <sstream>

#include <QList>
#include <QString>
#include <QDBusAbstractAdaptor>
#include <QDBusMessage>

#include "timed/event-io.h"
#include "timed/wall-info.h"
#include "timed/wall-settings.h"

#include "timed.h"
#include "misc.h"
#include "credentials.h"

#include <timed/interface> // TODO: is Maemo::Timed::bus() the correct way?

class com_nokia_time : public QDBusAbstractAdaptor
{
  Q_OBJECT ;
  Q_CLASSINFO("D-Bus Interface", "com.nokia.time") ;
  Timed *timed ;

public:
  com_nokia_time(Timed *parent) : QDBusAbstractAdaptor(parent), timed(parent)
  {
    setAutoRelaySignals(true) ;
  }
  // void emit_settings(Maemo::Timed::WallClock::wall_info_pimple_t *p) { emit settings_changed(p) ; }

signals:
  void settings_changed(const Maemo::Timed::WallClock::Info &, bool) ;
  void settings_changed_1(bool) ;

public slots:
  Maemo::Timed::WallClock::Info get_wall_clock_info()
  {
    log_debug() ;
    return timed->settings->get_wall_clock_info(nanotime_t()) ;
  }

  bool wall_clock_settings(const Maemo::Timed::WallClock::wall_settings_pimple_t &p)
  {
    log_debug() ;
    // log_debug("%s", string_std_to_q(p.str()).c_str()) ;
    return timed->settings->wall_clock_settings(p) ;
  }

  uint add_event(const Maemo::Timed::event_io_t &x, const QDBusMessage &message)
  {
    // TODO: here we're not asking about credentials immediately
    //       because an event could contain empty action set
    //       --> forwarding the QDBusMessage to Timed::add_even
    //       -->                        then to machine::add_event
    log_debug() ;
    return timed->add_event(cookie_t(), x, message).value() ;
  }

  void add_events(const Maemo::Timed::event_list_io_t &lst, const QDBusMessage &message, QList<QVariant> &res)
  {
    log_debug() ;
    // TODO: is Maemo::Timed::bus() the correct way?
    //       yes, but:
    //       Let's see, if there is a better way
    timed->add_events(lst, res, message) ;
  }

  uint replace_event(const Maemo::Timed::event_io_t &x, uint old, const QDBusMessage &message)
  {
    log_debug() ;
    return timed->add_event(cookie_t(old), x, message).value() ;
  }

  bool dialog_response(uint cookie, int value)
  {
    log_debug() ;
    return timed->dialog_response(cookie_t(cookie), value) ;
  }

  void query(const QMap<QString,QVariant> &words, QList<QVariant> &res)
  {
    log_debug() ;
    timed->am->query(words, res) ;
  }

  void query_attributes(uint cookie, QMap<QString,QVariant> &a)
  {
    log_debug() ;
    timed->am->get_event_attributes(cookie_t(cookie), a) ;
  }

  bool cancel(uint cookie)
  {
    log_debug() ;
    return timed->cancel(cookie_t(cookie)) ;
  }

  QString ping()
  {
    log_debug() ;
    iodata::record *r = timed->am->save(false) ; // false = not for backup
    std::ostringstream s ;
    s << *r ;
    delete r ;
    return string_std_to_q(s.str()) ;
  }

  QString parse(QString text)
  {
    log_debug() ;
    return iodata::parse_and_print(text) ;
  }

  int pid()
  {
    log_debug() ;
    return getpid() ;
  }

  void enable_alarms(bool enable)
  {
    log_debug() ;
    timed->settings->alarms_are_enabled = enable ;
    timed->alarm_gate(enable) ;
  }

  bool alarms_enabled()
  {
    log_debug() ;
    return timed->settings->alarms_are_enabled ;
  }

  bool set_default_snooze(int value)
  {
    log_debug() ;
    return timed->default_snooze(value)==value ;
  }

  int get_default_snooze()
  {
    log_debug() ;
    return timed->default_snooze(0) ;
  }

  void halt(const QString &what)
  {
    log_info("halt('%s') requested, going to sleep soon...", what.toStdString().c_str()) ;
    timed->halt(string_q_to_std(what)) ;
  }

  bool fake_nitz_signal(int mcc, int offset, int time, int dst)
  {
    log_debug("(fake) mcc=%d offset=%d time=%d dst=%d", mcc, offset, time, dst) ;
    cellular_handler::object()->fake_nitz_signal(mcc, offset, time, dst) ;
    return true ; // TODO make above method returning bool (not void) and check parameters
  }

  bool fake_operator_signal(const QString &mcc, const QString &mnc)
  {
    log_debug("(fake) mcc='%s' mnc='%s'", mcc.toStdString().c_str(), mnc.toStdString().c_str()) ;
    cellular_handler::object()->new_operator(mcc, mnc) ;
    return true ;
  }
} ;

#endif
