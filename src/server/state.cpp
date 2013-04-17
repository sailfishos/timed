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
#include "../lib/event-io.h"
#include "../lib/nanotime.h"
#include "../lib/aliases.h"

#include "../voland/reminder.h"
#include "../voland/interface.h"
#include "../voland/reminder-pimple.h"

#include "machine.h"
#include "event.h"
#include "misc.h"
#include "timed.h"

#include "state.h"

abstract_state_t::abstract_state_t(const string &state_name, machine_t *owner)
{
  machine = owner ;
  s_name = state_name ;
  action_mask = 0 ;
}

void abstract_state_t::go_to(event_t *e)
{
  machine->request_state(e, this) ;
}

#if 0
void abstract_state_t::enter(event_t *e)
{
  uint32_t cluster_before = e->flags & EventFlags::Cluster_Mask ;
  uint32_t cluster_after = cluster_bits() ;
  uint32_t off = cluster_before & ~cluster_after ;
  uint32_t on = cluster_after & ~cluster_before ;
  log_debug("[%d]->%s before=0x%08X after=0x%08X off=0x%08X on=0x%08X", e->cookie.value(), name(), cluster_before, cluster_after, off, on) ;
  if (off)
    for (uint32_t b; b = (off ^ (off-1)), b &= off ; off ^= b)
      machine->clusters[b]->leave(e) ;

  e->flags &= ~ EventFlags::Cluster_Mask ;
  e->flags |= cluster_after ;
  if (on)
    for(uint32_t b; b = (on ^ (on-1)), b &= on ; on ^= b)
      machine->clusters[b]->enter(e) ;
}
#else
void abstract_state_t::enter(event_t *e)
{
  uint32_t cluster_after = cluster_bits() ;
  if (uint32_t on_xor_off = (e->flags ^ cluster_after) & EventFlags::Cluster_Mask)
  {
    uint32_t on = on_xor_off & cluster_after ;
    if (uint32_t off = on_xor_off ^ on)
      for (uint32_t b; b = (off ^ (off-1)), b &= off ; off ^= b)
        machine->clusters[b]->leave(e) ;

    e->flags ^= on_xor_off ;

    if (on)
      for(uint32_t b; b = (on ^ (on-1)), b &= on ; on ^= b)
        machine->clusters[b]->enter(e) ;
  }
}
#endif

void abstract_io_state_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ; // process clusters
  events.insert(e) ;
}

void abstract_io_state_t::leave(event_t *e)
{
  events.erase(e) ;
  abstract_state_t::leave(e) ; // does nothing
}

void abstract_io_state_t::abort(event_t *e)
{
  machine->state_aborted->go_to(e) ;
  machine->process_transition_queue() ;
}

void abstract_io_state_t::abort_all(abstract_state_t *st)
{
  if (this==st)
    return ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    st->go_to(*it) ;
}

void state_tranquil_t::abort_all(abstract_state_t *)
{
  return ; // do nothing
}

void state_epoch_t::abort_all(abstract_state_t *)
{
  return ; // do nothing
}

abstract_gate_state_t::abstract_gate_state_t(const string &state_name, const string &next_state_name, machine_t *owner)
  : abstract_io_state_t(state_name, owner)
{
  s_next_state = next_state_name ;
  next_state = NULL ;
  is_open = false ;
}

void abstract_gate_state_t::resolve_names()
{
  next_state = machine->state_by_name(s_next_state) ;
  abstract_io_state_t::resolve_names() ;
}

void abstract_gate_state_t::enter(event_t *e)
{
  if(is_open)
  {
    abstract_state_t::enter(e) ;
    machine->request_state(e, next_state) ;
    machine->process_transition_queue() ;
  }
  else
    abstract_io_state_t::enter(e) ;
}

void abstract_gate_state_t::close()
{
  is_open = false ;
  emit closed() ;
}

void abstract_gate_state_t::open()
{
  log_debug() ;
  is_open = true ;
  log_debug() ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    machine->request_state(*it, next_state) ;
  log_debug("events.empty()=%d", events.empty()) ;
  if (not events.empty())
    machine->process_transition_queue() ;
  log_debug() ;
  emit opened() ;
}

void abstract_concentrating_state_t::open()
{
  // Not setting "is_open" to true: it's always closed
  // Not emit any opened() / closed() signals
  log_debug() ;
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    machine->request_state(*it, next_state) ;
  if (not events.empty())
    machine->process_transition_queue() ;
  log_debug() ;
}

abstract_filter_state_t::abstract_filter_state_t(const string &state_name, const string &retry_state_name, const string &thru_state_name, machine_t *owner) :
  abstract_gate_state_t(state_name, retry_state_name, owner)
{
  s_thru_state = thru_state_name ;
  thru_state = NULL ;
  QObject::connect(this, SIGNAL(closed()), this, SLOT(emit_close())) ;
}

void abstract_filter_state_t::resolve_names()
{
  thru_state = machine->state_by_name(s_thru_state) ;
  abstract_gate_state_t::resolve_names() ;
}

void abstract_filter_state_t::enter(event_t *e)
{
  if (is_open or not filter(e))
  {
    machine->request_state(e, thru_state) ;
    machine->process_transition_queue() ;
  }
  else
    abstract_gate_state_t::enter(e) ;
}

// States //

void state_start_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  machine->state_epoch->go_to(e) ;
}

state_epoch_t::state_epoch_t(machine_t *owner) : abstract_gate_state_t("EPOCH", "WAITING", owner)
{
  is_open = access(lost(), F_OK) != 0 ;
}

state_waiting_t::state_waiting_t(machine_t *owner) : abstract_gate_state_t("WAITING", "NEW", owner)
{
  is_open = false ;
}

void state_epoch_t::open()
{
  if (not is_open and unlink(lost())<0)
    log_critical("can't unlink '%s': %m", lost()) ;
  abstract_gate_state_t::open() ;
}

void state_new_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  abstract_state_t *next_state = machine->state_aborted ;  // TODO: make a new state "FAILED" for this case

  if(e->flags & EventFlags::In_Dialog)
  {
    ticker_t t_now = machine->transition_start_time ;

    bool restore = true ;
    if(e->last_triggered.is_valid() && t_now - e->last_triggered > machine->dialog_discard_threshold)
      restore = false ;

    if(restore)
    {
      e->last_triggered = t_now ;
      next_state = machine->state_dlg_wait ;
    }
    else
    {
      e->flags &= ~ EventFlags::In_Dialog ;
      next_state = machine->state_served ;
    }
  }
  else if(e->has_ticker() || e->has_time() || e->has_recurrence())
    next_state = machine->state_scheduler ;
  else if(e->to_be_keeped())
    next_state = machine->state_tranquil ;

  machine->request_state(e, next_state) ;
  machine->process_transition_queue() ;
}

void state_scheduler_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  log_debug() ;
  abstract_state_t *next_state = machine->state_qentry ;
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
    if (not e->has_timezone())
      e->trigger = mktime_local(&T) ;
    else if(Maemo::Timed::is_tz_name(e->tz))
      e->trigger = mktime_oversea(&T, e->tz) ;
    else
      log_error("can't schedule an event for an unknown time zone '%s'", e->tz.c_str()) ;

    log_debug("now=%ld e->trigger=%ld diff=%ld", time(NULL), e->trigger.value(), e->trigger.value()-time(NULL)) ;
    log_debug("e->has_timezone()=%d", e->has_timezone()) ;
    if(!e->trigger.is_valid())
    {
      log_error("Failed to calculate trigger time for %s", e->t.str().c_str()) ;
      next_state = machine->state_aborted ; // TODO: make a new state "FAILED" for this case
    }
  }
  else if(e->has_recurrence())
    next_state = machine->state_recurred ;
  log_debug() ;
  machine->request_state(e, next_state) ;
  log_debug() ;
}

void state_qentry_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  log_assert(e->trigger.is_valid()) ;
  machine->state_flt_conn->go_to(e) ;
}

state_queued_t::state_queued_t(machine_t *owner) : abstract_io_state_t("QUEUED", owner)
{
  pause_x = 0 ;
  alarm_timer = new QTimer ;
  alarm_timer->setSingleShot(true) ;
  connect(alarm_timer, SIGNAL(timeout()), this, SLOT(alarm_timeout())) ;
}

state_queued_t::~state_queued_t()
{
  delete alarm_timer ;
}

void state_queued_t::engine_pause(int dx)
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
    timer_start() ;
  }
}

void state_queued_t::alarm_timeout()
{
  machine_t::pause_t x(machine) ;
  log_info("waked up by alarm_timer") ;
  ticker_t time_now = now() ;
  log_debug("time_now=%ld queue.begin=%ld", time_now.value(), queue.begin()!=queue.end() ? queue.begin()->first.value() : (time_t)-1) ;
  typedef set<event_pair>::iterator iterator ;
  bool event_found = false ;
  for(iterator it=queue.begin(); it!=queue.end() && it->first <= time_now; ++it, event_found=true)
    machine->state_due->go_to(it->second) ;
  if(event_found)
    machine->process_transition_queue() ;
  else
    log_info("no due event found, need to sleep more") ;
}

void state_queued_t::enter(event_t *e)
{
  machine_t::pause_t x(machine) ;
  abstract_io_state_t::enter(e) ;
  event_pair p = make_pair(e->trigger, e) ;
  queue.insert(p) ;
  if(e->flags & EventFlags::Boot)
    bootup.insert(p) ;
}

void state_queued_t::leave(event_t *e)
{
  machine_t::pause_t x(machine) ;
  event_pair p = make_pair(e->trigger, e) ;
  queue.erase(p) ;
  bootup.erase(p) ;
  abstract_io_state_t::leave(e) ;
}

void state_queued_t::timer_start()
{
  if(queue.empty())
  {
    log_notice("go to sleep, no alarm queued") ;
    machine->send_queue_context() ;
    emit sleep() ;
    return ;
  }

  nanotime_t time_to_wait = nanotime_t(queue.begin()->first.value(), 0) - nanotime_t::systime_now() ;
  bool no_sleep = false ;
  if(time_to_wait<0)
  {
    log_notice("no sleep today: an alarm is %s seconds overdue", (-time_to_wait).str().c_str()) ;
    no_sleep = true ;
  }
  else
    log_notice("go to sleep, next alarm in %s seconds", time_to_wait.str().c_str()) ;

  if(no_sleep)
    alarm_timer->start(0) ;
  else
  {
    static const int threshold = 3600+1 ; // One hour
    int milisec ;
    if(time_to_wait<threshold)
      milisec = time_to_wait.sec() * 1000 + time_to_wait.nano() / 1000000 ;
    else
      milisec = (threshold-1) * 1000 ;
    log_notice("starting alarm_timer for %d milisec", milisec) ;
    alarm_timer->start(milisec) ;
    machine->send_queue_context() ;
    emit sleep() ;
  }
}

void state_queued_t::timer_stop()
{
  alarm_timer->stop() ;
}

ticker_t state_queued_t::next_bootup()
{
  if (not bootup.empty())
    return bootup.begin()->first ;
  else
    return ticker_t() ;
}

ticker_t state_queued_t::next_event_without_bootflag()
{
  // No events, return invalid ticker_t
  if (queue.empty())
    return ticker_t();

  // Search for the first event that does not have the EventFlags::Boot
  for(set<event_pair>::iterator it = queue.begin(); it != queue.end(); ++it) {
    if (!(it->second->flags & EventFlags::Boot))
      return it->first;
  }

  // No events without EventFlags::Boot, return invalid ticker_t
  return ticker_t();
}

void state_queued_t::filter_closed(abstract_filter_state_t *f_st)
{
  log_assert(!f_st->is_open) ;
  machine_t::pause_t x(machine) ;
  typedef set<event_pair>::iterator iterator ;
  bool event_found = false ;
  log_debug("event_found=%d", event_found) ;
  for(iterator it=queue.begin(); it!=queue.end(); ++it)
  {
    if(! f_st->filter(it->second))
      continue ;
    event_found = true ;
    log_debug("event_found=%d", event_found) ;
    log_debug("event [%u] found in state '%s', requesting staet '%s'", it->second->cookie.value(), name(), f_st->name()) ;
    machine->request_state(it->second, f_st) ;
  }
  log_debug("event_found=%d", event_found) ;
  if(event_found)
    machine->process_transition_queue() ;
}

bool state_flt_conn_t::filter(event_t *e)
{
  return e->flags & EventFlags::Need_Connection ;
}

bool state_flt_alrm_t::filter(event_t *e)
{
  return e->flags & EventFlags::Alarm ;
}

bool state_flt_user_t::filter(event_t *e)
{
  return e->flags & EventFlags::User_Mode ;
}

void state_missed_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  e->flags |= EventFlags::Missed ;

  if(e->flags & EventFlags::Trigger_If_Missed)
    machine->state_armed->go_to(e) ;
  else
    machine->state_skipped->go_to(e) ;
}

void state_due_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;

  e->flags &= ~ EventFlags::Missed ; // this flag will be set in MISSED state
  e->flags &= ~ EventFlags::Snoozing ;
  e->flags &= ~ EventFlags::Trigger_When_Adjusting ; // A due event will be discarded soon
  e->last_triggered = machine->transition_started() ;

  abstract_state_t *next_state = machine->state_armed ;

  if (e->flags & EventFlags::Empty_Recurring)
    next_state = machine->state_recurred  ;
  else if(machine->transition_started() - e->trigger > RenameMeNameSpace::Missing_Threshold)
    next_state = machine->state_missed ;

  log_notice("due event [%d]: e->trigger=%ld, now=%ld, next_state='%s'", e->cookie.value(), e->trigger.value(), machine->transition_started().value(), next_state->name()) ;
  machine->request_state(e, next_state) ;
}

void state_skipped_t::enter(event_t *e)
{
  machine->state_served->go_to(e) ;
}

void state_snoozed_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  log_assert(e->to_be_snoozed > 0) ;

  // compute next trigger time and jump back to queue
  if(e->flags & EventFlags::Aligned_Snooze)
    e->ticker = ticker_align(e->last_triggered, e->to_be_snoozed, machine->transition_started());
  else
    e->ticker = machine->transition_started() + e->to_be_snoozed ;

  e->flags |= EventFlags::Snoozing ;
  e->to_be_snoozed = 0 ; // doesn't need it anymore
  machine->state_scheduler->go_to(e) ;
  machine->invoke_process_transition_queue() ;
}

static ticker_t recur_irregular_day(const broken_down_t &day, const recurrence_pattern_t *p, time_t threshold)
{
  broken_down_t d = day ;
  d.hour = d.minute = 0 ;
  set<time_t> found ;
  for (bool inc_flag = false; d.find_a_good_minute_with_increment(p, inc_flag); inc_flag = true)
  {
    bool time_is_valid = false ;
    for (int dst=0; dst<=1; ++dst)
    {
      time_t t = d.mktime_strict(dst) ;
      if (t>0)
        time_is_valid = true ;
      if (t>threshold)
        found.insert(t) ;
    }
    if (time_is_valid)
      continue ;
    if (not (p->flags & RecurrenceFlags::Fill_Gaps))
      continue ;
    // now flicking the gap (usually caused by entering DST)
    log_notice("starting search to fill the gap: %s", d.str().c_str()) ;
    for(unsigned day = d.day; day==d.day; d.increment_min(1))
    {
      time_t t = d.mktime_strict() ; // dst is set to -1 here
      log_info("trying '%s' (t=%lld)", d.str().c_str(), (long long)t) ;
      if (t<0)
        continue ;
      log_notice("found a valid minute while flicking the gap: %s", d.str().c_str()) ;
      if (t>threshold)
        found.insert(t) ;
      break ;
    }
  }
  set<time_t>::const_iterator it = found.begin() ;
  if (it==found.end())
    return ticker_t() ;
  else
    return ticker_t(*it) ;
}

static ticker_t recur_regular_day(const broken_down_t &day, const recurrence_pattern_t *p, time_t threshold)
{
  log_debug("day=%s, p->mins=0x%llx, threshold=%lld", day.str().c_str(), p->mins, (long long)threshold) ;
  broken_down_t d = day ;
  for (bool inc_flag = false; d.find_a_good_minute_with_increment(p, inc_flag); inc_flag = true)
  {
    struct tm tm ;
    d.to_struct_tm(&tm) ;
    time_t t = mktime(&tm) ;
    if (t<0 or t<=threshold or not d.same_struct_tm(&tm))
      continue ;
    log_debug("returning t=%lld", (long long)t) ;
    return ticker_t(t) ;
  }
  log_debug("returning t=null") ;
  return ticker_t() ;
}

ticker_t state_recurred_t::apply_pattern(const broken_down_t &start, int wday, const recurrence_pattern_t *p)
{
  broken_down_t day = start ;
  unsigned nxt_year = day.year + 1 ;
  if(broken_down_t::YEARX <= nxt_year)
    -- nxt_year ;
  time_t threshold = machine->transition_started().value() ;
  for(bool today_flag=true;  day.find_a_good_day(p, wday, today_flag, nxt_year) ; today_flag=false)
  {
    if (not today_flag)
      day.hour = day.minute = 0 ;
    ticker_t t = (day.is_a_regular_day() ? recur_regular_day : recur_irregular_day) (day, p, threshold) ;
    if (t.is_valid())
      return t ;
  }
  return ticker_t() ;
}

#if 0 // here is the old pattern scheduling code

ticker_t state_recurred_t::apply_pattern(broken_down_t &t, int wday, const recurrence_pattern_t *p)
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
      if(time <= machine->transition_started().value())
      {
        log_debug() ;
        td.increment_min(1) ;
        log_debug() ;
        continue ;
      }
      log_debug() ;
      if(!td.same_struct_tm(&tm))
      {
        td.increment_min(1) ;
        continue ;
      }
      log_debug() ;
      t = td ;
      return ticker_t(time) ;
    }
  }
  log_debug() ;
  return ticker_t(0) ;
}
#endif

void state_recurred_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  switch_timezone x(e->tz) ;
  broken_down_t now ;
  int now_wday ;
  now.from_time_t(machine->transition_started(), &now_wday) ;
  ticker_t best_ticker = ticker_t(0) ;
  for(unsigned i=0; i<e->recrs.size(); ++i)
  {
    ticker_t res = apply_pattern(now, now_wday, &e->recrs[i]) ;
    if(res.is_valid() && (!best_ticker.is_valid() || res<best_ticker))
      best_ticker = res ;
  }
  abstract_state_t *next_state = machine->state_qentry ;
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
      next_state = machine->state_scheduler ; // back to scheduler
    else
      next_state = machine->state_aborted ; // TODO: FAILED is better here
  }
  machine->request_state(e, next_state) ;
}

void state_triggered_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;

  // Frist get rid of one time trigger info:
  e->ticker = ticker_t() ;
  e->invalidate_t() ;

  if(e->flags & EventFlags::Single_Shot)
    e->recrs.resize(0) ; // no recurrence anymore

  if (e->flags & EventFlags::Reminder)
    machine->state_dlg_wait->go_to(e) ;
  else
    machine->state_served->go_to(e) ;

  machine->process_transition_queue() ;
}

static string button_state_name(signed no)
{
  return str_printf("BUTTON_%s_%d", no>0 ? "APP" : "SYS", no>0 ? no : -no) ;
}

state_button_t::state_button_t(machine_t *owner, signed n) : abstract_state_t(button_state_name(n), owner)
{
  no = n ;
  set_action_mask(no>0 ? ActionFlags::app_button(no) : ActionFlags::sys_button(-no)) ;
}

void state_button_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  log_notice("Voland responded: cookie=%d, button-value=%d", e->cookie.value(), no) ;
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
  if(snooze_length==+1)
    snooze_length = machine->timed->settings->default_snooze() ;

  if(snooze_length > 0)
  {
    e->to_be_snoozed = snooze_length ;
    machine->state_snoozed->go_to(e) ;
  }
  else
    machine->state_served->go_to(e) ;
}

void state_served_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;

  if (e->has_recurrence())
    machine->state_recurred->go_to(e) ;
  else if (e->flags & EventFlags::Trigger_When_Adjusting)
    machine->state_scheduler->go_to(e) ;
  else if (e->to_be_keeped())
    machine->state_tranquil->go_to(e) ;
  else
    machine->state_removed->go_to(e) ;

  machine->process_transition_queue() ;
}

void state_removed_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  machine->state_finalized->go_to(e) ;
  machine->process_transition_queue() ;
}

void state_finalized_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  machine->request_state(e, NULL) ;
  machine->process_transition_queue() ;
}

void state_aborted_t::enter(event_t *e)
{
  abstract_state_t::enter(e) ;
  machine->state_finalized->go_to(e) ;
  machine->process_transition_queue() ;
}

void state_dlg_wait_t::enter(event_t *e)
{
  e->flags |= EventFlags::In_Dialog ;
  if (not is_open)
    emit voland_needed() ;
  abstract_gate_state_t::enter(e) ;
}

state_dlg_cntr_t::state_dlg_cntr_t(machine_t *owner)
  : abstract_concentrating_state_t("DLG_CNTR", "DLG_REQU", owner)
{
  s_back_state = "DLG_WAIT" ;
  back_state = NULL ;
}

void state_dlg_cntr_t::resolve_names()
{
  back_state = machine->state_by_name(s_back_state) ;
  abstract_concentrating_state_t::resolve_names() ;
}

void state_dlg_cntr_t::open()
{
  if (not events.empty())
    request_voland() ;
  abstract_concentrating_state_t::open() ;
}

void state_dlg_cntr_t::send_back()
{
  for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    machine->request_state(*it, back_state) ;
  if (not events.empty())
    machine->process_transition_queue() ;
}

void state_dlg_cntr_t::request_voland()
{
  if (events.empty())
    return ; // avoid a memory leak for 'w' below.

  QList<QVariant> reminders ;
  request_watcher_t *w = new request_watcher_t(machine) ;
  Maemo::Timed::Voland::Interface ifc(QDBusConnection::systemBus());
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
#if 0 // GET RID OF THIS PIECE SOON !
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

void state_dlg_requ_t::enter(event_t *e)
{
  abstract_gate_state_t::enter(e) ;
}

void state_dlg_requ_t::abort(event_t *e)
{
  if (e->request_watcher)
    e->request_watcher->detach(e) ;

  Maemo::Timed::Voland::Interface ifc(QDBusConnection::systemBus());
  ifc.close_async(e->cookie.value()) ;

  abstract_io_state_t::abort(e) ;
}

void state_dlg_user_t::abort(event_t *e)
{
  Maemo::Timed::Voland::Interface ifc(QDBusConnection::systemBus());
  ifc.close_async(e->cookie.value()) ;

  abstract_io_state_t::abort(e) ;
}
