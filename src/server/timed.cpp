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
#define _BSD_SOURCE

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <ContextProvider>

#include <timed/interface>
#include <qm/log>

#include "adaptor.h"
#include "timed.h"
#include "settings.h"
#include "tz.h"

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
  qDebug() << str ;
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

Timed::Timed(int ac, char **av) : QCoreApplication(ac, av)
{
  spam() ;
  halted = "" ;
  //check acting dead / user mode
  bool act_dead_mode = access("/tmp/ACT_DEAD", F_OK) == 0 ;
  bool user_mode = access("/tmp/USER", F_OK) == 0 ;

  if(act_dead_mode == user_mode)
  {
    // some people are running it in scrachbox :(
    const char *path = getenv("PATH") ;
    bool scratchbox = path && strstr(path, "scratchbox") ;

    bool p = user_mode ;
    log_critical("%s ACT_DEAD and USER %s present in /tmp, exitting...", p?"both":"none of", p?"are":"is") ;

    if(scratchbox)
      log_info("Do not exit: it seems we are in scratchbox") ;
    else
      ::exit(1) ;
  }


  load_rc() ;

  short_save_threshold_timer = new simple_timer(threshold_period_short) ;
  long_save_threshold_timer = new simple_timer(threshold_period_long) ;
  QObject::connect(short_save_threshold_timer, SIGNAL(timeout()), this, SLOT(queue_threshold_timeout())) ;
  QObject::connect(long_save_threshold_timer, SIGNAL(timeout()), this, SLOT(queue_threshold_timeout())) ;

  am = new machine ;
  QObject::connect(am, SIGNAL(child_created(unsigned,int)), this, SLOT(register_child(unsigned,int))) ;
  q_pause = NULL ;
  clear_invokation_flag() ;
  load_settings() ;

  ping = new pinguin(ping_period, ping_max_num) ;

  QObject::connect(am, SIGNAL(voland_needed()), ping, SLOT(voland_needed())) ;
  QObject::connect(this, SIGNAL(voland_registered()), ping, SLOT(voland_registered())) ;

  QObject::connect(am, SIGNAL(queue_to_be_saved()), this, SLOT(event_queue_changed())) ;

  // starting context stuff

  (new ContextProvider::Service(Maemo::Timed::bus()))->setAsDefault() ;

  ContextProvider::Property("Alarm.Trigger") ;
  ContextProvider::Property("Alarm.Present") ;
  ContextProvider::Property("/com/nokia/time/time_zone/oracle") ;
  time_operational_p = new ContextProvider::Property("/com/nokia/time/system_time/operational") ;
  time_operational_p->setValue(am->is_epoch_open()) ;
  QObject::connect(am, SIGNAL(next_bootup_event(int)), this, SLOT(send_next_bootup_event(int))) ;

  new com_nokia_time(this) ;
  /* XXX
   * The stupid and simple backup dbus adaptor
   */
  new com_nokia_backupclient(this) ;
  if(! Maemo::Timed::bus().registerService(Maemo::Timed::service()))
  {
    string msg = Maemo::Timed::bus().lastError().message().toStdString() ;
    log_critical("aborting, because can't register service, message: '%s'", msg.c_str()) ;
    // may be to throw an exception ?
    ::exit(1) ;
  }

  log_info("service %s registered", Maemo::Timed::service()) ;

  // load the queue from file ....
  load_events() ;

  am->process_transition_queue() ;

  am->device_mode_detected(user_mode) ;

  bool res_obj = Maemo::Timed::bus().registerObject(Maemo::Timed::objpath(), this) ;
  if(!res_obj)
    log_critical("can't register D-Bus object: %s", Maemo::Timed::bus().lastError().message().toStdString().c_str()) ;

  // ses_iface = Maemo::Timed::Voland::bus().interface() ;
  voland_watcher = new QDBusServiceWatcher((QString)Maemo::Timed::Voland::service(), Maemo::Timed::Voland::bus()) ;
  // QObject::connect(ses_iface, SIGNAL(serviceOwnerChanged(QString,QString,QString)), this, SLOT(system_owner_changed(QString,QString,QString))) ;
  QObject::connect(voland_watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)), this, SLOT(system_owner_changed(QString,QString,QString))) ;
  QObject::connect(this, SIGNAL(voland_registered()), am, SIGNAL(voland_registered())) ;
  QObject::connect(this, SIGNAL(voland_unregistered()), am, SIGNAL(voland_unregistered())) ;

  check_voland_service() ;

  save_time_to_file_timer = new QTimer ;
  QObject::connect(save_time_to_file_timer, SIGNAL(timeout()), this, SLOT(save_time_to_file())) ;
  save_time_to_file() ;

  cellular_handler *nitz_object = cellular_handler::object() ;
  int nitzrez = QObject::connect(nitz_object, SIGNAL(cellular_data_received(const cellular_info_t &)), this, SLOT(nitz_notification(const cellular_info_t &))) ;
  log_debug("nitzrez=%d", nitzrez) ;
  // nitz_object->invoke_signal() ;

  signal_object = UnixSignal::object() ;
  QObject::connect(signal_object, SIGNAL(signal(int)), this, SLOT(unix_signal(int))) ;
  signal_object->handle(SIGINT) ;
  signal_object->handle(SIGCHLD) ;

  tz_oracle = new tz_oracle_t ;
  QObject::connect(tz_oracle, SIGNAL(tz_detected(olson *, tz_suggestions_t)), this, SLOT(tz_by_oracle(olson *, tz_suggestions_t))) ;
  QObject::connect(nitz_object, SIGNAL(cellular_data_received(const cellular_info_t &)), tz_oracle, SLOT(nitz_data(const cellular_info_t &))) ;
}

void Timed::check_voland_service()
{
  QDBusConnectionInterface *bus_ifc = Maemo::Timed::Voland::bus().interface() ;
  bool present = bus_ifc->isServiceRegistered(Maemo::Timed::Voland::service()) ;

  if(present)
  {
    log_info("Voland service %s detected", Maemo::Timed::Voland::service()) ;
    emit voland_registered() ;
  }
}

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
  if(c.is_valid() && remove.is_valid() && !am->cancel(remove))
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

bool Timed::dialog_response(cookie_t c, int value)
{
  log_debug("Responded: %d(value=%d)", c.value(), value) ;
  return am->dialog_response(c, value) ;
}

void Timed::system_owner_changed(const QString &name, const QString &oldowner, const QString &newowner)
{
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

void Timed::send_next_bootup_event(int value)
{
  QDBusConnection dsme = QDBusConnection::systemBus() ;
  QString path = Maemo::Timed::objpath() ;
  QString iface = Maemo::Timed::interface() ;
  QString signal = "next_bootup_event" ;
  QDBusMessage m = QDBusMessage::createSignal(path, iface, signal) ;
  m << value ;
  if(dsme.send(m))
    log_info("signal %s(%d) sent", string_q_to_std(signal).c_str(), value) ;
  else
    log_error("Failed to send the signal %s(%d) on system bus: %s", string_q_to_std(signal).c_str(), value, dsme.lastError().message().toStdString().c_str()) ;
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

void Timed::load_rc()
{
  timed_rc_storage = new iodata::storage ;
  timed_rc_storage->set_primary_path("/etc/timed.rc") ;
  timed_rc_storage->set_validator("/usr/share/timed/typeinfo/timed-rc.type", "timed_rc_t") ;

  iodata::record *rc = timed_rc_storage->load() ;
  log_assert(rc, "loading rc-setings failed") ;

  if(timed_rc_storage->source()==0)
    log_info("loaded rc-setting from file") ;
  else
    log_warning("rc-file not present or corrupted, using default settings") ;

  events_path = rc->get("queue_path")->str() ;
  settings_path = rc->get("settings_path")->str() ;
  threshold_period_long = rc->get("queue_threshold_long")->value() ;
  threshold_period_short = rc->get("queue_threshold_short")->value() ;
  ping_period = rc->get("voland_ping_sleep")->value() ;
  ping_max_num = rc->get("voland_ping_retries")->value() ;
  save_time_path = rc->get("saved_utc_time_path")->str() ;

  delete rc ;
}

/*
 * xxx
 * These are the "stupid and simple" backup methods.
 * Just like the doctor ordered. :)
 * The chmod is a workaround for backup-framework crash bug.
 */
void Timed::backup()
{
 /* Excerpts from Backup User Guide V0.2 ...
  *
  * 2.1 Backup Start
  *
  * This method will be called just before the actual backup
  * starts. The application can dump all the data in RAM to their
  * persistent storage. If the application wish to exit on getting
  * this call, it can do that after responding DBus calls. (The
  * application which uses temporary backup files can generate the
  * the file on getting this call.)
  */

  system("mkdir /tmp/.timed-backup; cp /var/cache/timed/*.data /etc/timed.rc /etc/timed-cust.rc /tmp/.timed-backup; chmod -R 0777 /tmp/.timed-backup");
}

void Timed::backup_finished()
{
  /* 2.2 Backup Finished
   *
   * The applications are notified that backup is finished. (The
   * applications which uses the temporary backup files can delete
   * the file on getting this call.)
   */

  system("rm -rf /tmp/.timed-backup");
}

void Timed::restore()
{
  /* 2.3 Restore Start
   *
   * The applications are notified that restore operation is going
   * to start. If the application wish to exit on getting this call,
   * it can do that.
   */
}

void Timed::restore_finished()
{
  /* 2.4 Restore Finished
   *
   * The applications are notified that one restore operation is
   * finished. On getting this method call the application should
   * reload all data from their persistent storage and update the
   * UI. (The applications which uses the temporary backup files can
   * import data from temporary files and delete them after it.)
   */

  system("cp -f /tmp/.timed-backup/*.data /var/cache/timed; cp -f /tmp/.timed-backup/*.rc /etc");
  backup_finished();
  QCoreApplication::exit(1);
}

void Timed::load_events()
{
  event_storage = new iodata::storage ;
  event_storage->set_primary_path(events_path) ;
  event_storage->set_secondary_path(events_path+".bak") ;
  event_storage->set_validator("/usr/share/timed/typeinfo/queue.type", "event_queue_t") ;

  iodata::record *events = event_storage->load() ;
  if(! event_storage->fix_files(false))
    log_critical("can't fix the primary event queue file") ;

  log_assert(events) ;

  am->load(events) ;

  delete events ;
}

void Timed::save_event_queue()
{
  iodata::record *queue = am->save() ;
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


void Timed::load_settings()
{
  cust_settings = new customization_settings();
  cust_settings->load();

  settings_storage = new iodata::storage ;
  settings_storage->set_primary_path(settings_path) ;
  settings_storage->set_secondary_path(settings_path+".bak") ;
  settings_storage->set_validator("/usr/share/timed/typeinfo/settings.type", "settings_t") ;

  iodata::record *tree = settings_storage->load() ;
  // If we dont have user settings stored, use customization ones.
  if (settings_storage->source() == -1)
  {
    log_debug("CUST settings_storage->source() == -1");
    tree->add("time_nitz", cust_settings->time_nitz);
    tree->add("local_cellular", cust_settings->time_nitz);
    tree->add("format_24", cust_settings->format_24);
  }
  if (!cust_settings->net_time_enabled)
  {
      tree->add("time_nitz", 0);
      tree->add("local_cellular", 0);
  }
  if(! settings_storage->fix_files(false))
    log_critical("can't fix the primary settings file") ;

  log_assert(tree) ;

  settings = new source_settings(this) ;
  settings->load(tree) ;

  delete tree ;
}

void Timed::invoke_signal(const nanotime_t &back)
{
  systime_back += back ;
  if(signal_invoked)
    return ;
  signal_invoked = true ;
  int methodIndex = this->metaObject()->indexOfMethod("send_time_settings()") ;
  QMetaMethod method = this->metaObject()->method(methodIndex);
  method.invoke(this, Qt::QueuedConnection);
  log_assert(q_pause==NULL) ;
  q_pause = new queue_pause(am) ;
  log_debug("new q_pause=%p", q_pause) ;
}

void Timed::send_time_settings()
{
  log_debug() ;
  log_info("settings->cellular_zone='%s'", settings->cellular_zone->zone().c_str()) ;
  nanotime_t diff = systime_back ;
  clear_invokation_flag() ;
  save_settings() ;
  settings->fix_etc_localtime() ;
  emit settings_changed(settings->get_wall_clock_info(diff), !diff.is_zero()) ;
  // emit settings_changed_1(systime) ;
  am->reshuffle_queue(diff) ;
  if(q_pause)
  {
    delete q_pause ;
    q_pause = NULL ;
  }
}

void Timed::save_time_to_file()
{
  save_time_to_file_timer->stop() ;

  if(FILE *fp = fopen(save_time_path.c_str(), "w"))
  {
    const int time_length = 32 ;
    char value[time_length+1] ;

    time_t tick = time(NULL) ;
    struct tm tm ;
    gmtime_r(&tick, &tm) ;
    strftime(value, time_length, "%F %T", &tm) ;

    fprintf(fp, "%s\n", value) ;
    if(fclose(fp)==0)
      log_info("current time (%s) saved in %s", value, save_time_path.c_str()) ;
    else
      log_error("can't write to file %s: %m", save_time_path.c_str()) ;
  }
  else
    log_error("can't open file %s: %m", save_time_path.c_str()) ;

  save_time_to_file_timer->start(1000*3600) ; // 1 hour
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
  }
}

void Timed::nitz_notification(const cellular_info_t &ci)
{
  log_debug() ;
  log_info("nitz_notification: %s", ci.to_string().c_str()) ;
  settings->cellular_information(ci) ;
  log_debug() ;
}

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

void Timed::update_oracle_context(bool set)
{
  static ContextProvider::Property oracle_p("/com/nokia/time/timezone/oracle") ;
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
}

void Timed::open_epoch()
{
  am->open_epoch() ;
  time_operational_p->setValue(true) ;
}
