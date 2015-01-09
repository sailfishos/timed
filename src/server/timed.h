/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2011 Nokia Corporation.                           **
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
#ifndef TIMED_H
#define TIMED_H

#include <QCoreApplication>
#include <QMetaMethod>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <ContextProvider>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <iodata-qt5/validator>
#include <iodata-qt5/storage>
#else
#include <iodata/validator>
#include <iodata/storage>
#endif

#include "wrappers.h"
#include "singleshot.h"
#include "pinguin.h"
#include "settings.h"
#include "unix-signal.h"
#include "onitz.h"
#include "olson.h"
#include "machine.h"
#include "tz.h"
#include "csd.h"
#include "event.h"
#include "peer.h"
#if HAVE_DSME
#include "dsme-mode.h"
#endif
#include "notification.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
namespace statefs {
  namespace qt {
    class InOutWriter;
  }
}
#endif

class NtpController;

struct Timed : public QCoreApplication
{
public:
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  inline const char *configuration_path() { return  "/etc/timed-qt5.rc" ; }
#else
  inline const char *configuration_path() { return  "/etc/timed.rc" ; }
#endif
  // inline const char *configuration_type() { return  "/usr/share/timed/typeinfo/config.type" ; }

  inline const char *customization_path() { return  "/usr/share/timed/customization.data" ; } // TODO: make it configurable
  // inline const char *customization_type() { return  "/usr/share/timed/typeinfo/customization.type" ; }

  // inline const char *settings_file_type() { return  "/usr/share/timed/typeinfo/settings.type" ; }

  // inline const char *event_queue_type() { return  "/usr/share/timed/typeinfo/queue.type" ; }

private:
  bool scratchbox_mode ;

  bool format24_by_default ;
  bool auto_time_by_default ;
  bool guess_tz_by_default ;

  bool nitz_supported ;
  string tz_by_default ;
  bool first_boot_date_adjusted;

public:
  bool is_nitz_supported() { return nitz_supported ; }
  const string &default_timezone() { return tz_by_default ; }
  const QString get_settings_path() { return settings_path; }
  void init_first_boot_hwclock_time_adjustment_check();

private:

  // init_* methods, to be called by constructor only
  void init_unix_signal_handler() ;
  void init_dbus_peer_info() ;
  void init_scratchbox_mode() ;
  void init_act_dead() ;
  void init_configuration() ;
  void init_customization() ;
  void init_read_settings() ;
  void init_create_event_machine() ;
  void init_device_mode() ;
  void init_context_objects() ;
  void init_backup_object() ;
  void init_main_interface_object() ;
  void init_backup_dbus_name() ;
  void init_main_interface_dbus_name() ;
  void init_load_events() ;
  void init_dst_checker() ;
  void init_start_event_machine() ;
  void init_cellular_services() ;
  void init_ntp();
  void init_apply_tz_settings() ;
  void init_kernel_notification() ;

public:
  void stop_machine() ;
  void stop_context() ;
  void stop_stuff() ;
  void stop_dbus() ;

public:

  machine_t *am ;
  pinguin_t *ping ;
  source_settings *settings ;
  cellular_handler *nitz_object ;
#if OFONO
  csd_t *csd ;
#endif
  peer_t *peer ;

  void load_events() ;
  void check_voland_service() ;
  void start_voland_watcher() ;
  void stop_voland_watcher() ;
  cookie_t add_event(cookie_t remove, const Maemo::Timed::event_io_t &event, const QDBusMessage &message) ;
  void add_events(const Maemo::Timed::event_list_io_t &events, QList<QVariant> &res, const QDBusMessage &message) ;
  bool get_event(cookie_t c, Maemo::Timed::event_io_t &res) ;
  bool get_events(const QList<uint> &cookies, Maemo::Timed::event_list_io_t &res) ;
  bool dialog_response(cookie_t c, int value) ;
  bool cancel(cookie_t c) { return am->cancel_by_cookie(c) ; }
  void cancel_events(const QList<uint> &cookies, QList<uint> &failed) { am->cancel_events(cookies, failed) ;}
  void alarm_gate(bool value) { return am->alarm_gate(value) ; }
  int default_snooze(int value) { return settings->default_snooze(value) ; }
  void enable_ntp_time_adjustment(bool enable);

  QDBusConnectionInterface *ses_iface ;

  map<int,unsigned> children ;

public Q_SLOTS:
  void system_owner_changed(const QString &name, const QString &oldowner, const QString &newowner) ;
  void register_child(unsigned cookie, int pid) { children[pid] = cookie ; }
  void session_reported(const QString &address) ;
Q_SIGNALS:
  void voland_registered() ;
  void voland_unregistered() ;
  void settings_changed(const Maemo::Timed::WallClock::Info &, bool system_time) ;
  void next_bootup_event(int next_boot_event, int next_non_boot_event);
  void alarm_triggers_changed(Maemo::Timed::Event::Triggers);
  // void settings_changed_1(bool system_time) ;
public:
  Timed(int ac, char **av) ;
  virtual ~Timed() ;
  int get_default_gmt_offset() { return default_gmt_offset ; }

private:
  QDBusServiceWatcher *voland_watcher ;
  iodata::storage *event_storage, *settings_storage ;

  simple_timer *short_save_threshold_timer, *long_save_threshold_timer ;
  unsigned threshold_period_long, threshold_period_short ;
  unsigned ping_period, ping_max_num ;
  QString data_path, events_path, settings_path;
  int default_gmt_offset ;
#if HAVE_DSME
  dsme_mode_t *dsme_mode_handler ;
#endif
  std::string current_mode ;
  void load_rc() ;
  void load_settings() ;
public:
  void save_settings() ;
private:
  Q_INVOKABLE void save_event_queue() ;

  machine_t::pause_t *q_pause ;
  Q_INVOKABLE void send_time_settings() ;
  bool signal_invoked ;
  nanotime_t systime_back ;
  QTimer *dst_timer ;
  std::string sent_signature ;
  tz_oracle_t *tz_oracle ;
  NtpController *ntp_controller;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  statefs::qt::InOutWriter *alarm_present;
  statefs::qt::InOutWriter *alarm_trigger;
#else
  ContextProvider::Property *time_operational_p ;
  ContextProvider::Property *alarm_present;
  ContextProvider::Property *alarm_trigger;
  ContextProvider::Service *context_service ;
#endif

  QObject *backup_object ;
public:
  kernel_notification_t *notificator ;
  void invoke_signal(const nanotime_t &) ;
  void invoke_signal() { nanotime_t zero=0 ; invoke_signal(zero) ; }
  void clear_invokation_flag() { signal_invoked = false ; systime_back.set(0) ; }
public Q_SLOTS:
  void event_queue_changed() ;
private Q_SLOTS:
  void queue_threshold_timeout() ;
  void unix_signal(int signo) ;
#if HAVE_DSME
  void dsme_mode_reported(const string &mode) ;
  void dsme_mode_is_changing() ;
#endif
  void harmattan_init_done(int runlevel) ;
  void harmattan_desktop_visible() ;
  void kernel_notification(const nanotime_t &jump_forwards) ;
  void restart_alarm_timer();
  void set_alarm_present(bool present);
  void set_alarm_trigger(const QMap<QString, QVariant> &triggers);
public:
  void device_mode_reached(bool user_mode) ;
public Q_SLOTS:
  void check_dst() ;
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
    catch(const iodata::exception &e)
    {
      log_critical("iodata::exception: '%s'", e.info().c_str()) ;
    }
    catch(const event_exception &e)
    {
      log_critical("event_exception: pid=%d, '%s'", e.pid(), e.what()) ;
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
