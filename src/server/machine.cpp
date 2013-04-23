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
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "../common/log.h"

#include "state.h"
#include "event.h"
#include "machine.h"
#include "timed.h"
#include "cluster.h"

machine_t::machine_t(const Timed *daemon) : timed(daemon)
{
  log_debug() ;
  // T = transition state
  // IO = waiting for i/o state
  // G = gate state
  // C = concentrating gate state
  // F = filtering state
  // A = actions allowed
  // -->NEW loaded as new
  // -->DUE loaded as due
  abstract_state_t *S[] =
  {
    state_start = new state_start_t(this),                     // T
    state_epoch = new state_epoch_t(this),                     // T
    state_waiting = new state_waiting_t(this),                 // IO G
    state_new = new state_new_t(this),                         // T
    state_scheduler = new state_scheduler_t(this),             // T
    state_qentry = new state_qentry_t(this),                   // T
    state_flt_conn = new state_flt_conn_t(this),               // IO G F -->NEW
    state_flt_alrm = new state_flt_alrm_t(this),               // IO G F -->NEW
    state_flt_user = new state_flt_user_t(this),               // IO G F -->NEW
    state_queued = new state_queued_t(this),                   // IO A   -->NEW

    state_due = new state_due_t(this),                         // T

    state_missed = new state_missed_t(this),                   // T A
    state_skipped = new state_skipped_t(this),                 // T

    state_armed = new state_armed_t(this),                     // IO G
    state_triggered = new state_triggered_t(this),             // T A

    state_dlg_wait = new state_dlg_wait_t(this),               // IO G   -->DUE
    state_dlg_cntr = new state_dlg_cntr_t(this),               // IO C   -->DUE
    state_dlg_requ = new state_dlg_requ_t(this),               // IO G   -->DUE
    state_dlg_user = new state_dlg_user_t(this),               // IO G   -->DUE
    state_dlg_resp = new state_dlg_resp_t(this),               // T

   /* state_buttons: below */                                  // T A

    state_snoozed = new state_snoozed_t(this),                 // T
    state_recurred = new state_recurred_t(this),               // T
    state_served = new state_served_t(this),                   // T
    state_tranquil = new state_tranquil_t(this),               // IO A -->DUE

    state_removed = new state_removed_t(this),                 // T
    state_aborted = new state_aborted_t(this),                 // T
    state_finalized = new state_finalized_t(this),             // T A
    NULL
  } ;
  log_debug() ;
  for(int i=0; S[i]; ++i)
    states.insert(S[i]) ;

  log_debug() ;
  for(int i=0; i<=Maemo::Timed::Number_of_Sys_Buttons; ++i)
    states.insert(buttons[-i] = new state_button_t(this, -i)) ;

  log_debug() ;
  for(int i=1; i<=Maemo::Timed::Max_Number_of_App_Buttons; ++i)
    states.insert(buttons[i] = new state_button_t(this, i)) ;

  for (set<abstract_state_t*>::iterator it=states.begin(); it!=states.end(); ++it)
    (*it)->resolve_names() ;

  log_debug() ;
  state_triggered->set_action_mask(ActionFlags::State_Triggered) ;
  state_queued->set_action_mask(ActionFlags::State_Queued) ;
  state_missed->set_action_mask(ActionFlags::State_Missed) ;
  state_tranquil->set_action_mask(ActionFlags::State_Tranquil) ;
  state_finalized->set_action_mask(ActionFlags::State_Finalized) ;
  state_due->set_action_mask(ActionFlags::State_Due) ;
  state_snoozed->set_action_mask(ActionFlags::State_Snoozed) ;
  state_served->set_action_mask(ActionFlags::State_Served) ;
  state_aborted->set_action_mask(ActionFlags::State_Aborted) ;
  // states_failed->set_action_mask(ActionFlags::State_Failed) ;

#if 0
  log_debug() ;
  io_state *queued = dynamic_cast<io_state*> (states["QUEUED"]) ;
  log_assert(queued!=NULL) ;

  gate_state *armed = dynamic_cast<gate_state*> (states["ARMED"]) ;
  log_assert(armed!=NULL) ;
  armed->open() ; // will be closed in some very special situations

  log_debug() ;
  gate_state *dlg_wait = dynamic_cast<gate_state*> (states["DLG_WAIT"]) ;
  gate_state *dlg_requ = dynamic_cast<gate_state*> (states["DLG_REQU"]) ;
  gate_state *dlg_user = dynamic_cast<gate_state*> (states["DLG_USER"]) ;
  gate_state *dlg_cntr = dynamic_cast<gate_state*> (states["DLG_CNTR"]) ;
  log_assert(dlg_wait!=NULL) ;
  log_assert(dlg_requ!=NULL) ;
  log_assert(dlg_user!=NULL) ;
  log_assert(dlg_cntr!=NULL) ;
#endif

  state_armed->open() ;

  QObject::connect(state_dlg_wait, SIGNAL(voland_needed()), this, SIGNAL(voland_needed())) ;

  QObject::connect(state_dlg_wait, SIGNAL(closed()), state_dlg_requ, SLOT(open())) ;
  QObject::connect(state_dlg_wait, SIGNAL(closed()), state_dlg_user, SLOT(open())) ;
  QObject::connect(state_dlg_requ, SIGNAL(closed()), state_dlg_wait, SLOT(open())) ;

  QObject::connect(this, SIGNAL(voland_registered()), state_dlg_requ, SLOT(close())) ;
  QObject::connect(this, SIGNAL(voland_registered()), state_dlg_user, SLOT(close())) ;
  QObject::connect(this, SIGNAL(voland_unregistered()), state_dlg_wait, SLOT(close())) ;
  QObject::connect(this, SIGNAL(voland_unregistered()), state_dlg_cntr, SLOT(send_back())) ;

  QObject::connect(state_queued, SIGNAL(sleep()), state_dlg_cntr, SLOT(open()), Qt::QueuedConnection) ;
  QObject::connect(state_dlg_wait, SIGNAL(opened()), state_dlg_cntr, SLOT(open()), Qt::QueuedConnection) ;

#if 0
  log_debug() ;
  filter_state *flt_conn = dynamic_cast<filter_state*> (states["FLT_CONN"]) ;
  filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
  filter_state *flt_user = dynamic_cast<filter_state*> (states["FLT_USER"]) ;
  log_assert(flt_conn) ;
  log_assert(flt_alrm) ;
  log_assert(flt_user) ;
#endif

  QObject::connect(state_flt_conn, SIGNAL(closed(abstract_filter_state_t*)), state_queued, SLOT(filter_closed(abstract_filter_state_t*))) ;
  QObject::connect(state_flt_alrm, SIGNAL(closed(abstract_filter_state_t*)), state_queued, SLOT(filter_closed(abstract_filter_state_t*))) ;
  QObject::connect(state_flt_user, SIGNAL(closed(abstract_filter_state_t*)), state_queued, SLOT(filter_closed(abstract_filter_state_t*))) ;

  log_debug() ;
  QObject::connect(this, SIGNAL(engine_pause(int)), state_queued, SLOT(engine_pause(int))) ;
  log_debug() ;
  initial_pause = new pause_t(this) ;
  log_debug() ;

  cluster_queue = new cluster_queue_t(this) ;
  log_debug() ;
  clusters[cluster_queue->bit] = cluster_queue ;
  log_debug() ;

  cluster_dialog = new cluster_dialog_t(this) ;
  log_debug() ;
  clusters[cluster_dialog->bit] = cluster_dialog ;
  log_debug() ;
  signalled_bootup = -1 ; // no signal sent yet
  signalled_non_boot_event = -1;
  log_debug() ;

  log_debug("machine->settings->alarms_are_enabled=%d", timed->settings->alarms_are_enabled) ;
  log_debug() ;
  alarm_gate(timed->settings->alarms_are_enabled) ;
  log_debug() ;

  transition_start_time = ticker_t(0) ;
  transition_time_adjustment.set(0) ;
  log_debug() ;
  next_cookie = 1 ;
  log_debug() ;
  context_changed = false ;
  log_debug("last line") ;
}

machine_t::~machine_t()
{
  log_debug() ;

  log_notice("deleting events") ;
  for (map<cookie_t, event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
    delete it->second ;

  log_notice("deleting clusters") ;
  for (map<uint32_t, abstract_cluster_t*>::iterator it=clusters.begin(); it!=clusters.end(); ++it)
    delete it->second ;

  log_notice("deleting states") ;
  for (set<abstract_state_t*>::iterator it=states.begin(); it!=states.end(); ++it)
    delete *it ;

  log_debug() ;
}

void machine_t::start()
{
  delete initial_pause ;
  initial_pause = NULL ;
  process_transition_queue() ;
}

void machine_t::device_mode_detected(bool user_mode)
{
  if(user_mode)
    state_flt_user->open() ;
  else
    state_flt_user->close() ;
  log_info("device_mode_detected: %s", user_mode ? "USER" : "ACT_DEAD") ;
}

bool machine_t::is_epoch_open()
{
  return state_epoch->is_open ;
}

void machine_t::open_epoch()
{
  state_epoch->open() ;
}

string machine_t::s_states()
{
  ostringstream os ;
  map<string, set<event_t*> > s2e ;
  for (map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
  {
    event_t *e = it->second ;
    string name = e->get_state()==NULL ? "<null>" : e->get_state()->name() ;
    s2e[name].insert(e) ;
  }

  for (map<string, set<event_t*> >::const_iterator it=s2e.begin(); it!=s2e.end(); ++it)
    for (set<event_t*>::const_iterator q=it->second.begin(); q!=it->second.end(); ++q)
      os << (q==it->second.begin() ? string(it==s2e.begin()?"":" ")+it->first+":" : ",") << (*q)->cookie.value() ;

  return os.str() ;
}

string machine_t::s_transition_queue()
{
  ostringstream os ;
  for(deque<pair<event_t*, abstract_state_t*> >::const_iterator it=transition_queue.begin(); it!=transition_queue.end(); ++it)
  {
    cookie_t c = it->first->cookie ;
    abstract_state_t *s = it->second ;
    bool first = it==transition_queue.begin() ;
    os << (first ? "" : ", ") << c.value() << "->" << (s?s->name():"null") ;
  }
  return os.str() ;
}

void machine_t::process_transition_queue()
{
  if(transition_in_progress())
  {
    log_debug("process_transition_queue() is already in progress, returning") ;
    return ; // never do it recursively
  }
  // log_debug("begin processing, states: %s tqueue: %s" , s_states().c_str(), s_transition_queue().c_str()) ;
  transition_start_time = ticker_t(now()) ;
  bool queue_changed = false ;
  for(; not transition_queue.empty(); queue_changed = true, transition_queue.pop_front())
  {
#define state_name(p) (p?p->name():"null")
    event_t *e = transition_queue.front().first ;
    abstract_state_t *new_state = transition_queue.front().second ;
    if (not is_event_registered(e))
    {
      log_error("requested to move destroyed event %p to state '%s'", e, state_name(new_state)) ;
      continue ;
    }
    abstract_state_t *old_state = e->get_state() ;
    log_notice("State transition %d:'%s'->'%s'", e->cookie.value(), state_name(old_state), state_name(new_state)) ;
    if (new_state==old_state)
      log_critical("Event %d: new_state=old_state='%s'", e->cookie.value(), old_state->name()) ;
#undef state_name
    if(old_state)
      old_state->leave(e) ;
    e->set_state(new_state) ;
    if(new_state)
    {
      new_state->enter(e) ;
      e->sort_and_run_actions(new_state->get_action_mask()) ;
    }
    else
    {
      log_notice("Destroying the event %u (event object %p)", e->cookie.value(), e) ;
      unregister_event(e) ;
      delete e ;
    }
  }
  // log_debug("processing done,  states: %s tqueue: %s" , s_states().c_str(), s_transition_queue().c_str()) ;
  transition_start_time = ticker_t(0) ;
  transition_time_adjustment.set(0) ;
  if(queue_changed)
    emit queue_to_be_saved() ;
  if(context_changed)
    send_queue_context() ;
  send_bootup_signal() ;
}

ticker_t machine_t::calculate_bootup()
{
  if (cluster_dialog->has_bootup_events())
    return ticker_t(1) ; // right now
  return state_queued->next_bootup() ;
}

void machine_t::send_bootup_signal()
{
  log_debug() ;
  if (is_frozen())
  {
    log_debug("skipping send_bootup_signal() because the machine is frozen") ;
    return ;
  }
  ticker_t tick = calculate_bootup() ;
  int32_t next_bootup = 0 ;
  if(tick.is_valid())
    next_bootup = tick.value() ;

  int32_t next_non_boot_event = 0;
  tick = state_queued->next_event_without_bootflag();
  if (tick.is_valid())
    next_non_boot_event = tick.value();

  log_debug("signalled_bootup=%d, next_bootup=%d", signalled_bootup, next_bootup) ;
  log_debug("signalled_event=%d, next_event=%d", signalled_non_boot_event, next_non_boot_event);
  if((signalled_bootup < 0 || signalled_bootup != next_bootup)
     || (signalled_non_boot_event < 0 || signalled_non_boot_event != next_non_boot_event)) {
    emit next_bootup_event(signalled_bootup = next_bootup, signalled_non_boot_event = next_non_boot_event);
  }
  log_debug() ;
}

void machine_t::invoke_process_transition_queue()
{
  int methodIndex = this->metaObject()->indexOfMethod("process_transition_queue()");
  QMetaMethod method = this->metaObject()->method(methodIndex);
  method.invoke(this, Qt::QueuedConnection);
}

void machine_t::reshuffle_queue(const nanotime_t &back)
{
  abstract_io_state_t *from[] = {state_queued, state_flt_conn, state_flt_alrm, state_flt_user, NULL} ;
  for(abstract_io_state_t **p=from, *state; (state=*p); ++p)
  {
    for(set<event_t*>::iterator it=state->events.begin(); it!=state->events.end(); ++it)
    {
      event_t *e = *it ;
      bool snoozing = e->flags & EventFlags::Snoozing ;
      bool system_time_changing = not back.is_zero() ;
      bool queued = state == state_queued ;
      bool trigger_when_ajusting = e->flags & EventFlags::Trigger_When_Adjusting ;
      bool trigger_when_settings_changed = e->flags & EventFlags::Trigger_When_Settings_Changed ;

      if (snoozing and system_time_changing)
      {
        e->ticker = e->trigger + (- back.to_time_t()) ;
        state_scheduler->go_to(e) ;
      }

      if (snoozing)
        continue ;

      if (queued and system_time_changing and trigger_when_ajusting)
      {
        state_armed->go_to(e) ;
        transition_time_adjustment = back ;
        continue ;
      }

      if (queued and trigger_when_settings_changed)
      {
        state_armed->go_to(e) ;
        continue ;
      }

      if (e->has_ticker() and not system_time_changing)
        continue ;

      if(e->flags & EventFlags::Empty_Recurring) // TODO: rewrite recurrence calculation and get rid of "Empty_Recurring"
        e->invalidate_t() ;

      state_scheduler->go_to(e) ;
    }
  }
  process_transition_queue() ;
}

void machine_t::request_state(event_t *e, abstract_state_t *st)
{
  log_debug("request_state(%u,%s)", e->cookie.value(), st?st->name():"null") ;
  transition_queue.push_back(make_pair(e, st)) ;
  // log_debug("done; transition_queue: %s; states: %s", s_transition_queue().c_str(), s_states().c_str()) ;
}

#if 0
void machine_t::request_state(event_t *e, const char *state_name)
{
  request_state(e, string(state_name)) ;
}

void machine_t::request_state(event_t *e, const string &state_name)
{
  state *new_state = NULL ;
  if(not state_name.empty())
  {
    map<string, state*>::iterator it = states.find(state_name) ;
    log_assert(it!=states.end(), "Unknown state: '%s'", state_name.c_str()) ;
    new_state = it->second ;
  }
  request_state(e, new_state) ;
}
#endif

void machine_t::send_queue_context()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // TODO: add Qt5 replacement for ContextProvider
#else
  // TODO CONTEXT CLEAN UP
  static ContextProvider::Property alarm_triggers_p("Alarm.Trigger") ;
  static ContextProvider::Property alarm_present_p("Alarm.Present") ;

  alarm_triggers_p.setValue(QVariant::fromValue(cluster_queue->alarm_triggers)) ;
  alarm_present_p.setValue(not cluster_queue->alarm_triggers.isEmpty()) ;
  context_changed = false ;
#endif
}

cookie_t machine_t::add_event(const Maemo::Timed::event_io_t *eio, bool process_queue, const credentials_t *creds, const QDBusMessage *p_message)
{
  // The credentials for the event are either already known (creds)
  //   or have to be established by the QDBusMessage structure (from dbus daemon)
  // Using pointers instead of usual C++ references, just because a NULL-reference
  //   usually confuses people (though working just fine)

#if 0
  if (event_t *e = new event_t)
  {
    // #include "simple-event.c++"
    #include "clock-ui-event.c++"
#else
  if (event_t *e = event_t::from_dbus_iface(eio))
  {
    if (e->actions.size() > 0)
      e->client_creds = creds ? new credentials_t(*creds) : new credentials_t(*p_message) ;
#endif
    register_event(e) ;

    if (process_queue)
      invoke_process_transition_queue() ;
    log_info("new event: cookie=%d, object=%p", e->cookie.value(), e) ;
    return e->cookie ;
  }

  return cookie_t(0) ;
}

void machine_t::register_event(event_t *e)
{
  if (not e->cookie.is_valid())
    e->cookie = cookie_t(next_cookie++) ;
  events[e->cookie] = e ;
  state_start->go_to(e) ;
}

void machine_t::unregister_event(event_t *e)
{
  log_assert(is_event_registered(e)) ;
  if (e->request_watcher)
    e->request_watcher->detach(e) ;
  log_assert(not e->request_watcher) ;

  events.erase(e->cookie) ;
}

bool machine_t::is_event_registered(event_t *e)
{
  // XXX: it could be an invalid read there
  cookie_t cookie = e->cookie ;

  map<cookie_t, event_t*>::iterator it = events.find(cookie) ;
  return it!=events.end() && it->second==e ;
}

void machine_t::add_events(const Maemo::Timed::event_list_io_t &lst, QList<QVariant> &res, const QDBusMessage &message)
{
  credentials_t *creds = NULL ;
  bool valid = false ;
  for(int i=0; i<lst.ee.size(); ++i)
  {
#if 1
    bool need_credentials = lst.ee[i].actions.size() > 0 ;
    if (need_credentials and not creds)
      creds = new credentials_t(message) ;
    unsigned cookie = add_event(&lst.ee[i], false, creds, NULL).value() ;
#else // make some testing here
    unsigned cookie = add_event(&lst.ee[i], false, NULL  , &message).value() ;
#endif
    res.push_back(cookie) ;
    if (cookie)
    {
      valid = true ;
      log_debug("event[%d]: accepted, cookie=%d", i, cookie) ;
    }
    else
      log_warning("event[%d]: rejected", i) ;
  }
  delete creds ;
  if(valid)
    invoke_process_transition_queue() ;
}

void machine_t::query(const QMap<QString,QVariant> &words, QList<QVariant> &res)
{
  vector<string> qk, qv ;
  for(QMap<QString,QVariant>::const_iterator it=words.begin(); it!=words.end(); ++it)
  {
    qk.push_back(string_q_to_std(it.key())) ;
    qv.push_back(string_q_to_std(it.value().toString())) ;
  }
  unsigned N = qk.size() ;
  for(map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
  {
    bool text_matches = true ;
    for(unsigned i=0; text_matches && i<N; ++i)
    {
      map<string,string>::const_iterator F = it->second->attr.txt.find(qk[i]) ;
      if(F==it->second->attr.txt.end())
        text_matches = qv[i]=="" ;
      else
        text_matches = qv[i]==F->second ;
    }
    if(text_matches)
      res.push_back(it->second->cookie.value()) ;
  }
}

void machine_t::get_event_attributes(cookie_t c, QMap<QString,QVariant> &a)
{
  map<cookie_t,event_t*>::iterator it = events.find(c) ;
  if(it==events.end())
    return ;
  event_t *e = it->second ;
  a.insert("STATE", e->state->name()) ;
  a.insert("COOKIE", QString("%1").arg(c.value())) ;
  for(attribute_t::const_iterator at=e->attr.txt.begin(); at!=e->attr.txt.end(); at++)
  {
    QString key = string_std_to_q(at->first) ;
    QString val = string_std_to_q(at->second) ;
    a.insert(key,val) ;
  }
}

void machine_t::get_attributes_by_cookie(uint cookie, QMap<QString,QString> &a)
{
  map<cookie_t,event_t*>::iterator it = events.find(cookie_t(cookie)) ;
  if(it==events.end())
    return ;
  event_t *e = it->second ;
  a.insert("STATE", e->state->name()) ;
  static QString decimal = "%1" ;
  a.insert("COOKIE", decimal.arg(cookie)) ;
  for(attribute_t::const_iterator at=e->attr.txt.begin(); at!=e->attr.txt.end(); at++)
  {
    QString key = string_std_to_q(at->first) ;
    QString val = string_std_to_q(at->second) ;
    a.insert(key,val) ;
  }
}

void machine_t::get_attributes_by_cookies(const QList<uint> &cookies, QMap<uint, QMap<QString,QString> > &a)
{
  for(QList<uint>::const_iterator it=cookies.begin(); it!=cookies.end(); ++it)
  {
    uint cookie = *it ;
    QMap<QString,QString> empty ;
    a[cookie] = empty ;
    get_attributes_by_cookie(cookie, a[cookie]) ;
  }
}

void machine_t::get_cookies_by_attributes(const QMap<QString,QString> &words, QList<uint> &res)
{
  vector<string> keys, values ;
  for(QMap<QString,QString>::const_iterator it=words.begin(); it!=words.end(); ++it)
  {
    keys.push_back(string_q_to_std(it.key())) ;
    values.push_back(string_q_to_std(it.value())) ;
  }
  unsigned N = keys.size() ;
  for(map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
  {
    bool text_matches = true ;
    const map<string,string> &attr = it->second->attr.txt ;
    for(unsigned i=0; text_matches and i<N; ++i)
    {
      map<string,string>::const_iterator F = attr.find(keys[i]) ;
      if (F==attr.end())
        text_matches = values[i].empty() ;
      else
        text_matches = values[i]==F->second ;
    }
    if(text_matches)
      res.push_back(it->second->cookie.value()) ;
  }
}

bool machine_t::dialog_response(cookie_t c, int value)
{
  if (events.count(c)==0)
  {
    log_error("Invalid cookie [%d] in respond", c.value()) ;
    return false ;
  }
  event_t *e = events[c] ;
  if (e->state == state_dlg_requ)
  {
    if (e->request_watcher)
      e->request_watcher->detach(e) ;
  }
  else if (e->state != state_dlg_user)
  {
    log_error("Unexpected response for event [%d] in state '%s'", c.value(), e->state->name()) ;
    return false ;
  }
  if (value < -Maemo::Timed::Number_of_Sys_Buttons)
  {
    log_error("Invalid negative value in respond: event [%d], value=%d", c.value(), value) ;
    return false ;
  }
  int b = e->b_attr.size() ;
  if (value > b)
  {
    log_error("Invalid value in respond: event [%d], value=%d, number of buttons: %d", c.value(), value, b) ;
    return false ;
  }
  log_assert(buttons.count(value)>0, "no button with value=%d present", value) ;
  request_state(e, buttons[value]) ;
  e->flags &= ~ EventFlags::In_Dialog ;
  invoke_process_transition_queue() ;
  return true ;
}

event_t *machine_t::find_event(cookie_t c)
{
  log_assert(!transition_in_progress()) ;
  process_transition_queue() ;

  map<cookie_t,event_t*>::iterator it = events.find(c) ;
  return it==events.end() ? NULL : it->second ;
}

bool machine_t::cancel_by_cookie(cookie_t c) // XXX need some clean up here?
{
  pause_t x(this) ;

  if(event_t *e = find_event(c))
  {
    cancel_event(e) ;
    return true ;
  }
  else
  {
    log_error("[%d]: cookie not found", c.value()) ;
    return false ;
  }
}

void machine_t::cancel_events(const QList<uint> &cookies, QList<uint> &failed)
{
  pause_t x(this) ;
  set<unsigned> done ;
  for (int i=0; i<cookies.size(); ++i)
  {
    unsigned ci = cookies[i] ;
    cookie_t c(ci) ;
    if (done.count(ci)>0)
      continue ;
    done.insert(ci) ;
    if (not cancel_by_cookie(c))
      failed.append(ci) ;
  }
}


void machine_t::cancel_event(event_t *e)
{
  // TODO: assert (queue is paused)
  abstract_io_state_t *st = dynamic_cast<abstract_io_state_t*> (e->state) ;
  log_assert(st) ;
  st->abort(e) ;
}

void machine_t::alarm_gate(bool open)
{
  if (open)
    state_flt_alrm->open() ;
  else
    state_flt_alrm->close() ;
}

uint32_t machine_t::attr(uint32_t mask)
{
  return flags & mask ;
}

iodata::record *machine_t::save(bool for_backup)
{
  iodata::record *r = new iodata::record ;
  iodata::array *q = new iodata::array ;

  for(map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
  {
    event_t *e = it->second ;
    if(for_backup and not (e->flags & EventFlags::Backup))
      continue ;
    q->add(e->save(for_backup)) ;
  }

  r->add("events", q) ;
  if(not for_backup)
    r->add("next_cookie", next_cookie) ;
  return r ;
}

void machine_t::load(const iodata::record *r)
{
  next_cookie = r->get("next_cookie")->value() ;
  const iodata::array *a = r->get("events")->arr() ;
  load_events(a, true, true) ;
}

void machine_t::load_events(const iodata::array *events_data, bool trusted_source, bool use_cookies)
{
  for(unsigned i=0; i < events_data->size(); ++i)
  {
    const iodata::record *ee = events_data->get(i)->rec() ;
    unsigned cookie = use_cookies ? ee->get("cookie")->value() : next_cookie++ ;
    event_t *e = new event_t ;
    e->cookie = cookie_t(cookie) ;

    e->ticker = ticker_t(ee->get("ticker")->value()) ;
    e->t.load(ee->get("t")->rec()) ;

    e->tz = ee->get("tz")->str() ;

    e->attr.load(ee->get("attr")->arr()) ;
    e->flags = ee->get("flags")->decode(event_t::codec) ;
    iodata::load(e->recrs, ee->get("recrs")->arr()) ;
    iodata::load_int_array(e->snooze, ee->get("snooze")->arr()) ;

    const iodata::array *a = ee->get("b_attr")->arr() ;
    unsigned nb = a->size() ;
    e->b_attr.resize(nb) ;
    for(unsigned i=0; i<nb; ++i)
      e->b_attr[i].load(a->get(i)->rec()->get("attr")->arr()) ;

    e->last_triggered = ticker_t(ee->get("dialog_time")->value()) ;
    e->tsz_max = ee->get("tsz_max")->value() ;
    e->tsz_counter = ee->get("tsz_counter")->value() ;
    if(trusted_source)
    {
      iodata::load(e->actions, ee->get("actions")->arr()) ;
      if (e->actions.size()>0)
        e->client_creds = new credentials_t(ee->get("client_creds")->rec()) ;
      const iodata::array *cred_modifier = ee->get("cred_modifier")->arr() ;
      if (cred_modifier->size()>0)
        e->cred_modifier = new cred_modifier_t(cred_modifier) ;
    }

    if(e->flags & EventFlags::Empty_Recurring)
      e->invalidate_t() ;

    register_event(e) ;
  }
}

void machine_t::cancel_backup_events()
{
  // TODO: assert (queue is paused)
  vector<event_t*> backup ;
  for(map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    if (it->second->flags & EventFlags::Backup)
      backup.push_back(it->second) ;

  for(vector<event_t*>::const_iterator it=backup.begin(); it!=backup.end(); ++it)
    cancel_event(*it) ;

  log_debug("cancelled all the bacjup events") ;
}

abstract_state_t *machine_t::state_by_name(const string &name)
{
  for (set<abstract_state_t*>::iterator it=states.begin(); it!=states.end(); ++it)
    if ((*it)->s_name==name)
      return *it ;
  log_abort("invalid state name '%s'", name.c_str()) ;
}

void machine_t::freeze()
{
  log_notice("freezing event machine") ;
  state_waiting->close() ;
  for(set<abstract_state_t*>::iterator it=states.begin(); it!=states.end(); ++it)
    if(abstract_io_state_t *st=dynamic_cast<abstract_io_state_t*> (*it))
      st->abort_all(state_waiting) ;
  invoke_process_transition_queue() ;
}

void machine_t::unfreeze()
{
  log_notice("unfreezing event machine") ;
  state_waiting->open() ;
  send_bootup_signal() ;
}

bool machine_t::is_frozen()
{
  return state_waiting->is_closed() ;
}

request_watcher_t::request_watcher_t(machine_t *owner)
{
  machine = owner ;
  w = NULL ;
}

request_watcher_t::~request_watcher_t()
{
  delete w ;
  for(set<event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    detach_not_destroy(*it) ;
}

void request_watcher_t::watch(const QDBusPendingCall &async_call)
{
  log_assert(w==NULL, "watch() called more then once") ;
  w = new QDBusPendingCallWatcher(async_call) ;
  QObject::connect(w, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(call_returned(QDBusPendingCallWatcher*))) ;
}

void request_watcher_t::attach(event_t *e)
{
  // First, we have to detach a link to an orphaned watcher
  if (e->request_watcher)
  {
    log_assert(e->request_watcher != this, "attaching the same event twice") ;
    e->request_watcher->detach(e) ;
  }

  // Now attach the event to this watcher object
  e->request_watcher = this ;
  this->events.insert(e) ;
}

void request_watcher_t::detach_not_destroy(event_t *e)
{
  // First make sure, it's really attached
  log_assert(e->request_watcher) ;
  log_assert(e->request_watcher==this) ;
  log_assert(this->events.count(e) > 0) ;

  // Now detach:
  e->request_watcher = NULL ;
  this->events.erase(e) ;
}

void request_watcher_t::detach(event_t *e)
{
  detach_not_destroy(e) ;

  // The object should be destroyed if empty
  if (this->events.empty())
    delete this ; // ouch! is it kosher C++ ? I hope so, at least...
}

void request_watcher_t::call_returned(QDBusPendingCallWatcher *w)
{
  log_debug() ;
  log_assert(w==this->w, "oops, somethig is really wrong with qdbus...") ;

  QDBusPendingReply<bool> reply = *w ;

  bool ok = reply.isValid() and reply.value() ;

  abstract_io_state_t *next = machine->state_dlg_user ;

  if (ok)
    log_notice("Voland acknowledge received, moving %d event(s) to DLG_USER", events.size()) ;
  else
  {
    string err = reply.isValid() ? "false returned" : reply.error().message().toStdString() ;
    log_error("Voland call 'create' failed, message: '%s'", err.c_str()) ;
    log_notice("moving %d event(s) to DLG_WAIT", events.size()) ;
    next = machine->state_dlg_wait ;
  }

  for(set<event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    next->go_to(*it) ;

  if (events.size()>0)
    machine->process_transition_queue() ;

  // Don't need the watcher any more
  delete this ;
}

void machine_t::online_state_changed(bool connected)
{
  log_notice("ONLINE state: %s", connected?"connected":"not connected") ;
  if (connected)
    state_flt_conn->open() ;
  else
    state_flt_conn->close() ;
}
