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
#include "f.h"

#include <pwd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <algorithm>
#include <map>
#include <string>
#include <set>
#include <iostream>
#include <sstream>
using namespace std ;

#include <pcrecpp.h>

#include <QDebug>
#include <QMetaMethod>
#include <QString>
#include <QRegExp>
#include <QMap>
#include <QDBusConnection>
#include <QDBusPendingReply>

#include <iodata/validator.h>
#include <qm/log>

#include "timed/event-io.h"
#include "timed/nanotime.h"

#include "automata.h"
#include "states.h"
#include "flags.h"
#include "event.h"
#include "misc.h"
#include "credentials.h"
#include "timed.h"

  state::state(const char *n, machine *m) : om(m)
  {
    name = strdup(n) ;
    log_assert(name!=NULL) ;
    action_mask = 0 ;
  }

  void state::enter(event_t *e)
  {
    uint32_t cluster_before = e->flags & EventFlags::Cluster_Mask ;
    uint32_t cluster_after = cluster_bits() ;
    uint32_t off = cluster_before & ~cluster_after ;
    uint32_t on = cluster_after & ~cluster_before ;
    log_debug("[%d]->%s before=0x%08X after=0x%08X off=0x%08X on=0x%08X", e->cookie.value(), name, cluster_before, cluster_after, off, on) ;
    for(uint32_t b; b = (off ^ (off-1)), b &= off ; off ^= b)
      om->clusters[b]->leave(e) ;

    e->flags &= ~ EventFlags::Cluster_Mask ;
    e->flags |= cluster_after ;
    for(uint32_t b; b = (on ^ (on-1)), b &= on ; on ^= b)
      om->clusters[b]->enter(e) ;
  }

  void state::leave(event_t *)
  {
  }

  io_state::io_state(const char *n, machine *m, QObject *parent)
    : QObject(parent), state(n, m)
  {
  }

  void io_state::enter(event_t *e)
  {
    state::enter(e) ; // process clusters
    events.insert(e) ;
  }

  void io_state::leave(event_t *e)
  {
    events.erase(e) ;
    state::leave(e) ; // does nothing
  }

  void io_state::abort(event_t *e)
  {
    om->request_state(e, "ABORTED") ;
    om->process_transition_queue() ;
  }

  void gate_state::enter(event_t *e)
  {
    if(is_open)
    {
      state::enter(e) ;
      om->request_state(e, nxt_state) ;
      om->process_transition_queue() ;
    }
    else
      io_state::enter(e) ;
  }

  gate_state::gate_state(const char *name, const char *nxt, machine *m, QObject *p) :
    io_state(name, m, p),
    nxt_state(strdup(nxt)),
    is_open(false)
  {
    log_assert(nxt_state!=NULL) ;
  }

  void gate_state::close()
  {
    is_open = false ;
    emit closed() ;
  }

  void gate_state::open()
  {
    log_debug() ;
    is_open = true ;
    log_debug() ;
    for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
      om->request_state(*it, nxt_state) ;
    log_debug("events.empty()=%d", events.empty()) ;
    if (not events.empty())
      om->process_transition_queue() ;
    log_debug() ;
    emit opened() ;
  }

  void concentrating_state::open()
  {
    // Not setting "is_open" to true: it's always closed
    // Not emit any opened() / closed() signals
    log_debug() ;
    for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
      om->request_state(*it, nxt_state) ;
    if (not events.empty())
      om->process_transition_queue() ;
    log_debug() ;
  }

  filter_state::filter_state(const char *name, const char *retry, const char *nxt, machine *m, QObject *p) :
    gate_state(name, retry, m, p),
    next(strdup(nxt))
  {
    QObject::connect(this, SIGNAL(closed()), this, SLOT(emit_close())) ;
  }

  void filter_state::enter(event_t *e)
  {
    if(is_open || ! filter(e))
    {
      om->request_state(e, next) ;
      om->process_transition_queue() ;
    }
    else
      gate_state::enter(e) ;
  }

  void machine::start()
  {
    delete initial_pause ;
    initial_pause = NULL ;
    process_transition_queue() ;
  }

  machine::machine(const Timed *daemon) : owner(daemon)
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
    state *S[] =
    {
      new state_start(this),          // T
        epoch =
      new state_epoch(this),          // T
      new state_new(this),            // T
      new state_scheduler(this),      // T
      new state_qentry(this),         // T
      new state_flt_conn(this),       // IO G F -->NEW
      new state_flt_alrm(this),       // IO G F -->NEW
      new state_flt_user(this),       // IO G F -->NEW
      new state_queued(this),         // IO A   -->NEW

      new state_due(this),            // T

      new state_missed(this),         // T A
//      new state_postponed(this),      // T
      new state_skipped(this),        // T

      new state_armed(this),          // IO G
      new state_triggered(this),      // T A

      new state_dlg_wait(this),       // IO G   -->DUE
      new state_dlg_cntr(this),       // IO C   -->DUE
      new state_dlg_requ(this),       // IO G   -->DUE
      new state_dlg_user(this),       // IO G   -->DUE
      new state_dlg_resp(this),       // T

      /* state_button: below */       // T A

      new state_snoozed(this),        // T
      new state_recurred(this),       // T
      new state_served(this),         // T
      new state_tranquil(this),       // IO A -->DUE

      new state_removed(this),        // T
      new state_aborted(this),        // T
      new state_finalized(this),      // T A
      NULL
    } ;
    log_debug() ;
    for(int i=0; S[i]; ++i)
      states[S[i]->name] = S[i] ;


    log_debug() ;
    for(int i=0; i<=Maemo::Timed::Number_of_Sys_Buttons; ++i)
    {
      state *s = new state_button(this, -i) ;
      states[s->name] = s ;
      button_states[-i] = s ;
    }

    log_debug() ;
    for(int i=1; i<=Maemo::Timed::Max_Number_of_App_Buttons; ++i)
    {
      state *s = new state_button(this, i) ;
      states[s->name] = s ;
      button_states[i] = s ;
    }

    log_debug() ;
    states["TRIGGERED"]->set_action_mask(ActionFlags::State_Triggered) ;
    states["QUEUED"]->set_action_mask(ActionFlags::State_Queued) ;
    states["MISSED"]->set_action_mask(ActionFlags::State_Missed) ;
    states["TRANQUIL"]->set_action_mask(ActionFlags::State_Tranquil) ;
    states["FINALIZED"]->set_action_mask(ActionFlags::State_Finalized) ;
    states["DUE"]->set_action_mask(ActionFlags::State_Due) ;
    states["SNOOZED"]->set_action_mask(ActionFlags::State_Snoozed) ;
    states["SERVED"]->set_action_mask(ActionFlags::State_Served) ;
    states["ABORTED"]->set_action_mask(ActionFlags::State_Aborted) ;
    if(0) states["FAILED"]->set_action_mask(ActionFlags::State_Failed) ;

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

    QObject::connect(dlg_wait, SIGNAL(voland_needed()), this, SIGNAL(voland_needed())) ;

    QObject::connect(dlg_wait, SIGNAL(closed()), dlg_requ, SLOT(open())) ;
    QObject::connect(dlg_wait, SIGNAL(closed()), dlg_user, SLOT(open())) ;
    QObject::connect(dlg_requ, SIGNAL(closed()), dlg_wait, SLOT(open())) ;

    QObject::connect(this, SIGNAL(voland_registered()), dlg_requ, SLOT(close())) ;
    QObject::connect(this, SIGNAL(voland_registered()), dlg_user, SLOT(close())) ;
    QObject::connect(this, SIGNAL(voland_unregistered()), dlg_wait, SLOT(close())) ;
    QObject::connect(this, SIGNAL(voland_unregistered()), dlg_cntr, SLOT(send_back())) ;

    QObject::connect(queued, SIGNAL(sleep()), dlg_cntr, SLOT(open()), Qt::QueuedConnection) ;
    QObject::connect(dlg_wait, SIGNAL(opened()), dlg_cntr, SLOT(open()), Qt::QueuedConnection) ;

    log_debug() ;
    filter_state *flt_conn = dynamic_cast<filter_state*> (states["FLT_CONN"]) ;
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    filter_state *flt_user = dynamic_cast<filter_state*> (states["FLT_USER"]) ;
    log_assert(flt_conn) ;
    log_assert(flt_alrm) ;
    log_assert(flt_user) ;

    QObject::connect(flt_conn, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;
    QObject::connect(flt_alrm, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;
    QObject::connect(flt_user, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;


    log_debug() ;
    QObject::connect(this, SIGNAL(engine_pause(int)), queued, SLOT(engine_pause(int))) ;
    log_debug() ;
    initial_pause = new queue_pause(this) ;
    log_debug() ;

    cluster_queue *c_queue = new cluster_queue(this) ;
    log_debug() ;
    clusters[c_queue->bit] = c_queue ;
    log_debug() ;

    cluster_dialog *c_dialog = new cluster_dialog(this) ;
    log_debug() ;
    clusters[c_dialog->bit] = c_dialog ;
    log_debug() ;
    signalled_bootup = -1 ; // no signal sent yet
    log_debug() ;

    log_debug("owner->settings->alarms_are_enabled=%d", owner->settings->alarms_are_enabled) ;
    log_debug() ;
    alarm_gate(owner->settings->alarms_are_enabled) ;
    log_debug() ;

    transition_start_time = ticker_t(0) ;
    log_debug() ;
    next_cookie = 1 ;
    log_debug() ;
    context_changed = false ;
    log_debug("last line") ;
  }

  void machine::device_mode_detected(bool user_mode)
  {
    filter_state *flt_user = dynamic_cast<filter_state*> (states["FLT_USER"]) ;
    log_assert(flt_user) ;
    if(user_mode)
      flt_user->open() ;
    else
      flt_user->close() ;
    log_info("device_mode_detected: %s", user_mode ? "USER" : "ACT_DEAD") ;
  }

  bool machine::is_epoch_open()
  {
    return epoch->is_open ;
  }

  void machine::open_epoch()
  {
    epoch->open() ;
  }

  string machine::s_states()
  {
    ostringstream os ;
    map<string, set<event_t*> > s2e ;
    for (map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    {
      event_t *e = it->second ;
      string name = e->get_state()==NULL ? "<null>" : e->get_state()->name ;
      s2e[name].insert(e) ;
    }

    for (map<string, set<event_t*> >::const_iterator it=s2e.begin(); it!=s2e.end(); ++it)
      for (set<event_t*>::const_iterator q=it->second.begin(); q!=it->second.end(); ++q)
        os << (q==it->second.begin() ? string(it==s2e.begin()?"":" ")+it->first+":" : ",") << (*q)->cookie.value() ;

    return os.str() ;
  }

  string machine::s_transition_queue()
  {
    ostringstream os ;
    for(deque<pair<event_t*, state*> >::const_iterator it=transition_queue.begin(); it!=transition_queue.end(); ++it)
    {
      cookie_t c = it->first->cookie ;
      state *s = it->second ;
      bool first = it==transition_queue.begin() ;
      os << (first ? "" : ", ") << c.value() << "->" << (s?s->name:"null") ;
    }
    return os.str() ;
  }

  void machine::process_transition_queue()
  {
    // abort() ;
    if(transition_in_progress())
      return ; // never do it recursively
    transition_start_time = ticker_t(now()) ;
    bool queue_changed = false ;
    for(; !transition_queue.empty(); queue_changed = true, transition_queue.pop_front())
    {
      event_t *e = transition_queue.front().first ;
      state *old_state = e->get_state() ;
      state *new_state = transition_queue.front().second ;
      log_assert(new_state!=old_state, "New state is the same as the old one (%s)", old_state->name) ;
#define state_name(p) (p?p->name:"null")
      log_info("State transition %d:'%s'->'%s'", e->cookie.value(), state_name(old_state), state_name(new_state)) ;
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
        log_info("Destroying the element %p", e) ;
        events.erase(e->cookie) ;
        delete e ;
      }
    }
    update_rtc_alarm() ;
    transition_start_time = ticker_t(0) ;
    if(queue_changed)
      emit queue_to_be_saved() ;
    if(context_changed)
      send_queue_context() ;
    send_bootup_signal() ;
  }

  void machine::update_rtc_alarm()
  {
    state_queued *q = dynamic_cast<state_queued*> (states["QUEUED"]) ;
    log_assert(q!=NULL) ;
    time_t tick = q->next_rtc_bootup().value() ;
    if(tick>0) // valid
    {
      tick -= RenameMeNameSpace::Bootup_Length ;
      if(tick < transition_start_time.value())
        tick = -1 ;
    }
    struct rtc_wkalrm rtc ;
    memset(&rtc, 0, sizeof(struct rtc_wkalrm)) ;

    if(tick>0) // still valid
      rtc.enabled = 1 ;
    else
    {
      rtc.enabled = 0 ;
      tick = transition_start_time.value() ; // need a valid timestamp
    }

    log_debug("rtc.enabled=%d", rtc.enabled) ;
    struct tm tm ;
    gmtime_r(&tick, &tm) ;
    memset(&rtc.time, -1, sizeof rtc.time) ;
#define cp(name) rtc.time.tm_##name = tm.tm_##name
    cp(sec), cp(min), cp(hour), cp(mday), cp(mon), cp(year) ;
#undef cp

    int fd = open("/dev/rtc0", O_RDONLY) ;
    if(fd<0)
    {
      log_error("Can't open real time clock: %s", strerror(errno)) ;
      return ;
    }
    int res = ioctl(fd, RTC_WKALM_SET, &rtc) ;
    if(res<0)
      log_error("Can't set real time clock alarm: %s", strerror(errno)) ;
    else
      log_info("Real time clock alarm: %s", !rtc.enabled ? "OFF"
        : str_printf("ON (%04d-%02d-%02d %02d:%02d:%02d)",
          rtc.time.tm_year+1900, rtc.time.tm_mon+1, rtc.time.tm_mday,
          rtc.time.tm_hour, rtc.time.tm_min, rtc.time.tm_sec).c_str()) ;
    close(fd) ;
  }

  ticker_t machine::calculate_bootup()
  {
    cluster_dialog *d = dynamic_cast<cluster_dialog*> (clusters[EventFlags::Cluster_Dialog]) ;
    log_assert(d!=NULL) ;
    if(d->has_bootup_events())
      return ticker_t(1) ; // right now
    state_queued *q = dynamic_cast<state_queued*> (states["QUEUED"]) ;
    log_assert(q!=NULL) ;
    return q->next_bootup() ;
  }

  void machine::send_bootup_signal()
  {
    log_debug() ;
    ticker_t tick = calculate_bootup() ;
    int32_t next_bootup = 0 ;
    if(tick.is_valid())
      next_bootup = tick.value() ;
    log_debug("signalled_bootup=%d, next_bootup=%d", signalled_bootup, next_bootup) ;
    if(signalled_bootup<0 || signalled_bootup!=next_bootup)
      emit next_bootup_event(signalled_bootup=next_bootup) ;
    log_debug() ;
  }

  void machine::invoke_process_transition_queue()
  {
    int methodIndex = this->metaObject()->indexOfMethod("process_transition_queue()");
    QMetaMethod method = this->metaObject()->method(methodIndex);
    method.invoke(this, Qt::QueuedConnection);
  }

  void machine::reshuffle_queue(const nanotime_t &back)
  {
    // log_assert(false, "not implemented") ;
    state *sch = states["SCHEDULER"] ;
    const char *from[] = {"QUEUED","FLT_CONN","FLT_ALRM","FLT_USER",NULL} ;
    for(const char **name=from; *name; ++name)
    {
      io_state *s = dynamic_cast<io_state*> (states[*name]) ;
      log_assert(s) ;
      for(set<event_t*>::iterator it=s->events.begin(); it!=s->events.end(); ++it)
      {
        event_t *e = *it ;
        bool skip = false ;
        if(back.is_zero())
          skip = e->has_ticker() || (e->flags & EventFlags::Snoozing) ;
        if(skip)
          continue ;
        if(e->flags & EventFlags::Empty_Recurring)
          e->invalidate_t() ;
        if(!back.is_zero() && (e->flags & EventFlags::Snoozing))
          e->trigger -= back.to_time_t() ;
        request_state(e, sch) ;
      }
    }
    process_transition_queue() ;
  }

  void machine::request_state(event_t *e, state *st)
  {
    transition_queue.push_back(make_pair(e, st)) ;
    // TODO: I would like to log the current queue state
  }

  void machine::request_state(event_t *e, const char *state_name)
  {
    state *new_state = NULL ;
    if(state_name)
    {
      map<string, state*>::iterator it = states.find(state_name) ;
      log_assert(it!=states.end(), "Unknown state: '%s'", state_name) ;
      new_state = it->second ;
    }
    request_state(e, new_state) ;
  }

  void machine::send_queue_context()
  {
    log_debug() ;
    static ContextProvider::Property alarm_triggers_p("Alarm.Trigger") ;
    log_debug() ;
    static ContextProvider::Property alarm_present_t("Alarm.Present") ;
    log_debug() ;
    cluster_queue *c_queue = dynamic_cast<cluster_queue*> (clusters[EventFlags::Cluster_Queue]) ;
    log_debug() ;
    alarm_triggers_p.setValue(QVariant::fromValue(c_queue->alarm_triggers)) ;
    log_debug() ;
    alarm_present_t.setValue(!c_queue->alarm_triggers.isEmpty()) ;
    log_debug() ;
    context_changed = false ;
    log_debug() ;
  }

  cookie_t machine::add_event(const Maemo::Timed::event_io_t *eio, bool process_queue, const credentials_t *p_creds, const QDBusMessage *p_message)
  {
    // The credentials for the event are either already known (p_creds)
    //   or have to be established by the QDBusMessage structure (from dbus daemon)
    // Using pointers instead of usual C++ references, just because a NULL-reference
    //   usually confuses people (though working just fine)
    if(event_t *e = event_t::from_dbus_iface(eio))
    {
      if(e->actions.size() > 0)
        e->client_creds = p_creds ? *p_creds : credentials_t::from_dbus_connection(*p_message) ;
      request_state(events[e->cookie = cookie_t(next_cookie++)] = e, "START") ;
      if(process_queue)
        invoke_process_transition_queue() ;
      log_info("event cookie=%d", e->cookie.value()) ;
      return e->cookie ;
    }

    return cookie_t(0) ;
  }

  void machine::add_events(const Maemo::Timed::event_list_io_t &lst, QList<QVariant> &res, const QDBusMessage &message)
  {
    // Here we're asking credentials immediately, not like in add_event
    // TODO:
    // But may be it's reasonable first to check, if we really have actions?
    credentials_t creds = credentials_t::from_dbus_connection(message) ;
    bool valid = false ;
    for(int i=0; i<lst.ee.size(); ++i)
    {
      unsigned cookie = add_event(&lst.ee[i], false, &creds, NULL).value() ;
      res.push_back(cookie) ;
      if (cookie)
      {
        valid = true ;
        log_info("event[%d]: accepted, cookie=%d", i, cookie) ;
      }
      else
        log_info("event[%d]: rejected", i) ;
    }
    if(valid)
      invoke_process_transition_queue() ;
  }

#if 0
  void machine::call_returned(QDBusPendingCallWatcher *w)
  {
    if(watcher_to_event.count(w)==0)
    {
      log_critical("unknown QDBusPendingCallWatcher %p", w) ;
      return ;
    }
    event_t *e = watcher_to_event[w] ;
    if(e->dialog_req_watcher!=w)
    {
      log_critical("oops, will terminate in a moment...") ;
      log_debug("w=%p, e=%p, e->cookie=%d", w, e, e->cookie.value()) ;
    }
    log_assert(e->dialog_req_watcher==w) ;
    e->process_dialog_ack() ;
  }
#endif

  void machine::query(const QMap<QString,QVariant> &words, QList<QVariant> &res)
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

  void machine::get_event_attributes(cookie_t c, QMap<QString,QVariant> &a)
  {
    map<cookie_t,event_t*>::iterator it = events.find(c) ;
    if(it==events.end())
      return ;
    event_t *e = it->second ;
    a.insert("STATE", e->st->name) ;
    a.insert("COOKIE", QString("%1").arg(c.value())) ;
    for(attribute_t::const_iterator at=e->attr.txt.begin(); at!=e->attr.txt.end(); at++)
    {
      QString key = string_std_to_q(at->first) ;
      QString val = string_std_to_q(at->second) ;
      a.insert(key,val) ;
    }
  }

  bool machine::dialog_response(cookie_t c, int value)
  {
    if(events.count(c)==0)
    {
      log_error("Invalid cookie [%d] in respond", c.value()) ;
      return false ;
    }
    event_t *e = events[c] ;
    if(e->st != states["DLG_USER"])
    {
      log_error("Unexpected response for event [%d] in state %s", c.value(), e->st->name) ;
      return false ;
    }
    if(value < -Maemo::Timed::Number_of_Sys_Buttons)
    {
      log_error("Invalid negative value in respond: event [%d], value=%d", c.value(), value) ;
      return false ;
    }
    int b = e->b_attr.size() ;
    if(value > b)
    {
      log_error("Invalid value in respond: event [%d], value=%d, number of buttons: %d", c.value(), value, b) ;
      return false ;
    }
    log_assert(button_states.count(value)>0, "value=%d, count=%d", value,button_states.count(value) ) ;
    request_state(e, button_states[value]) ;
    e->flags &= ~ EventFlags::In_Dialog ;
    invoke_process_transition_queue() ;
    return true ;
  }

  event_t *machine::find_event(cookie_t c)
  {
    log_assert(!transition_in_progress()) ;
    process_transition_queue() ;

    map<cookie_t,event_t*>::iterator it = events.find(c) ;
    return it==events.end() ? NULL : it->second ;
  }

  bool machine::cancel_by_cookie(cookie_t c) // XXX need some clean up here?
  {
    queue_pause x(this) ;

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

  void machine::cancel_event(event_t *e)
  {
    // TODO: assert (queue is paused)
    io_state *s = dynamic_cast<io_state*> (e->st) ;
    log_assert(s) ;
    s->abort(e) ;
  }

  void machine::alarm_gate(bool value)
  {
    filter_state *filter = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    log_assert(filter) ;
    value ? filter->open() : filter->close() ;
  }

#if 0
  void event_t::process_dialog_ack() // should be move to state_dlg_requ?
  {
    QDBusPendingReply<bool> reply = *dialog_req_watcher ;
    bool ok = reply.isValid() && reply.value() ;
    if(ok)
    {
      st->om->request_state(this, "DLG_USER") ;
      st->om->process_transition_queue() ;
    }
    else
      log_error("Requesting event %d dialog failed: %s", cookie.value(), reply.error().message().toStdString().c_str()) ;
    delete dialog_req_watcher ;
    dialog_req_watcher = NULL ;
  }
#endif

  uint32_t machine::attr(uint32_t mask)
  {
    return flags & mask ;
  }

  iodata::record *machine::save(bool for_backup)
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
#if 0
    r->add("default_snooze", default_snooze_value) ;
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    r->add("alarms", flt_alrm->is_open ? 1 : 0) ;
#endif

    return r ;
  }

  void machine::load(const iodata::record *r)
  {
    next_cookie = r->get("next_cookie")->value() ;
#if 0
    default_snooze_value = r->get("default_snooze")->value() ;
#endif
    const iodata::array *a = r->get("events")->arr() ;
#if 0
    for(unsigned i=0; i < a->size(); ++i)
    {
      const iodata::record *ee = a->get(i)->rec() ;
      cookie_t c(ee->get("cookie")->value()) ;
      event_t *e = new event_t ;
      events[e->cookie = c] = e ;

      e->ticker = ticker_t(ee->get("ticker")->value()) ;
      e->t.load(ee->get("t")->rec()) ;

      e->tz = ee->get("tz")->str() ;

      e->attr.load(ee->get("attr")->rec()) ;
      e->flags = ee->get("flags")->decode(event_t::codec) ;
      iodata::load(e->recrs, ee->get("recrs")->arr()) ;
      iodata::load(e->actions, ee->get("actions")->arr()) ;
      iodata::load_int_array(e->snooze, ee->get("snooze")->arr()) ;
      iodata::load(e->b_attr, ee->get("b_attr")->arr()) ;
      e->last_triggered = ticker_t(ee->get("dialog_time")->value()) ;
      e->tsz_max = ee->get("tsz_max")->value() ;
      e->tsz_counter = ee->get("tsz_counter")->value() ;
      e->client_creds.load(ee->get("client_creds")->rec()) ;
      e->cred_modifier.load(ee->get("cred_modifier")->arr()) ;

      const char *next_state = "START" ;

      if(e->flags & EventFlags::Empty_Recurring)
        e->invalidate_t() ;

      request_state(e, next_state) ;
    }
#else
    load_events(a, true, true) ;
#endif

#if 0
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    if(r->get("alarms")->value())
      flt_alrm->open() ;
    else
      flt_alrm->close() ;
#else
    // alarm_gate(true, r->get("alarms")->value()) ;
#endif
  }

  void machine::load_events(const iodata::array *events_data, bool trusted_source, bool use_cookies)
  {
    for(unsigned i=0; i < events_data->size(); ++i)
    {
      const iodata::record *ee = events_data->get(i)->rec() ;
      unsigned cookie = use_cookies ? ee->get("cookie")->value() : next_cookie++ ;
      event_t *e = new event_t ;
      events[e->cookie = cookie_t(cookie)] = e ;

      e->ticker = ticker_t(ee->get("ticker")->value()) ;
      e->t.load(ee->get("t")->rec()) ;

      e->tz = ee->get("tz")->str() ;

      e->attr.load(ee->get("attr")->rec()) ;
      e->flags = ee->get("flags")->decode(event_t::codec) ;
      iodata::load(e->recrs, ee->get("recrs")->arr()) ;
      iodata::load_int_array(e->snooze, ee->get("snooze")->arr()) ;
      iodata::load(e->b_attr, ee->get("b_attr")->arr()) ;
      e->last_triggered = ticker_t(ee->get("dialog_time")->value()) ;
      e->tsz_max = ee->get("tsz_max")->value() ;
      e->tsz_counter = ee->get("tsz_counter")->value() ;
      if(trusted_source)
      {
        iodata::load(e->actions, ee->get("actions")->arr()) ;
        e->client_creds.load(ee->get("client_creds")->rec()) ;
        e->cred_modifier.load(ee->get("cred_modifier")->arr()) ;
      }

      const char *next_state = "START" ;

      if(e->flags & EventFlags::Empty_Recurring)
        e->invalidate_t() ;

      request_state(e, next_state) ;
    }
  }

  void machine::cancel_backup_events()
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

#if 0
  int machine::default_snooze(int new_value)
  {
    if(30 <= new_value) // TODO: make it configurierable?
    {
      default_snooze_value = new_value ;
      emit queue_to_be_saved() ;
    }
    return default_snooze_value ;
  }
#endif

  // TODO: do it accessible from outside of this file:
  //       too many uncaught exceptions :)
  struct event_exception : public std::exception
  {
    string message ;
    pid_t pid_value ;

    pid_t pid() const { return pid_value ; }
    event_exception(const string &msg) : message(msg), pid_value(getpid()) { }
   ~event_exception() throw() { }
  } ;

  // what a mess... event_t::stuff should be in event.cpp

  void event_t::sort_and_run_actions(uint32_t mask)
  {
    if (mask==0) // TODO: check, if this check is not yet done on state_* layer...
      return ;

    // don't want to call unnecesary constructor here
    vector <unsigned> *r = NULL ;

    for (unsigned i=0; i<actions.size(); ++i)
      if(actions[i].flags & mask) // something to run
      {
        if (r==NULL)
          r = new vector <unsigned> ;
        r->push_back(i) ;
      }

    if (r==NULL) // nothing to do
      return ;

    log_assert(r) ;

    log_info("Beginning the action list dump") ;
    for(vector<unsigned>::const_iterator it=r->begin(); it!=r->end(); ++it)
      log_info("Action %d, cred_key: '%s'", *it, actions[*it].cred_key().c_str()) ;
    log_info("Done:     the action list dump") ;

    log_info("Beginning the action array sorting") ;
    sort(r->begin(), r->end(), action_comparison_t(this)) ;
    log_info("Done:     the action array sorting") ;

    unsigned start_index = ~0 ; // invalid value
    string current_key ; // empty key (invalid?)
    for(unsigned i=0, have_exec=false; i < r->size(); ++i)
    {
      action_t &a = actions[(*r) [i]] ;
      bool i_exec = a.flags & ActionFlags::Run_Command ; // current action is an exec action
      bool start_now = a.cred_key() != current_key || (have_exec && i_exec) ; // either new key or second exec
      if (start_now && i>0)
        run_actions(*r, start_index, i) ;
      if (start_now || i==0) // i==0 is probably redundant, as cred_key should never be empty
      {
        have_exec = i_exec ;
        start_index = i ;
        current_key = a.cred_key() ;
      }
    }

    log_assert(r->size()>0) ; // to be sure for the run_actions() call below
    run_actions(*r, start_index, r->size()) ;

    delete r ;
  }

  template <class element_t>
  string print_vector(const vector<element_t> &array, unsigned begin, unsigned end)
  {
    ostringstream os ;

    for(unsigned i=begin, first=true; i<end; ++i)
    {
      os << ( first ? first=false, "[" : ", " ) ;
      os << array[i] ;
    }
    os << "]" ;
    return os.str() ;
  }

  template <class element_t> // an yet another hihgly inefficient function :-(
  void set_change(set<element_t> &x, const set<element_t> &y, bool add)
  {
    for(typename set<element_t>::const_iterator it=y.begin(); it!=y.end(); ++it)
      if(add)
        x.insert(*it) ;
      else
        x.erase(*it) ;
  }

  void event_t::run_actions(const vector<unsigned> &acts, unsigned begin, unsigned end)
  {
    log_debug("begin=%d, end=%d, actions: %s", begin, end, print_vector(acts, begin, end).c_str()) ;
    const action_t &a_begin = actions [acts[begin]] ;

    // we want to detect, it the exeption was thrown in the daemon itself or in a child
    pid_t daemon = getpid() ;

    try
    {
      if (fork_and_set_credentials_v3(a_begin)>0) // parent
        return ;
    }
    catch(const event_exception &e)
    {
      log_error("action list %d%s failed: %s", cookie.value(), print_vector(acts, begin, end).c_str(), e.message.c_str()) ;
      pid_t process = e.pid() ;
      if (process!=daemon)
      {
        log_info("terminating child (pid=%d) of daemon (pid=%d)", process, daemon) ;
        ::exit(1) ;
      }
    }

    // now we're running in the child with proper credentials, so let's execute dbus stuff !

    unsigned exec_action_i = ~0 ; // not found yet
    QDBusConnection *conn[2] = {NULL, NULL} ; // 0->system, 1->session
    const QString cname = "timed-private" ;

    int dbus_fail_counter = 0 ;

    for (unsigned i=begin; i<end; ++i)
    {
      const unsigned acts_i = acts[i] ;
      const action_t &a = actions [acts_i] ;

      if (a.flags & ActionFlags::Run_Command)
        exec_action_i = acts_i ;

      if((a.flags & ActionFlags::DBus_Action) == 0)
        continue ;

      try
      {
        // set up message to be sent
        QString path = string_std_to_q(find_action_attribute("DBUS_PATH", a)) ;
        QString ifac = string_std_to_q(find_action_attribute("DBUS_INTERFACE", a, (a.flags & ActionFlags::DBus_Signal)!=0)) ;

        QDBusMessage message ;

        if (a.flags & ActionFlags::DBus_Method)
        {
          QString serv = string_std_to_q(find_action_attribute("DBUS_SERVICE", a)) ;
          QString meth = string_std_to_q(find_action_attribute("DBUS_METHOD", a)) ;
          message = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
        }
        else
        {
          QString sgnl = string_std_to_q(find_action_attribute("DBUS_SIGNAL", a)) ;
          message = QDBusMessage::createSignal(path, ifac, sgnl) ;
        }

        QMap<QString,QVariant> param ;

        if (a.flags & ActionFlags::Send_Cookie)
          param["COOKIE"] = QString("%1").arg(cookie.value()) ;

        if (a.flags & ActionFlags::Send_Event_Attributes)
          add_strings(param, attr.txt) ;

        if (a.flags & ActionFlags::Send_Action_Attributes)
          add_strings(param, a.attr.txt) ;

        message << QVariant::fromValue(param) ;

        int bus = (a.flags & ActionFlags::Use_System_Bus) ? 0 : 1 ;
        QDBusConnection * &c = conn[bus] ;

        if (c==NULL) // not used yes, have to create object and connect
        {
          QDBusConnection::BusType ctype = bus==0 ? QDBusConnection::SystemBus : QDBusConnection::SessionBus ;
          c = new QDBusConnection(QDBusConnection::connectToBus(ctype, cname)) ;
        }

        if (c->send(message))
          log_debug("%u[%d]: D-Bus Message sent", cookie.value(), acts_i) ;
        else
          throw event_exception(c->lastError().message().toStdString().c_str()) ;
      }
      catch(const event_exception &e)
      {
        log_error("%u[%d]: dbus-action not executed: %s", cookie.value(), acts_i, e.message.c_str()) ;
        ++ dbus_fail_counter ;
      }
    }

    for(int c=0; c<2; ++c)
      if(QDBusConnection *cc = conn[c])
      {
        // as we are about to exit immediately after queuing
        // and there seems to be no way to flush the connection
        // and be sure that we have actually transmitted the
        // message -> do a dummy synchronous query from dbus
        // daemon and hope that is enough to get the actual
        // message to be delivered ...
        QString connection_name  = cc->baseService() ;
        unsigned owner = get_name_owner_from_dbus_sync(*cc, connection_name) ;

        // it should be us (either pid or uid dependin on feature set) ...
        log_debug("pid=%d, ruid=%d, euid=%d, connection owner is '%u'", getpid(), getuid(), geteuid(), owner) ;
        QDBusConnection::disconnectFromBus(connection_name) ;
        delete cc ;
      }

    if(dbus_fail_counter)
      log_warning("failed to exeute %d out of %u dbus actions", dbus_fail_counter, end-begin) ;

    if(exec_action_i == ~0u) // no exec action in this action package
      ::exit(dbus_fail_counter<100 ? dbus_fail_counter : 100) ; // exit value can't be toooo large

    // All the dbus actions are done. Now we have to execute the only command line action

    const action_t &a = actions [exec_action_i] ;

    string cmd = find_action_attribute("COMMAND", a) ;
    if(a.flags & ActionFlags::Send_Cookie)
    {
      log_debug("cmd='%s', COOKIE to be replaced by value", cmd.c_str()) ;
      using namespace pcrecpp ;
      static RE exp("(<COOKIE>)|\\b(COOKIE)\\b", UTF8()) ;
      ostringstream decimal ;
      decimal << cookie.value() ;
      exp.GlobalReplace(decimal.str(), &cmd);
      log_debug("cmd='%s', COOKIE replaced", cmd.c_str()) ;
    }
    log_info("execuing command line action %u[%d]: '%s'", cookie.value(), exec_action_i, cmd.c_str());
    execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL) ;
    log_error("execl(/bin/sh -c '%s') failed: %m", cmd.c_str());

    ::exit(101) ; // use dbus_fail_counter here as well?
  }

  bool event_t::accrue_privileges(const action_t &a)
  {
    credentials_t creds = credentials_t::from_current_process() ;

#if F_TOKENS_AS_CREDENTIALS
    const cred_modifier_t &E = cred_modifier, &A = a.cred_modifier ;

    // tokens_to_accrue1 := EVENT.add - ACTION.drop
    set<string> tokens_to_accrue1 = E.tokens_by_value(true) ;
    set_change<string> (tokens_to_accrue1, A.tokens_by_value(false), false) ;

    // tokens_to_accrue2 := ACTION.add - EVENT.drop
    set<string> tokens_to_accrue2 = A.tokens_by_value(true) ;
    set_change<string> (tokens_to_accrue2, E.tokens_by_value(false), false) ;

    // creds += (tokens_to_accrue 1+2)
    set_change<string> (creds.tokens, tokens_to_accrue1, true) ;
    set_change<string> (creds.tokens, tokens_to_accrue2, true) ;
#endif // F_TOKENS_AS_CREDENTIALS

    string uid = find_action_attribute("USER", a, false) ;
    string gid = find_action_attribute("GROUP", a, false) ;

    if (!uid.empty())
      creds.uid = uid ;
    if (!gid.empty())
      creds.gid = gid ;

    return creds.apply_and_compare() ;
  }

  bool event_t::drop_privileges(const action_t &a)
  {
    credentials_t creds = client_creds ;

#if F_TOKENS_AS_CREDENTIALS
    const cred_modifier_t &E = cred_modifier, &A = a.cred_modifier ;

    // tokens_to_remove1 := EVENT.drop - ACTION.add
    set<string> tokens_to_remove1 = E.tokens_by_value(false) ;
    set_change<string> (tokens_to_remove1, A.tokens_by_value(true), false) ;

    // tokens_to_remove2 := ACTION.drop - EVENT.add
    set<string> tokens_to_remove2 = A.tokens_by_value(false) ;
    set_change<string> (tokens_to_remove2, E.tokens_by_value(true), false) ;

    // creds := client_creds - (tokens_to_remove 1+2)
    set_change<string> (creds.tokens, tokens_to_remove1, false) ;
    set_change<string> (creds.tokens, tokens_to_remove2, false) ;
#endif // F_TOKENS_AS_CREDENTIALS

    return creds.apply_and_compare() ;
  }

  string event_t::find_action_attribute(const string &key, const action_t &a, bool exc)
  {
    string value = a.attr(key) ;
    if(value.empty())
      value = attr(key) ;
    if(value.empty() && exc)
      throw event_exception(string("empty attribute '")+key+"'") ;
    return value ;
  }

  void event_t::add_strings(QMap<QString, QVariant> &x, const map<string,string> &y)
  {
    for(map<string,string>::const_iterator it=y.begin(); it!=y.end(); ++it)
      x.insert(string_std_to_q(it->first), QVariant::fromValue(string_std_to_q(it->second))) ;
  }

  pid_t event_t::fork_and_set_credentials_v3(const action_t &action)
  {
    pid_t pid = fork() ;

    if (pid<0) // can't fork
    {
      log_error("fork() failed: %m") ;
      throw event_exception("can't fork");
    }
    else if (pid>0) // parent
    {
      log_info("forked successfully, child pid: '%d'", pid) ;
      st->om->emit_child_created(cookie.value(), pid) ;
      return pid ;
    }
    else // child
    {
      log_info("event [%u]: in child process", cookie.value()) ;

      if (setsid() < 0) // detach from session
      {
        log_error("setsid() failed: %m") ;
        throw event_exception("can't detach from session") ;
      }

      if (!drop_privileges(action))
        throw event_exception("can't drop privileges") ;

      if (!accrue_privileges(action))
        log_warning("can't accrue privileges, still continuing") ;

      // Skipping all the uid/gid settings, because it's part of
      // credentials_t::apply() (or it should be at least)

      // TODO: go to home dir _here_ !

      // That's it then, isn't it?

      return pid ;
    }
  }

  iodata::record *event_t::save(bool for_backup)
  {
    iodata::record *r = new iodata::record ;
    r->add("cookie", cookie.value()) ;
    r->add("ticker", ticker.value()) ;
    r->add("t", t.save()) ;
    r->add("tz", new iodata::bytes(tz)) ;
    r->add("attr", attr.save() ) ;
    r->add("flags", new iodata::bitmask(flags &~ EventFlags::Cluster_Mask, codec)) ;
    r->add("recrs", iodata::save(recrs)) ;
    r->add("snooze", iodata::save_int_array(snooze)) ;
    r->add("b_attr", iodata::save(b_attr)) ;

    r->add("dialog_time", (flags & EventFlags::In_Dialog) ? last_triggered.value() : 0) ;
    r->add("tsz_max", tsz_max) ;
    r->add("tsz_counter", tsz_counter) ;

    if(not for_backup)
      r->add("actions", iodata::save(actions)) ;
    if(not for_backup)
      r->add("client_creds", client_creds.save()) ;
    if(not for_backup)
      r->add("cred_modifier", cred_modifier.save()) ;
    return r ;
  }

  bool event_t::compute_recurrence()
  {
    log_assert(false, "obsolete function") ;
    return false ;
  }

request_watcher_t::request_watcher_t(machine *om)
{
  this->om = om ;
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

#if 0
  if (reply.isValid() && reply.value())
#else
  log_info("reminder servide replied: %d,%d", reply.isValid(), reply.isValid()&&reply.value()) ;
  if (true) // XXX: remove this after reminders returns TRUE here.
#endif
  {
    for(set<event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
      om->request_state(*it, "DLG_USER") ;

    if (events.size()>0)
      om->process_transition_queue() ;
  }
  else
    log_error("DBus call voland::create failed events not listed here...") ;


  // Don't need the watcher any more

  delete this ;
}
