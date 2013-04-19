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
#define _BSD_SOURCE

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QFile>
#include <QDateTime>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
// TODO: add Qt5 replacement for ContextProvider
#else
#include <ContextProvider>
#endif

#include "../voland/interface.h"

#include "queue.type.h"
#include "config.type.h"
#include "customization.type.h"
#include "settings.type.h"

#if HAVE_DSME
#include "interfaces.h"
#endif
#include "adaptor.h"
#include "backup.h"
#include "timed.h"
#include "settings.h"
#include "tz.h"
#include "tzdata.h"
#include "csd.h"
#include "notification.h"
#include "time.h"
#include "../common/log.h"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

static void spam()
{
#if NO_SPAM
  time_t now = time(NULL) ;
  for(int i=0; i<12; ++i)
  {
    time_t then = now + i*30*24*60*60 ;
    struct tm t ;
    localtime_r(&then, &t) ;
    log_debug("i=%d, time:%ld, t.tm_gmtoff=%ld", i, then, t.tm_gmtoff) ;
  }
#endif
#if NO_SPAM
  qlonglong A=1111, B=2222 ;
  QString str = "bebe " ;
  str += QString(" timestamp: %1/%2").arg(A).arg(B) ;
  log_debug("%s", str.toStdString().c_str()) ;
#endif
#if NO_SPAM
  log_info("AA") ;
  for(nanotime_t x(-2,0); x.sec()<3; x+=nanotime_t(0,100000000))
  {
    QString res ;
    QTextStream os(&res) ;
    os << x ;
    log_info("%s=%ld", string_q_to_std(res).c_str(), x.to_time_t()) ;
  }
  log_info("BB") ;
#endif
}

Timed::Timed(int ac, char **av) :
  QCoreApplication(ac, av),
  peer(NULL)
//  session_bus_name("timed_not_connected"),
//  session_bus_address("invalid_address")
{
  spam() ;
  halted = "" ; // XXX: remove it, as we don't want to halt anymore
  first_boot_date_adjusted = false;
  log_debug() ;

  init_scratchbox_mode() ;
  log_debug() ;

  init_unix_signal_handler() ;
  log_debug() ;

 // QMLOG_IF
 //   init_dbus_peer_info() ;
 //   log_debug() ;
 // QMLOG_ENDIF ;

  // init_act_dead() ;
  // init_dsme_mode() ;
  log_debug() ;

  init_configuration() ;
  log_debug() ;

  init_customization() ;
  log_debug() ;

  init_read_settings() ;
  log_debug() ;

  init_create_event_machine() ;
  log_debug() ;

  init_device_mode() ;
  log_debug() ;

  init_context_objects() ;
  log_debug() ;

  init_backup_object() ;
  log_debug() ;

  init_main_interface_object() ;
  log_debug() ;

  init_backup_dbus_name() ;
  log_debug() ;

  init_main_interface_dbus_name() ;
  log_debug() ;

  start_voland_watcher();
  log_debug() ;

  init_kernel_notification();

  init_first_boot_hwclock_time_adjustment_check();
  log_debug() ;

  init_load_events() ;
  log_debug() ;

#if OFONO
  init_cellular_services() ;
#endif // OFONO

  log_debug() ;

  init_network_events() ;
  log_debug() ;

  init_dst_checker() ;

  log_debug("starting event mahine") ;

  init_start_event_machine() ;
  log_debug() ;

  log_debug("applying time zone settings") ;

  init_apply_tz_settings() ;
  log_debug() ;

  log_info("daemon is up and running") ;
}

// * Start Unix signal handling
void Timed::init_unix_signal_handler()
{
  signal_object = UnixSignal::object() ;
  QObject::connect(signal_object, SIGNAL(signal(int)), this, SLOT(unix_signal(int))) ;
  signal_object->handle(SIGINT) ;
  signal_object->handle(SIGTERM) ;
  signal_object->handle(SIGCHLD) ;
}

// * Enable questioning of Dbus peers
void Timed::init_dbus_peer_info()
{
  peer =  new peer_t(true) ;
}

// * Condition "running inside of scratchbox" is detected
void Timed::init_scratchbox_mode()
{
#if F_SCRATCHBOX
#if 0
  const char *path = getenv("PATH") ;
  scratchbox_mode = path && strstr(path, "scratchbox") ; // XXX: more robust sb detection?
#else
  const char *magic_path = "/targets/links/scratchbox.config" ;
  scratchbox_mode = access(magic_path, F_OK)==0 ;
#endif
  log_info("%s" "SCRATCHBOX detected", scratchbox_mode ? "" : "no ") ;
#else
  scratchbox_mode = false ;
#endif
}

// * Condition "running in ACT DEAD mode" is detected.
// * When running on Harmattan device (not scratchbox!):
//      some sanity checks are performed.

#if F_ACTING_DEAD
// Ask DSME: are we in ACT_DEAD mode?
// Returns:
//          -1: nobody knows (for example: DSME not running)
//           0: USER mode
//           1: ACT_DEAD mode
#if 0
static int is_act_dead_by_dsme(string &dsme_mode)
{
  // QDBusInterface dsme(dsme_service, dsme_req_path, dsme_req_interface, QDBusConnection::systemBus()) ;
  DsmeReqInterface dsme ;

  if (not dsme.isValid())
  {
    log_error("DSME interface isn't valid") ;
    return -1 ;
  }

  QDBusReply<QString> res = dsme.get_state_sync() ;

  if (not res.isValid())
  {
    log_error("DSME returned invalid answer, last error: '%s'", QDBusConnection::systemBus().lastError().message().toStdString().c_str()) ;
    return -1 ;
  }

  dsme_mode = res.value().toStdString() ;
  log_notice("got a mode string from DSME: '%s'", dsme_mode.c_str()) ;

  if (dsme_mode=="USER")
    return 0 ;
  else if (dsme_mode=="ACTDEAD")
    return 1 ;
  else
    return -1 ;
}
#endif

// Detecting run mode by /tmp/USER, /tmp/ACT_DEAD, /tmp/STATUS
// Returns:
//          -1: nobody knows (files are not in consistent state)
//           0: USER mode
//           1: ACT_DEAD mode
static int is_act_dead_by_status_files()
{
  bool tmp_act_dead = access("/tmp/ACT_DEAD", F_OK) == 0 ;
  bool tmp_user = access("/tmp/USER", F_OK) == 0 ;
  string tmp_state ;
  iodata::storage::read_file_to_string("/tmp/STATE", tmp_state) ;
  bool mode_is_user = tmp_user and not tmp_act_dead and tmp_state == "USER\n" ;
  bool mode_is_act_dead = not tmp_user and tmp_act_dead and tmp_state == "ACT_DEAD\n" ;

  if (mode_is_user)
    return 0 ;
  else if (mode_is_act_dead)
    return 1 ;
  else
  {
    log_error("inconsistent device mode indication, more info follows") ;
    log_error("/tmp/USER %s exist", tmp_user ? "does" : "doesn't") ;
    log_error("/tmp/ACT_DEAD %s exist", tmp_act_dead ? "does" : "doesn't") ;
    log_error("content of /tmp/STATE: '%s'", tmp_state.c_str()) ;
    return -1 ;
  }
}

// Make two stage detection, return only if successfully detected
#if 0
static bool init_act_dead_v2(bool use_status_files)
{
  string s_dsme_mode ;
  int dsme_mode = is_act_dead_by_dsme(s_dsme_mode) ;
  if (0<=dsme_mode) // got a valid answer: user or act_dead
    return dsme_mode ;

  if (use_status_files)
  {
    int sb_mode = is_act_dead_by_status_files() ;
    if (0<=sb_mode)
      return sb_mode ;
  }

  // oops, we don't know which mode we're running in; let's die

  // how to die? it depends on dsme mode

  log_critical("can't decide in which mode to run (dsme mode: '%s')", s_dsme_mode.c_str()) ;

  if (s_dsme_mode=="SHUTDOWN" or s_dsme_mode=="REBOOT")
  {
    log_critical("go to sleep, waiting to be killed, bye...") ;
    while(true) sleep(239239239) ;
  }

  log_critical("will be terminated in two seconds, bye...") ;

  sleep(2) ;
  log_abort("aborting") ;
}
#endif

#endif

void Timed::init_device_mode()
{
  current_mode = "(unknown)" ;
#if HAVE_DSME
  dsme_mode_handler = new dsme_mode_t ;
  QObject::connect(dsme_mode_handler, SIGNAL(mode_is_changing()), this, SLOT(dsme_mode_is_changing())) ;
  QObject::connect(dsme_mode_handler, SIGNAL(mode_reported(const string &)), this, SLOT(dsme_mode_reported(const string &))) ;
#if F_ACTING_DEAD
  if (scratchbox_mode)
  {
    int is_act_dead = is_act_dead_by_status_files() ;
    bool user_mode = is_act_dead==0, mode_known = is_act_dead==0 or is_act_dead==1 ;
    if (not mode_known)
      log_abort("can't detect running mode") ;
    device_mode_reached(user_mode) ;
  }
  else
#endif
  {
    dsme_mode_handler->init_request() ;
  }
#endif
  const char *startup_path="/com/nokia/startup/signal", *startup_iface="com.nokia.startup.signal" ;
  const char *desktop_visible_slot = SLOT(harmattan_desktop_visible()) ;
  const char *init_done_slot = SLOT(harmattan_init_done(int)) ;
  bool res1 = QDBusConnection::systemBus().connect("", startup_path, startup_iface, "desktop_visible", this, desktop_visible_slot) ;
  bool res2 = QDBusConnection::systemBus().connect("", startup_path, startup_iface, "init_done", this, init_done_slot) ;
  if (not res1)
    log_critical("can't connect to 'desktop_visible' signal") ;
  if (not res2)
    log_critical("can't connect to 'init_done' signal") ;
}

#if 0
void Timed::init_act_dead()
{
#if F_ACTING_DEAD
  act_dead_mode = init_act_dead_v2(scratchbox_mode) ;
#else
  act_dead_mode = false ;
#endif
  log_notice("running in %s mode", act_dead_mode ? "ACT_DEAD" : "USER") ;
}
#endif

// * Reading configuration file
// * Warning if no exists (which is okey)
void Timed::init_configuration()
{
  iodata::storage *config_storage = new iodata::storage ;
  config_storage->set_primary_path(configuration_path()) ;
  config_storage->set_validator(etc_timed_rc_validator(), "config_t") ;

  iodata::record *c = config_storage->load() ;
  log_assert(c, "loading configuration settings failed") ;

  if(config_storage->source()==0)
    log_info("configuration loaded from '%s'", configuration_path()) ;
  else
    log_warning("configuration file '%s' corrupted or non-existing, using default values", configuration_path()) ;


  events_path = c->get("queue_path")->str() ; // TODO: make C++ variables match data fields
  settings_path = c->get("settings_path")->str() ;
  threshold_period_long = c->get("queue_threshold_long")->value() ;
  threshold_period_short = c->get("queue_threshold_short")->value() ;
  ping_period = c->get("voland_ping_sleep")->value() ;
  ping_max_num = c->get("voland_ping_retries")->value() ;
  log_debug("deleting iodata::record *c") ;
  delete c ;
  log_debug("deleting config_storage") ;
  delete config_storage ;
  log_debug("done") ;
}

static bool parse_boolean(const string &str)
{
  return str == "true" || str == "True" || str == "1" ;
}

// * read customization data provided by customization package
void Timed::init_customization()
{
  iodata::storage *storage = new iodata::storage ;
  storage->set_primary_path(customization_path()) ;
  storage->set_validator(customization_data_validator(), "customization_t") ;

  iodata::record *c = storage->load() ;
  log_assert(c, "loading customization settings failed") ;

  if(storage->source()==0)
    log_info("customization loaded from '%s'", customization_path()) ;
  else
    log_warning("customization file '%s' corrupted or non-existing, using default values", customization_path()) ;

  format24_by_default = parse_boolean(c->get("format24")->str()) ;
  nitz_supported = parse_boolean(c->get("useNitz")->str()) ;
  auto_time_by_default = parse_boolean(c->get("autoTime")->str()) ;
  guess_tz_by_default = parse_boolean(c->get("guessTz")->str()) ;
  tz_by_default = c->get("tz")->str() ;

  if (not nitz_supported and auto_time_by_default)
  {
    log_warning("automatic time update disabled because nitz is not supported in the device") ;
    auto_time_by_default = false ;
  }

  delete c ;
  delete storage ;
}

// * read settings
// * apply customization defaults, if needed
void Timed::init_read_settings()
{
  settings_storage = new iodata::storage ;
  settings_storage->set_primary_path(settings_path) ;
  settings_storage->set_secondary_path(settings_path+".bak") ;
  settings_storage->set_validator(settings_data_validator(), "settings_t") ;

  iodata::record *tree = settings_storage->load() ;

  log_assert(tree, "loading settings failed") ;

#define apply_cust(key, val)  do { if (tree->get(key)->value() < 0) tree->add(key, val) ; } while(false)
  apply_cust("format_24", format24_by_default) ;
  apply_cust("time_nitz", auto_time_by_default) ;
  apply_cust("local_cellular", guess_tz_by_default) ;
#undef apply_cust

  settings = new source_settings(this) ; // TODO: use tz_by_default here
  settings->load(tree, tz_by_default) ;

  delete tree ;
}

void Timed::init_create_event_machine()
{
  am = new machine_t(this) ;
  log_debug("am=new machine done") ;
  q_pause = NULL ;

  // The following call is commented out: device mode will be known later
#if 0
  am->device_mode_detected(not act_dead_mode) ; // TODO: avoid "not" here
#endif

  short_save_threshold_timer = new simple_timer(threshold_period_short) ;
  long_save_threshold_timer = new simple_timer(threshold_period_long) ;
  QObject::connect(short_save_threshold_timer, SIGNAL(timeout()), this, SLOT(queue_threshold_timeout())) ;
  QObject::connect(long_save_threshold_timer, SIGNAL(timeout()), this, SLOT(queue_threshold_timeout())) ;

  QObject::connect(am, SIGNAL(child_created(unsigned,int)), this, SLOT(register_child(unsigned,int))) ;
  clear_invokation_flag() ;

  ping = new pinguin_t(ping_period, ping_max_num, this) ;
  QObject::connect(am, SIGNAL(voland_needed()), ping, SLOT(voland_needed())) ;
  QObject::connect(this, SIGNAL(voland_registered()), ping, SLOT(voland_registered())) ;

  QObject::connect(am, SIGNAL(queue_to_be_saved()), this, SLOT(event_queue_changed())) ;

#if 0
  QDBusConnectionInterface *bus_ifc = Maemo::Timed::Voland::bus().interface() ;

  voland_watcher = new QDBusServiceWatcher((QString)Maemo::Timed::Voland::service(), Maemo::Timed::Voland::bus()) ;
  QObject::connect(voland_watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)), this, SLOT(system_owner_changed(QString,QString,QString))) ;
#else
  voland_watcher = NULL ;
#endif
  QObject::connect(this, SIGNAL(voland_registered()), am, SIGNAL(voland_registered())) ;
  QObject::connect(this, SIGNAL(voland_unregistered()), am, SIGNAL(voland_unregistered())) ;

#if 0
  bool voland_present = bus_ifc->isServiceRegistered(Maemo::Timed::Voland::service()) ;

  if(voland_present)
  {
    log_info("Voland service %s detected", Maemo::Timed::Voland::service()) ;
    emit voland_registered() ;
  }
#endif
}

void Timed::stop_voland_watcher()
{
  if (voland_watcher)
    delete voland_watcher ;
  voland_watcher = NULL ;
}

void Timed::start_voland_watcher()
{
  stop_voland_watcher() ;

  voland_watcher = new QDBusServiceWatcher((QString)Maemo::Timed::Voland::service(),
                                           QDBusConnection::systemBus());
  QObject::connect(voland_watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)), this, SLOT(system_owner_changed(QString,QString,QString))) ;

  QDBusConnectionInterface *bus_ifc = QDBusConnection::systemBus().interface();
  bool voland_present = bus_ifc and bus_ifc->isServiceRegistered(Maemo::Timed::Voland::service()) ;

  if(voland_present)
  {
    log_info("Voland service %s detected", Maemo::Timed::Voland::service()) ;
    emit voland_registered() ;
  }
}


void Timed::init_context_objects()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  // TODO: add Qt5 replacement for ContextProvider
#else
  context_service = new ContextProvider::Service(Maemo::Timed::bus()) ;
  context_service -> setAsDefault() ;

  log_debug("(new ContextProvider::Service(Maemo::Timed::bus()))->setAsDefault()") ;
  ContextProvider::Property("Alarm.Trigger") ;
  ContextProvider::Property("Alarm.Present") ;
  ContextProvider::Property("/com/nokia/time/time_zone/oracle") ;
  time_operational_p = new ContextProvider::Property("/com/nokia/time/system_time/operational") ;
  time_operational_p->setValue(am->is_epoch_open()) ;
  QObject::connect(am, SIGNAL(next_bootup_event(int,int)), this, SIGNAL(next_bootup_event(int,int)));
#endif
}

void Timed::init_backup_object()
{
  backup_object = new QObject ;
  new backup_t(this, backup_object) ;
  // XXX: what if we're using system bus: how should backup know this?
  // TODO: if using system bus, keep track of started/terminated sessions? (omg!)
  QDBusConnection conn = Maemo::Timed::bus() ;
  const char * const path = "/com/nokia/timed/backup" ;
  if (conn.registerObject(path, backup_object))
    log_info("backup interface object registered on path '%s'", path) ;
  else
  {
    log_critical("failed to register backup object on path '%s': %s", path, conn.lastError().message().toStdString().c_str()) ;
    log_critical("backup/restore not available") ;
  }
}

void Timed::init_main_interface_object()
{
  new com_nokia_time(this) ;
  QDBusConnection conn = Maemo::Timed::bus() ;
  const char * const path = Maemo::Timed::objpath() ;
  if (conn.registerObject(path, this))
    log_info("main interface object registered on path '%s'", path) ;
  else
    log_critical("remote methods not available; failed to register dbus object: %s", Maemo::Timed::bus().lastError().message().toStdString().c_str()) ;
  // XXX:
  // probably it's a good idea to terminate if failed
  // (usually it means, the dbus connection is not available)
  // but on the other hand we can still provide some services (like setting time and zone)
  // Anyway, we will terminate if the mutex like name is not available
}

void Timed::init_backup_dbus_name()
{
  // We're using an another name for backup interface
  //   to avoid mess while switching to system bus and back again (later)
  // XXX: But for now it's just the same connection as com.nokia.time
  QDBusConnection conn = Maemo::Timed::bus() ;
  const char * const name = "com.nokia.timed.backup" ;
  const string conn_name = conn.name().toStdString() ;
  if (conn.registerService(name))
    log_info("service name '%s' registered on bus '%s'", name, conn_name.c_str()) ;
  else
  {
    const string msg = conn.lastError().message().toStdString() ;
    log_critical("can't register service '%s' on bus '%s': '%s'", name, conn_name.c_str(), msg.c_str()) ;
    log_critical("backup/restore not available") ;
  }
}

void Timed::init_main_interface_dbus_name()
{
  // We're misusing the dbus name as a some kind of mutex:
  //   only one instance of timed is allowed to run.
  // This is the why we can't drop the name later.

  const string conn_name = Maemo::Timed::bus().name().toStdString() ;
  if (Maemo::Timed::bus().registerService(Maemo::Timed::service()))
    log_info("service name '%s' registered on bus '%s'", Maemo::Timed::service(), conn_name.c_str()) ;
  else
  {
    const string msg = Maemo::Timed::bus().lastError().message().toStdString() ;
    log_critical("can't register service '%s' on bus '%s': '%s'", Maemo::Timed::service(), conn_name.c_str(), msg.c_str()) ;
    log_critical("aborting") ;
    ::exit(1) ;
  }
}

void Timed::init_load_events()
{
  event_storage = new iodata::storage ;
  event_storage->set_primary_path(events_path) ;
  event_storage->set_secondary_path(events_path+".bak") ;
  event_storage->set_validator(events_data_validator(), "event_queue_t") ;

  iodata::record *events = event_storage->load() ;

  log_assert(events) ;

  am->load(events) ;

  delete events ;
}

void Timed::init_start_event_machine()
{
  if(not settings_storage->fix_files(false))
    log_critical("can't fix the primary settings file") ;
  if(not event_storage->fix_files(false))
    log_critical("can't fix the primary event queue file") ;
  am->process_transition_queue() ;
  am->start() ;
}

#if OFONO
void Timed::init_cellular_services()
{
#if 0
  nitz_object = cellular_handler::object() ;
  int nitzrez = QObject::connect(nitz_object, SIGNAL(cellular_data_received(const cellular_info_t &)), this, SLOT(nitz_notification(const cellular_info_t &))) ;
  log_debug("nitzrez=%d", nitzrez) ;
#endif
  tzdata::init(tz_by_default) ;
  csd = new csd_t(this) ;
  tz_oracle = new tz_oracle_t ;

  bool res1 = QObject::connect(csd, SIGNAL(csd_cellular_time(const cellular_time_t &)), settings, SLOT(cellular_time_slot(const cellular_time_t &)));
  bool res2 = QObject::connect(csd, SIGNAL(csd_cellular_offset(const cellular_offset_t &)), tz_oracle, SLOT(cellular_offset(const cellular_offset_t &)));
  bool res3 = QObject::connect(csd, SIGNAL(csd_cellular_operator(const cellular_operator_t &)), tz_oracle, SLOT(cellular_operator(const cellular_operator_t &)));
  bool res4 = QObject::connect(tz_oracle, SIGNAL(cellular_zone_detected(olson *, suggestion_t, bool)), settings, SLOT(cellular_zone_slot(olson *, suggestion_t, bool)));

  log_assert(res1) ;
  log_assert(res2) ;
  log_assert(res3) ;
  log_assert(res4) ;

#if 0
  QObject::connect(tz_oracle, SIGNAL(tz_detected(olson *, tz_suggestions_t)), this, SLOT(tz_by_oracle(olson *, tz_suggestions_t))) ;
  QObject::connect(nitz_object, SIGNAL(cellular_data_received(const cellular_info_t &)), tz_oracle, SLOT(nitz_data(const cellular_info_t &))) ;
#endif
}
#endif // OFONO

void Timed::init_network_events()
{
  network_configuration_manager = new QNetworkConfigurationManager ;
  connect(network_configuration_manager, SIGNAL(onlineStateChanged(bool)), am, SLOT(online_state_changed(bool))) ;
  bool connected_now = network_configuration_manager->isOnline() ;
  if (connected_now)
    am->online_state_changed(true) ;
}

void Timed::init_dst_checker()
{
  log_debug() ;
  dst_timer = new QTimer ;
  dst_timer->setSingleShot(true) ;
  QObject::connect(dst_timer, SIGNAL(timeout()), this, SLOT(check_dst())) ;
}

void Timed::init_apply_tz_settings()
{
  settings->postload_fix_manual_zone() ;
  settings->postload_fix_manual_offset() ;
  if(settings->check_target(settings->etc_localtime()) != 0)
    invoke_signal() ;
}

Timed::~Timed()
{
  stop_machine() ;
  stop_context() ;
  stop_dbus() ;
  stop_stuff() ;
}
void Timed::stop_machine()
{
  delete am ;
}
void Timed::stop_context()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
// TODO: add Qt5 replacement for ContextProvider
#else
  delete context_service ;
  delete time_operational_p ;
#endif
}
void Timed::stop_dbus()
{
  delete backup_object ;
  Maemo::Timed::bus().unregisterService(Maemo::Timed::service()) ;
  Maemo::Timed::bus().unregisterService("com.nokia.timed.backup") ;
  QDBusConnection::disconnectFromBus(QDBusConnection::systemBus().name()) ;
}
void Timed::stop_stuff()
{
  log_debug() ;
  // delete ping ;
  log_debug() ;
  delete settings ;
  log_debug() ;
  // delete ses_iface ;
  log_debug() ;
  delete voland_watcher ;
  log_debug() ;
  delete event_storage ;
  log_debug() ;
  delete settings_storage ;
  log_debug() ;
  delete short_save_threshold_timer ;
  log_debug() ;
  delete long_save_threshold_timer ;
  log_debug() ;
  delete tz_oracle ;
  log_debug() ;
  delete dst_timer ;
  log_debug() ;
  cellular_handler::uninitialize() ;
  log_notice("stop_stuff() DONE") ;
}


// Move the stuff below to machine:: class

cookie_t Timed::add_event(cookie_t remove, const Maemo::Timed::event_io_t &x, const QDBusMessage &message)
{
  if(remove.is_valid() && am->find_event(remove)==NULL)
  {
    log_error("[%d]: cookie not found, event can't be replaced", remove.value()) ;
    return cookie_t() ;
  }

  cookie_t c = am->add_event(&x, true, NULL, &message) ; // message is given, but no creds
  log_debug() ;
  QMap<QString,QString>::const_iterator test = x.attr.txt.find("TEST") ;
  log_debug() ;
  if(test!=x.attr.txt.end())
    log_debug("TEST event: '%s', cookie=%d", test.value().toStdString().c_str(), c.value()) ;
  log_debug() ;
  if(c.is_valid() && remove.is_valid() && !am->cancel_by_cookie(remove))
    log_critical("[%d]: failed to remove event", remove.value()) ;
  return c ;
}

void Timed::add_events(const Maemo::Timed::event_list_io_t &events, QList<QVariant> &res, const QDBusMessage &message)
{
  if(events.ee.size()==0)
  {
    log_info("empty event list to add, ignoring") ;
    return ;
  }
  QMap<QString,QString>::const_iterator test = events.ee[0].attr.txt.find("TEST") ;
  if(test!=events.ee[0].attr.txt.end())
    log_debug("TEST list of %d events: '%s'", events.ee.size(), test.value().toStdString().c_str()) ;
  am->add_events(events, res, message) ;
}

bool Timed::get_event(cookie_t c, Maemo::Timed::event_io_t &res)
{
  if (!c.is_valid())
  {
    log_error("[%d]: cookie is invalid", c.value()) ;
    return false;
  }

  event_t *event = am->find_event(c) ;
  if(event == NULL)
  {
    log_error("[%d]: cookie is not found", c.value()) ;
    return false;
  }

  event_t::to_dbus_iface(*event, res);
  return true ;
}

bool Timed::get_events(const QList<uint> &cookies, Maemo::Timed::event_list_io_t &res)
{
  if(cookies.size() == 0)
  {
    log_error("no any cookie in request argument") ;
    return false ;
  }

  res.ee.resize(cookies.count()) ;

  bool status = true ;
  for(int i = 0; i < cookies.count(); ++i)
  {
    log_debug("Searching for cookies[%d]", i) ;
    status = get_event(cookie_t(cookies[i]), res.ee[i]) ;
    if(!status)
      break ;
  }
  return status ;
}

bool Timed::dialog_response(cookie_t c, int value)
{
  log_debug("Responded: %d(value=%d)", c.value(), value) ;
  return am->dialog_response(c, value) ;
}

void Timed::system_owner_changed(const QString &name, const QString &oldowner, const QString &newowner)
{
  log_debug() ;
  bool name_match = name==Maemo::Timed::Voland::service() ;
  if(name_match && oldowner.isEmpty() && !newowner.isEmpty())
    emit voland_registered() ;
  else if(name_match && !oldowner.isEmpty() && newowner.isEmpty())
    emit voland_unregistered() ;
#define __qstr(a) (a.isEmpty()?"<empty>":a.toStdString().c_str())
  if(name_match)
    log_info("Service %s owner changed from %s to %s", __qstr(name), __qstr(oldowner), __qstr(newowner)) ;
  else
    log_error("expecing notification about '%s' got about '%s'", Maemo::Timed::Voland::service(), name.toStdString().c_str()) ;
#undef __qstr
}

void Timed::event_queue_changed()
{
  bool running = short_save_threshold_timer->isActive() ;
  if(running)
    short_save_threshold_timer->stop() ;
  else
    long_save_threshold_timer->start() ;
  short_save_threshold_timer->start() ;
}

void Timed::queue_threshold_timeout()
{
  short_save_threshold_timer->stop() ;
  long_save_threshold_timer->stop() ;
  int method_index = this->metaObject()->indexOfMethod("save_event_queue()") ;
  QMetaMethod method = this->metaObject()->method(method_index) ;
  method.invoke(this, Qt::QueuedConnection) ;
}

/*
 * xxx
 * These are the "stupid and simple" backup methods.
 * Just like the doctor ordered. :)
 * The chmod is a workaround for backup-framework crash bug.
 */

void Timed::save_event_queue()
{
#if 0
  log_warning("skipping writing queue file") ;
  return ;
#endif
  iodata::record *queue = am->save(false) ; // false = full queue, not backup
  int res = event_storage->save(queue) ;

  if(res==0) // primary file
    log_info("event queue written") ;
  else if(res==1)
    log_warning("event queue written to secondary file") ;
  else
    log_critical("event queue can't be saved") ;

  delete queue ;
}

void Timed::save_settings()
{
  iodata::record *tree = settings->save() ;
  int res = settings_storage->save(tree) ;

  if(res==0) // primary file
    log_info("wall clock settings written") ;
  else if(res==1)
    log_warning("wall clock settings written to secondary file") ;
  else
    log_critical("wall clock settings can't be saved") ;

  delete tree ;
}

void Timed::invoke_signal(const nanotime_t &back)
{
  log_debug("systime_back=%s, back=%s", systime_back.str().c_str(), back.str().c_str()) ;
  systime_back += back ;
  log_debug("new value: systime_back=%s", systime_back.str().c_str()) ;
  if(signal_invoked)
    return ;
  signal_invoked = true ;
  int methodIndex = this->metaObject()->indexOfMethod("send_time_settings()") ;
  QMetaMethod method = this->metaObject()->method(methodIndex);
  method.invoke(this, Qt::QueuedConnection);
  log_assert(q_pause==NULL) ;
  q_pause = new machine_t::pause_t(am) ;
  log_debug("new q_pause=%p", q_pause) ;
}

void Timed::send_time_settings()
{
  log_debug() ;
  log_debug("settings=%p", settings) ;
  log_debug("settings->cellular_zone=%p", settings->cellular_zone) ;
  log_debug("settings->cellular_zone='%s'", settings->cellular_zone->zone().c_str()) ;
  nanotime_t diff = systime_back ;
  clear_invokation_flag() ;
  save_settings() ;
  settings->fix_etc_localtime() ;
  sent_signature = dst_signature(time(NULL)) ;
  Maemo::Timed::WallClock::Info info(settings->get_wall_clock_info(diff)) ;
  log_notice("sending signal 'settings_changed': %s", info.str().toStdString().c_str()) ;
  emit settings_changed(info, not diff.is_zero()) ;
  log_notice("signal 'settings_changed' sent") ;
  // emit settings_changed_1(systime) ;
  am->reshuffle_queue(diff) ;
  if(q_pause)
  {
    delete q_pause ;
    q_pause = NULL ;
  }
  check_dst() ; // reschedule dst timer
}

void Timed::check_dst()
{
  dst_timer->stop() ;
  time_t t = time(NULL) ;
  string signature = dst_signature(t) ;
  if (signature != sent_signature)
  {
    invoke_signal() ;
    return ;
  }

  int look_forward = 3600 ; // 1 hour
  string signature_2 = dst_signature(t+look_forward) ;
  if (signature_2 == signature)
  {
    dst_timer->start(1000*(look_forward-60)) ; // 1 minute less
    return ;
  }

  int a=0, b=look_forward ;
  while (b-a > 1)
  {
    int c = (a+b) / 2 ;
    (signature==dst_signature(t+c) ? a : b) = c ;
  }
  // now 'a' is the time until the last 'old time' second
  //     'b=a+1' until the first 'new time' second
  dst_timer->start(1000*b) ;
}

void Timed::unix_signal(int signo)
{
  switch(signo)
  {
    default:
      log_info("unix signal %d [%s] detected", signo, strsignal(signo)) ;
      break ;
    case SIGCHLD:
      int status ;
      while(pid_t pid = waitpid(-1, &status, WNOHANG))
      {
        if(pid==-1 && errno==EINTR)
        {
          log_info("waitpid() interrupted, retrying...") ;
          continue ;
        }
        else if(pid==-1)
        {
          if(errno!=ECHILD)
            log_error("waitpid() failed: %m") ;
          break ;
        }
        unsigned cookie = children.count(pid) ? children[pid] : 0 ;
        string name = str_printf("the child pid=%d", pid) ;
        if(cookie)
          name += str_printf(" [cookie=%d]", cookie) ;
        else
          name += " (unknown cookie)" ;
        if(WIFEXITED(status))
          log_info("%s exited with status %d", name.c_str(), WEXITSTATUS(status)) ;
        else if(WIFSIGNALED(status))
          log_info("%s killed by signal %d", name.c_str(), WTERMSIG(status)) ;
        else
        {
          log_info("%s changed status", name.c_str()) ;
          continue ;
        }
        children.erase(pid) ;
      }
      break ;
    case SIGINT:
      log_info("Keyboard interrupt, oh weh... bye") ;
      quit() ;
      break ;
    case SIGTERM:
      log_info("Termination signal... bye") ;
      quit() ;
      break ;
  }
}

#if 0
void Timed::nitz_notification(const cellular_info_t &ci)
{
  log_debug() ;
  log_info("nitz_notification: %s", ci.to_string().c_str()) ;
  settings->cellular_information(ci) ;
  log_debug() ;
}
#endif

#if 0
void Timed::tz_by_oracle(olson *tz, tz_suggestions_t s)
{
  log_debug("time zone '%s' magicaly detected", tz->name().c_str()) ;
  settings->cellular_zone->value = tz->name() ;
  settings->cellular_zone->suggestions = s ;
  if(settings->local_cellular)
  {
    settings->fix_etc_localtime() ;
    update_oracle_context(true) ;
  }
  invoke_signal() ;
}
#endif

void Timed::update_oracle_context(bool s)
{
  log_warning("update_oracle_context(%d): NOT IMPLEMENTED", s) ;
#if 0
  static ContextProvider::Property oracle_p("/com/nokia/time/time_zone/oracle") ;
  static const char * const uncertain_key = "uncertain" ;
  static const char * const primary_key = "primary_candidates" ;
  static const char * const possible_key = "possible_candidates" ;

  if(!set)
  {
    oracle_p.unsetValue() ;
    return ;
  }

  QMap<QString, QVariant> map ;

  tz_suggestions_t &s = settings->cellular_zone->suggestions ;

  bool uncertain = s.gq == Uncertain ;
  if(uncertain)
  {
    map.insert(uncertain_key, true) ;
    QList<QVariant> primary_list ;
    for(vector<olson*>::iterator it=s.zones.begin(); it!=s.zones.end(); ++it)
      primary_list << string_std_to_q((*it)->name()) ;
    map.insert(primary_key, primary_list) ;
    (void) possible_key ; // not used variable
  }
  else
  {
    map.insert(uncertain_key, false) ;
  }

  oracle_p.setValue(map) ;
#endif
}

void Timed::open_epoch()
{
  am->open_epoch() ;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  // TODO: add Qt5 replacement for ContextProvider
#else
  time_operational_p->setValue(true) ;
#endif
}

#if HAVE_DSME
void Timed::dsme_mode_is_changing()
{
  log_notice("mode is changing, freezeng machine") ;
  // stop_voland_watcher() ;
  am->freeze() ;
}

void Timed::dsme_mode_reported(const string &mode)
{
  log_notice("MODE: reported by dsme '%s'", mode.c_str()) ;
  if (mode=="USER")
  {
    am->device_mode_detected(true) ;
    am->unfreeze() ;
  }
  else if (mode=="ACTDEAD")
  {
    am->device_mode_detected(false) ;
    am->unfreeze() ;
  }
  else
  {
    log_warning("MODE: machine remain frozen (mode reported by dsme: '%s')", mode.c_str()) ;
    return ;
  }
#if 0
  if (const char *addr = getenv("DBUS_SESSION_BUS_ADDRESS"))
    connect_to_session_bus(session_bus_address = addr) ;
  start_voland_watcher() ;
#endif
}
#endif

#if 0
void Timed::device_mode_reached(bool act_dead, const string &new_address)
{
  act_dead_mode = act_dead ;
  log_debug("act_dead=%d, new_address='%s'", act_dead, new_address.c_str()) ;
#if 0
  bool res = setenv("DBUS_SESSION_BUS_ADDRESS", dbus_session.c_str(), true) ;
  if (res<0)
  {
    log_error("can't set DBUS_SESSION_BUS_ADDRESS environment: %m") ;
    QDBusConnection::disconnectFromBus(session_bus_name.c_str()) ;
    return ;
  }
#endif
  connect_to_session_bus(session_bus_address = new_address) ;
  start_voland_watcher() ;
  am->device_mode_detected(not act_dead) ;
  am->unfreeze() ;
}
#endif
void Timed::device_mode_reached(bool user_mode)
{
  log_notice("MODE: running in %s mode", user_mode ? "USER" : "ACTDEAD") ;
  am->device_mode_detected(user_mode) ;
  am->unfreeze() ;
}

void Timed::session_reported(const QString &new_address)
{
#if 0
  session_bus_address = new_address.toStdString() ;
  log_notice("session bus address changed: '%s'", session_bus_address.c_str()) ;
  if (session_bus_address.empty())
  {
    stop_voland_watcher() ;
    QDBusConnection::disconnectFromBus(session_bus_name.c_str()) ;
  }
  else
  {
    start_voland_watcher() ;
  }
#else
  (void)new_address ;
#endif
}

void Timed::harmattan_desktop_visible()
{
  device_mode_reached(true) ; // USER mode
}

void Timed::harmattan_init_done(int runlevel)
{
  if (runlevel==5)
    device_mode_reached(false) ; // ACT_DEAD mode
}

static char *chomp(char *s)
{
  if (s)
  {
    char *p = s + strlen(s) - 1 ;
    while (*p=='\n' and s<=p)
      *p-- = '\0' ;
  }
  return s ;
}

void Timed::init_kernel_notification()
{
  notificator = new kernel_notification_t ;
  QObject::connect(notificator, SIGNAL(system_time_changed(const nanotime_t &)), this, SLOT(kernel_notification(const nanotime_t &))) ;
  notificator->start() ;
}

void Timed::kernel_notification(const nanotime_t &jump_forwards)
{
  log_notice("KERNEL: system time changed by %s", jump_forwards.str().c_str()) ;
  settings->process_kernel_notification(jump_forwards) ;
}

#define CONST_FIRST_BOOT_DATE_FILE        "/var/cache/timed/first-boot-hwclock.dat"

void Timed::init_first_boot_hwclock_time_adjustment_check() {
    if (first_boot_date_adjusted)
        return;

    QFile file(CONST_FIRST_BOOT_DATE_FILE);
    if (file.exists()) {
        first_boot_date_adjusted = true;
        return;
    }

    if (QDate::currentDate().year() < 2013) {
        log_info("first boot, updating old date from year %d to 01/01/2013", QDate::currentDate().year());
        settings->set_system_time(1357041600); // January 1, 12:00:00, 2013
    }

    first_boot_date_adjusted = true;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        log_error("Failed to open file %s", CONST_FIRST_BOOT_DATE_FILE);
        return;
    }
    if (!file.isWritable()) {
        log_error("File not writable: %s", CONST_FIRST_BOOT_DATE_FILE);
        return;
    }

    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString() << "\n";
    file.close();
}
