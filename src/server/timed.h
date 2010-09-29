/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
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
#ifndef TIMED_H
#define TIMED_H

#include <QCoreApplication>
#include <QMetaMethod>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

#include <iodata/storage>

#include "states.h"
#include "wrappers.h"
#include "singleshot.h"
#include "pinguin.h"
#include "settings.h"
#include "unix-signal.h"
#include "onitz.h"
#include "olson.h"
#include "tz.h"

struct Timed : public QCoreApplication
{
  machine *am ;
  pinguin *ping ;
  source_settings *settings ;
  customization_settings *cust_settings;

  void load_events() ;
  void check_voland_service() ;
  cookie_t add_event(cookie_t remove, const Maemo::Timed::event_io_t &event, const QDBusMessage &message) ;
  void add_events(const Maemo::Timed::event_list_io_t &events, QList<QVariant> &res, const QDBusMessage &message) ;
  bool dialog_response(cookie_t c, int value) ;
  bool cancel(cookie_t c) { return am->cancel(c) ; }
  bool alarm_gate(bool set=false, bool value=true) { return am->alarm_gate(set, value) ; }
  int default_snooze(int value) { return am->default_snooze(value) ; }
  QDBusConnectionInterface *ses_iface ;

  map<int,unsigned> children ;

  void backup();
  void backup_finished();
  void restore();
  void restore_finished();
public Q_SLOTS:
  void system_owner_changed(const QString &name, const QString &oldowner, const QString &newowner) ;
  void send_next_bootup_event(int value) ;
  void register_child(unsigned cookie, int pid) { children[pid] = cookie ; }
Q_SIGNALS:
  void voland_registered() ;
  void voland_unregistered() ;
  void settings_changed(const Maemo::Timed::WallClock::Info &, bool system_time) ;
  // void settings_changed_1(bool system_time) ;
public:
  Timed(int ac, char **av) ;
  const string &get_default_timezone() { return default_timezone ; }
  int get_default_gmt_offset() { return default_gmt_offset ; }

private:
  QDBusServiceWatcher *voland_watcher ;
  iodata::storage *event_storage, *settings_storage, *timed_rc_storage ;

  simple_timer *short_save_threshold_timer, *long_save_threshold_timer ;
  unsigned threshold_period_long, threshold_period_short ;
  unsigned ping_period, ping_max_num ;
  string save_time_path ;
  string events_path, settings_path ;
  string default_timezone ;
  int default_gmt_offset ;
  void load_rc() ;
  void load_settings() ;
  void save_settings() ;
  Q_INVOKABLE void save_event_queue() ;

  queue_pause *q_pause ;
  Q_INVOKABLE void send_time_settings() ;
  bool signal_invoked ;
  nanotime_t systime_back ;
  QTimer *save_time_to_file_timer ;
  tz_oracle_t *tz_oracle ;

  ContextProvider::Property *time_operational_p ;
public:
  void invoke_signal(const nanotime_t &) ;
  void invoke_signal() { nanotime_t zero ; invoke_signal(zero) ; }
  void clear_invokation_flag() { signal_invoked = false ; systime_back.invalidate() ; }
public Q_SLOTS:
  void event_queue_changed() ;
  void save_time_to_file() ;
private Q_SLOTS:
  void queue_threshold_timeout() ;
  void unix_signal(int signo) ;
  void nitz_notification(const cellular_info_t &) ;
  void tz_by_oracle(olson *tz, tz_suggestions_t) ;
public:
  void update_oracle_context(bool set) ;
  void open_epoch() ;
  void halt(const string &what) { halted = what ; exit(0) ; } // exit the main loop
  string is_halted() { return halted ; }
private:
  string halted ;
  UnixSignal *signal_object ;
public:
  Q_OBJECT ;
public:
  bool notify(QObject *obj, QEvent *ev)
  {
    try { return QCoreApplication::notify(obj, ev); }
    catch(const iodata::validator::exception &e)
    {
      log_critical("%s", e.info().c_str()) ;
    }
    catch(const std::exception &e)
    {
      log_critical("oops, unknown std::exception: %s", e.what()) ;
    }
    catch(...)
    {
      log_critical("oops, unknown exception of unknown type ...") ;
    }
    log_critical("aborting...") ;
    abort();
  }
} ;

#endif
