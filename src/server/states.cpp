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
#include <QDBusInterface>
#include <QDebug>

#include <timed-voland/reminder>
#include <timed-voland/interface>
#include "timed-voland/reminder-pimple.h"
#include <timed/nanotime.h>

#include "event.h"

#include "states.h"
#include "timeutil.h"

#include "flags.h"
#include "misc.h"
#include "timed.h"

void state_start::enter(event_t *e)
{
  state::enter(e) ;
  om->request_state(e, "EPOCH") ;
}

const char *state_epoch::lost = "/var/cache/timed/SYSTEM_TIME_LOST" ;

state_epoch::state_epoch(machine *am) :
  gate_state("EPOCH", "NEW", am)
{
  if(access(lost, F_OK) != 0)
    is_open = true ;
}

void state_epoch::open()
{
  if(!is_open && unlink(lost)<0)
    log_critical("can't unlink '%s': %m", lost) ;
  gate_state::open() ;
}


void state_new::enter(event_t *e)
{
  state::enter(e) ;
  const char *next_state = "ABORTED" ;  // TODO: make a new state "FAILED" for this case

  if(e->flags & EventFlags::In_Dialog)
  {
    ticker_t t_now = om->transition_start_time ;

    bool restore = true ;
    if(e->last_triggered.is_valid() && t_now - e->last_triggered > om->dialog_discard_threshold)
      restore = false ;

    if(restore)
    {
      e->last_triggered = t_now ;
      next_state = "DLG_WAIT" ;
    }
    else
    {
      e->flags &= ~ EventFlags::In_Dialog ;
      next_state = "SERVED" ;
    }
  }
  else if(e->has_ticker() || e->has_time() || e->has_recurrence())
    next_state = "SCHEDULER" ;
  else if(e->to_be_keeped())
    next_state = "TRANQUIL" ;

  om->request_state(e, next_state) ;
  om->process_transition_queue() ;
}

void state_scheduler::enter(event_t *e)
{
  state::enter(e) ;
  log_debug() ;
  const char *next_state = "QENTRY" ;
  if(e->has_ticker())
    e->trigger = ticker_t(e->ticker) ;
  else if(e->has_time())
  {
    struct tm T;
    T.tm_sec = 0 ;
    T.tm_min = e->t.minute ;
    T.tm_hour = e->t.hour ;
    T.tm_mday = e->t.day ;
    T.tm_mon = e->t.month - 1 ;
    T.tm_year = e->t.year - 1900 ;
    T.tm_isdst = -1 ;
    log_debug("%d-%d-%d %d:%d", e->t.year, e->t.month, e->t.day, e->t.hour, e->t.minute) ;
    e->trigger = e->has_timezone() ? mktime_oversea(&T, e->tz): mktime_local(&T) ;
    log_debug("now=%ld e->trigger=%ld diff=%ld", time(NULL), e->trigger.value(), e->trigger.value()-time(NULL)) ;
    log_debug("e->has_timezone()=%d", e->has_timezone()) ;
    if(!e->trigger.is_valid())
    {
      log_debug() ;
      log_error("Failed to calculate trigger time for %s", e->t.str().c_str()) ;
      next_state = "ABORTED" ; // TODO: make a new state "FAILED" for this case
      log_debug() ;
    }
    log_debug() ;
  }
  else if(e->has_recurrence())
    next_state = "RECURRED" ;
  log_debug() ;
  om->request_state(e, next_state) ;
  log_debug() ;
}

void state_qentry::enter(event_t *e)
{
  state::enter(e) ;
  log_assert(e->trigger.is_valid()) ;
  om->request_state(e, "FLT_CONN") ;
}

state_queued::state_queued(machine *am) : io_state("QUEUED",am)
{
  pause_x = 0 ;
  alarm_timer = new QTimer ;
  alarm_timer->setSingleShot(true) ;
  connect(alarm_timer, SIGNAL(timeout()), this, SLOT(alarm_timeout())) ;
}

void state_queued::engine_pause(int dx)
{
  log_debug("dx=%d, current pause value: '%d', new value will be %d", dx, pause_x, pause_x+dx) ;
  if(pause_x==0)
  {
    log_assert(dx>0) ;
    timer_stop() ;
  }
  pause_x += dx ;
  if(pause_x==0)
  {
    log_assert(dx<0) ;
    // log_assert(not "debugging assert -> abort()") ;
    timer_start() ;
  }
}

void state_queued::alarm_timeout()
{
  queue_pause x(om) ;
  log_info("waked up by alarm_timer") ;
  ticker_t time_now = now() ;
  log_debug("time_now=%ld queue.begin=%ld", time_now.value(), queue.begin()!=queue.end() ? queue.begin()->first.value() : (time_t)-1) ;
  typedef set<event_pair>::iterator iterator ;
  bool event_found = false ;
  for(iterator it=queue.begin(); it!=queue.end() && it->first <= time_now; ++it, event_found=true)
    om->request_state(it->second, "DUE") ;
  if(event_found)
    om->process_transition_queue() ;
  else
    log_info("No due event found, need to sleep more") ;
}

void state_queued::enter(event_t *e)
{
  io_state::enter(e) ;
  queue_pause x(om) ;
  event_pair p = make_pair(e->trigger, e) ;
  queue.insert(p) ;
  if(e->flags & EventFlags::Boot)
    bootup.insert(p) ;
}

void state_queued::leave(event_t *e)
{
  queue_pause x(om) ;
  event_pair p = make_pair(e->trigger, e) ;
  queue.erase(p) ;
  bootup.erase(p) ;
  io_state::leave(e) ;
}

void state_queued::timer_start()
{
  if(queue.empty())
  {
    log_info("go to sleep, no alarm queued") ;
    om->send_queue_context() ;
    emit sleep() ;
    return ;
  }

  nanotime_t time_to_wait = nanotime_t(queue.begin()->first.value(), 0) - nanotime_t::systime_now() ;
  bool no_sleep = false ;
  if(time_to_wait<0)
  {
    log_info("no sleep today: an alarm is %s seconds overdue", (-time_to_wait).str().c_str()) ;
    no_sleep = true ;
  }
  else
    log_info("go to sleep, next alarm in %s seconds", time_to_wait.str().c_str()) ;

  if(no_sleep)
    alarm_timer->start(0) ;
  else
  {
    static const int threshold = 3600 ; // One hour
    int milisec ;
    if(time_to_wait<threshold)
      milisec = time_to_wait.sec() * 1000 + time_to_wait.nano() / 1000000 ;
    else
      milisec = (threshold-1) * 1000 ;
    log_debug("starting alarm_timer for %d milisec", milisec) ;
    alarm_timer->start(milisec) ;
    om->send_queue_context() ;
    emit sleep() ;
  }
}

void state_queued::timer_stop()
{
  alarm_timer->stop() ;
}

ticker_t state_queued::next_bootup()
{
  ticker_t tick ; // default value: invalid
  set<event_pair>::const_iterator it = bootup.begin() ;
  if(it!=bootup.end())
    tick = it->first ;
  return tick ;
}

ticker_t state_queued::next_rtc_bootup()
{
  ticker_t current_time = om->transition_start_time ;
  if(!current_time.is_valid()) // not in transition
    current_time = now() ;

  ticker_t threshold = current_time + RenameMeNameSpace::Dsme_Poweroff_Threshold ;

  ticker_t tick ; // default value: invalid
  typedef set<event_pair>::const_iterator iterator ;
  for(iterator it = bootup.begin(); it != bootup.end(); ++it)
  {
    if(it->first <= threshold)
      continue ;
    tick = it->first ;
    break ;
  }

  return tick ;
}

void state_queued::filter_closed(filter_state *f_st)
{
  log_assert(!f_st->is_open) ;
  queue_pause x(om) ;
  typedef set<event_pair>::iterator iterator ;
  bool event_found = false ;
  log_debug("event_found=%d", event_found) ;
  for(iterator it=queue.begin(); it!=queue.end(); ++it)
  {
    if(! f_st->filter(it->second))
      continue ;
    event_found = true ;
    log_debug("event_found=%d", event_found) ;
    log_debug("event [%u] found in state '%s', requesting staet '%s'", it->second->cookie.value(), name, f_st->name) ;
    om->request_state(it->second, f_st) ;
  }
  log_debug("event_found=%d", event_found) ;
  if(event_found)
    om->process_transition_queue() ;
}

bool state_flt_conn::filter(event_t *e)
{
  return (e->flags & EventFlags::Need_Connection) != 0 ;
}

bool state_flt_alrm::filter(event_t *e)
{
  return (e->flags & EventFlags::Alarm) != 0 ;
}

bool state_flt_user::filter(event_t *e)
{
  return (e->flags & EventFlags::User_Mode) != 0 ;
}

void state_missed::enter(event_t *e)
{
  state::enter(e) ;
  const char *next_state = "SKIPPED" ;

  e->flags |= EventFlags::Missed ;

  if(e->flags & EventFlags::Trigger_If_Missed)
    next_state = "ARMED" ;
  om->request_state(e, next_state) ;
}

void state_due::enter(event_t *e)
{
  state::enter(e) ;
#if 0 // TRIGGERED state seems to be better for this
  // Frist get rif of one time trigger info:
  e->ticker = ticker_t() ;
  e->invalidate_t() ;
#endif

  // assume it's not missed, this flag will be set in MISSED state
  e->flags &= ~ EventFlags::Missed ;

  const char *next_state = "ARMED" ;
  if(e->flags & EventFlags::Empty_Recurring)
    next_state = "RECURRED" ;
  else if(om->transition_started() - e->trigger > RenameMeNameSpace::Missing_Threshold)
    next_state = "MISSED" ;
  om->request_state(e, next_state) ;
  log_debug("event [%d]: e->trigger=%ld, now=%ld, state->%s", e->cookie.value(), e->trigger.value(), om->transition_started().value(), next_state) ;
  e->last_triggered = om->transition_started() ;
  e->flags &= ~ EventFlags::Snoozing ;
}

void state_snoozed::enter(event_t *e)
{
  state::enter(e) ;
  log_assert(e->to_be_snoozed > 0) ;
  // compute next trigger time and jump back to queue

  if(e->flags & EventFlags::Aligned_Snooze)
    e->ticker = ticker_align(e->last_triggered, e->to_be_snoozed, om->transition_started());
  else
    e->ticker = om->transition_started() + e->to_be_snoozed ;

  e->flags |= EventFlags::Snoozing ;
  e->to_be_snoozed = 0 ; // doesn't need it anymore
  om->request_state(e, "SCHEDULER") ;
  om->invoke_process_transition_queue() ;
}

ticker_t state_recurred::apply_pattern(broken_down_t &t, int wday, const recurrence_pattern_t *p)
{
  unsigned nxt_year = t.year + 1 ;
  if(broken_down_t::YEARX <= nxt_year)
    -- nxt_year ;
  broken_down_t started = t ;
  for(bool today_flag=true;  t.find_a_good_day(p, wday, today_flag, nxt_year) ; today_flag=false)
  {
    log_debug() ;
    broken_down_t td = t ;
    if(!today_flag)
      td.hour = td.minute = 0 ;
    while(td.find_a_good_minute(p))
    {
      struct tm tm ;
      td.to_struct_tm(&tm) ;
      log_debug("td=(%s)", td.str().c_str()) ;
      log_debug("tm=%s", tm_str(&tm).c_str()) ;
      time_t time = mktime(&tm) ;
      log_debug("time=%d", (int)time) ;
      if(time==time_t(-1))
        continue ;
      log_debug() ;
      if(time <= om->transition_started().value())
      {
        log_debug() ;
        td.increment_min(1) ;
        log_debug() ;
        continue ;
      }
      log_debug() ;
      if(!td.same_struct_tm(&tm))
        continue ;
      log_debug() ;
      t = td ;
      return ticker_t(time) ;
    }
  }
  log_debug() ;
  return ticker_t(0) ;
}

void state_recurred::enter(event_t *e)
{
  state::enter(e) ;
  switch_timezone x(e->tz) ;
  broken_down_t best, now ;
  int now_wday ;
  now.from_time_t(om->transition_started(), &now_wday) ;
  ticker_t best_ticker = ticker_t(0) ;
  for(unsigned i=0; i<e->recrs.size(); ++i)
  {
    broken_down_t t = now ;
    ticker_t res = apply_pattern(t, now_wday, &e->recrs[i]) ;
    if(res.is_valid() && (!best_ticker.is_valid() || res<best_ticker))
      best = t, best_ticker = res ;
  }
  const char *next_state = "QENTRY" ;
  if(best_ticker.is_valid())
  {
    e->flags &= ~ EventFlags::Empty_Recurring ;
    e->trigger = best_ticker ;
  }
  else
  {
    e->flags |= EventFlags::Empty_Recurring ;
    e->t.year = now.year+1, e->t.month = 12, e->t.day = 31, e->t.hour = 0, e->t.minute = 42 ;
    if(e->t.is_valid()) // it valid from until 2036
      next_state = "SCHEDULER" ; // back to scheduler
    else
      next_state = "ABORTED" ; // TODO: FAILED is better here
  }
  om->request_state(e, next_state) ;
}

void state_triggered::enter(event_t *e)
{
  state::enter(e) ;

  // Frist get rid of one time trigger info:
  e->ticker = ticker_t() ;
  e->invalidate_t() ;

  if(e->flags & EventFlags::Single_Shot)
    e->recrs.resize(0) ; // no recurrence anymore
  om->request_state(e, e->flags & EventFlags::Reminder ? "DLG_WAIT" : "SERVED") ;
  om->process_transition_queue() ;
}

QString state_button::init_name(signed no)
{
  QString template_str = "BUTTON_%1_%2" ;
  return QString(template_str).arg(no>0 ? "APP" : "SYS").arg(no>0 ? no : -no) ;
}

state_button::state_button(machine *am, signed n) : state(init_name(n).toStdString().c_str(), am)
{
  no = n ;
  set_action_mask(no>0 ? ActionFlags::app_button(no) : ActionFlags::sys_button(-no)) ;
}

void state_button::enter(event_t *e)
{
  log_debug("Voland-response: cookie=%d, button-value=%d", e->cookie.value(), no) ;
  state::enter(e) ;
  int snooze_length = 0 ;

  if (e->tsz_max)
  {
    log_debug("processing restricted timeout snooze event: cookie=%d, max=%d, count=%d", e->cookie.value(), e->tsz_max, e->tsz_counter) ;
    if (no!=0)
      e->tsz_counter=0 ;
    else if (e->tsz_counter < e->tsz_max)
    {
      ++ e->tsz_counter ;
      snooze_length = e->snooze[0] ;
    }
    log_debug("processed: cookie=%d, max=%d, count=%d, snooze=%d", e->cookie.value(), e->tsz_max, e->tsz_counter, snooze_length) ;
  }
  else if (no==0)
    snooze_length = e->snooze[0] ;

  if(no==-1) // system wide default snooze button #1
    snooze_length = +1 ;
  else if(no>0)
  {
    log_assert((unsigned)no < e->snooze.size()) ;
    snooze_length = e->snooze[no] ;
  }

  // handle special value, +1 means default snooze
  // It's hardcoded for 5min for now...
  if(snooze_length==+1)
    snooze_length = om->owner->settings->default_snooze() ;

  if(snooze_length > 0)
  {
    e->to_be_snoozed = snooze_length ;
    om->request_state(e, "SNOOZED") ;
  }
  else
  {
    om->request_state(e, "SERVED") ;
  }
}

void state_served::enter(event_t *e)
{
  state::enter(e) ;
  bool recu = e->has_recurrence() ;
  bool keep = e->to_be_keeped() ;
  om->request_state(e, recu ? "RECURRED" : keep ? "TRANQUIL" : "REMOVED") ;
  om->process_transition_queue() ;
}

void state_removed::enter(event_t *e)
{
  state::enter(e) ;
  om->request_state(e, "FINALIZED") ;
  om->process_transition_queue() ;
}

void state_finalized::enter(event_t *e)
{
  state::enter(e) ;
  om->request_state(e, (state*)NULL) ;
  om->process_transition_queue() ;
}

void state_aborted::enter(event_t *e)
{
  state::enter(e) ;
  om->request_state(e, "FINALIZED") ;
  om->process_transition_queue() ;
}

void state_dlg_wait::enter(event_t *e)
{
  e->flags |= EventFlags::In_Dialog ;
  if(!is_open)
    emit voland_needed() ;
  gate_state::enter(e) ;
}

void state_dlg_cntr::open()
{
  log_debug() ;
  if (not events.empty())
    request_voland() ;
  concentrating_state::open() ;
  log_debug() ;
}

void state_dlg_cntr::send_back()
{
  log_debug() ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    om->request_state(*it, back) ;
  if (not events.empty())
    om->process_transition_queue() ;
  log_debug() ;
}

void state_dlg_cntr::request_voland()
{
  if (events.empty())
    return ; // avoid a memory leak for 'w' below.

  QList<QVariant> reminders ;
  request_watcher_t *w = new request_watcher_t(om) ;
  Maemo::Timed::Voland::Interface ifc ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
  {
    event_t *e = *it ;
    w->attach(e) ;
    Maemo::Timed::Voland::reminder_pimple_t *p = new Maemo::Timed::Voland::reminder_pimple_t ;
    log_debug("was soll ich schon vergessen haben?") ;
    p->flags = e->flags & EventFlags::Voland_Mask ;
    log_debug() ;
    p->cookie = e->cookie.value() ;
    log_debug() ;
    map_std_to_q(e->attr.txt, p->attr) ;
    log_debug() ;
    p->buttons.resize(e->b_attr.size()) ;
    log_debug() ;
    for(int i=0; i<p->buttons.size(); ++i)
      map_std_to_q(e->b_attr[i].txt, p->buttons[i].attr) ;
    log_debug() ;
    log_debug() ;
    Maemo::Timed::Voland::Reminder R(p) ;
    reminders.push_back(QVariant::fromValue(R)) ;
#if 1 // GET RID OF THIS PIECE SOON !
    log_debug() ;
    Maemo::Timed::Voland::Reminder RR = R ;
    ifc.open_async(RR); // fire and forget
    log_debug() ;
#endif
    log_debug() ;
  }
  log_debug() ;
  QDBusPendingCall async = ifc.open_async(reminders) ;
  w->watch(async) ;
}

void state_dlg_requ::enter(event_t *e)
{
  gate_state::enter(e) ;
#if 0
  Maemo::Timed::Voland::Interface ifc ;
  if(!ifc.isValid())
  {
    string msg = Maemo::Timed::Voland::bus().lastError().message().toStdString() ;
    log_critical("Can't use voland interface: %s", msg.c_str()) ;
    return ;
  }
  Maemo::Timed::Voland::reminder_pimple_t *p = new Maemo::Timed::Voland::reminder_pimple_t ;
  log_debug("was vergessen?") ;
  p->flags = e->flags & EventFlags::Voland_Mask ;
  p->cookie = e->cookie.value() ;
  map_std_to_q(e->attr.txt, p->attr) ;
  p->buttons.resize(e->b_attr.size()) ;
  for(int i=0; i<p->buttons.size(); ++i)
    map_std_to_q(e->b_attr[i].txt, p->buttons[i].attr) ;
  QDBusPendingCall async = ifc.open_async(Maemo::Timed::Voland::Reminder(p));
  if(e->dialog_req_watcher)
  {
    log_error("orphan dialog_req_watcher=%p, e=%p, cookie=%d", e->dialog_req_watcher, e, e->cookie.value()) ;
    delete e->dialog_req_watcher ;
  }
  e->dialog_req_watcher = new QDBusPendingCallWatcher(async);
  om->watcher_to_event[e->dialog_req_watcher] = e ;
  QObject::connect(e->dialog_req_watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), om, SLOT(call_returned(QDBusPendingCallWatcher*))) ;
#endif
}

void state_dlg_requ::abort(event_t *e)
{
  if (e->request_watcher)
    e->request_watcher->detach(e) ;
#if 0
  {
    om->watcher_to_event.erase(e->dialog_req_watcher) ;
    delete e->dialog_req_watcher ;
    e->dialog_req_watcher = NULL ;
  }
#endif

  Maemo::Timed::Voland::Interface ifc ;
  ifc.close_async(e->cookie.value()) ;

  io_state::abort(e) ;
}

void state_dlg_user::abort(event_t *e)
{
  Maemo::Timed::Voland::Interface ifc ;
  ifc.close_async(e->cookie.value()) ;

  io_state::abort(e) ;
}

void cluster_queue::enter(event_t *e)
{
  log_debug() ;
  bool empty_r = e->flags & EventFlags::Empty_Recurring ;
  bool alarm = e->flags & EventFlags::Alarm ;
  if(alarm && !empty_r)
  {
    QString key = QString("%1").arg(e->cookie.value()) ;
    uint64_t value = (uint64_t)nanotime_t::NANO * e->trigger.value() ;
    alarm_triggers.insert(key, value) ;
    log_debug("inserted %s=>%lld, state=%s", key.toStdString().c_str(), value, e->st->name) ;
    om->context_changed = true ;
  }
}

void cluster_queue::leave(event_t *e)
{
  bool empty_r = e->flags & EventFlags::Empty_Recurring ;
  bool alarm = e->flags & EventFlags::Alarm ;
  log_debug() ;
  if(alarm && !empty_r)
  {
    log_debug() ;
    QString key = QString("%1").arg(e->cookie.value()) ;
    alarm_triggers.remove(key) ;
    log_debug("removed %s=>'' state=%s", key.toStdString().c_str(), e->st ? e->st->name : "null") ;
    om->context_changed = true ;
  }
}

void cluster_dialog::enter(event_t *e)
{
  bool is_bootup = e->flags & EventFlags::Boot ;
  if(is_bootup)
    log_debug("insertng [%d]  to  cluster_dialog", e->cookie.value()) ;
  if(is_bootup)
    bootup_events.insert(e) ;
}

void cluster_dialog::leave(event_t *e)
{
  if(bootup_events.count(e))
    log_debug("removing [%d] from cluster_dialog", e->cookie.value()) ;
  bootup_events.erase(e) ;
}

bool cluster_dialog::has_bootup_events()
{
  return !bootup_events.empty() ;
}
