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
#include <pwd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <map>
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

#include "timed/event-io.h"

#include "log.h"

#include "automata.h"
#include "states.h"
#include "flags.h"
#include "event.h"
#include "misc.h"
#include "timed/nanotime.h"
#include "credentials.h"

static void log_child(const char *fmt, ...)
{
  // FIXME: remove this & replace with log_(error|warning|xxx)
  // helper for making child logging more visible
  va_list va;
  va_start(va, fmt);
  char *s = 0;
  vasprintf(&s, fmt, va);
  va_end(va);
  if( s != 0 ) dprintf(2, "%s\n", s), free(s);
}

#if 0
namespace Alarm
{
#endif
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
#if 0
    log_debug() ;
#endif
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
    is_open = true ;
    for(set<event_t*>::iterator it=events.begin(); it!=events.end(); ++it)
      om->request_state(*it, nxt_state) ;
    om->process_transition_queue() ;
    emit opened() ;
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

  machine::machine(QObject *parent) : QObject(parent)
  {
    // T = transition state
    // IO = waiting for i/o state
    // G = gate state
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
#if 0
      new state_postponed(this),      // T
#endif
      new state_skipped(this),        // T

      new state_triggered(this),      // T A

      new state_dlg_wait(this),       // IO G   -->DUE
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
    for(int i=0; S[i]; ++i)
      states[S[i]->name] = S[i] ;

    for(int i=0; i<=Maemo::Timed::Number_of_Sys_Buttons; ++i)
    {
      state *s = new state_button(this, -i) ;
      states[s->name] = s ;
      button_states[-i] = s ;
    }

    for(int i=1; i<=Maemo::Timed::Max_Number_of_App_Buttons; ++i)
    {
      state *s = new state_button(this, i) ;
      states[s->name] = s ;
      button_states[i] = s ;
    }

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

    io_state *queued = dynamic_cast<io_state*> (states["QUEUED"]) ;
    log_assert(queued!=NULL) ;

    gate_state *dlg_wait = dynamic_cast<gate_state*> (states["DLG_WAIT"]) ;
    gate_state *dlg_requ = dynamic_cast<gate_state*> (states["DLG_REQU"]) ;
    gate_state *dlg_user = dynamic_cast<gate_state*> (states["DLG_USER"]) ;
    log_assert(dlg_wait!=NULL) ;
    log_assert(dlg_requ!=NULL) ;
    log_assert(dlg_user!=NULL) ;

    QObject::connect(dlg_wait, SIGNAL(voland_needed()), this, SIGNAL(voland_needed())) ;

    QObject::connect(dlg_wait, SIGNAL(closed()), dlg_requ, SLOT(open())) ;
    QObject::connect(dlg_wait, SIGNAL(closed()), dlg_user, SLOT(open())) ;
    QObject::connect(dlg_requ, SIGNAL(closed()), dlg_wait, SLOT(open())) ;

    QObject::connect(this, SIGNAL(voland_registered()), dlg_requ, SLOT(close())) ;
    QObject::connect(this, SIGNAL(voland_registered()), dlg_user, SLOT(close())) ;
    QObject::connect(this, SIGNAL(voland_unregistered()), dlg_wait, SLOT(close())) ;

    filter_state *flt_conn = dynamic_cast<filter_state*> (states["FLT_CONN"]) ;
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    filter_state *flt_user = dynamic_cast<filter_state*> (states["FLT_USER"]) ;
    log_assert(flt_conn) ;
    log_assert(flt_alrm) ;
    log_assert(flt_user) ;

    QObject::connect(flt_conn, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;
    QObject::connect(flt_alrm, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;
    QObject::connect(flt_user, SIGNAL(closed(filter_state*)), queued, SLOT(filter_closed(filter_state*))) ;

    QObject::connect(this, SIGNAL(engine_pause(int)), queued, SLOT(engine_pause(int))) ;

    cluster_queue *c_queue = new cluster_queue(this) ;
    clusters[c_queue->bit] = c_queue ;

    cluster_dialog *c_dialog = new cluster_dialog(this) ;
    clusters[c_dialog->bit] = c_dialog ;
    signalled_bootup = -1 ; // no signal sent yet

    transition_start_time = ticker_t(0) ;
    next_cookie = 1 ;
    default_snooze_value = 300 ;
    context_changed = false ;
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

  void machine::process_transition_queue()
  {
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
#if 0
      log_debug("e=%p", e) ;
      log_debug("cookie=%d", e->cookie.value()) ;
      log_debug("From '%p'", old_state) ;
      log_debug("To '%p'", new_state) ;
      log_debug("From '%s'", state_name(old_state)) ;
      log_debug("To '%s'", state_name(new_state)) ;
#endif
      log_info("State transition %d:'%s'->'%s'", e->cookie.value(), state_name(old_state), state_name(new_state)) ;
#undef state_name
      if(old_state)
        old_state->leave(e) ;
      e->set_state(new_state) ;
      if(new_state)
      {
        new_state->enter(e) ;
        e->run_actions(new_state->get_action_mask()) ;
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
#if 0
    ostringstream os ;
    typedef deque <pair <event_t*, state*> >::const_iterator iterator ;
    bool start = true ;
    for(iterator it=transition_queue.begin(); it!=transition_queue.end(); ++it, start=false)
    {
      cookie_t cookie=it->first->cookie ;
      state *current = it->first->st, *requested = it->second ;
      // const char *current = it->first->st->name ;
      // const char *requested = it->second->name ;
      if(!start)
        os << ", " ;
#define state_name(p) (p?p->name:"null")
      os << "[" << cookie.value() << "," << state_name(current) << "]->" << state_name(requested) ;
#undef state_name
    }
    log_debug("request queue: %s", os.str().c_str()) ;
#endif
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
    static ContextProvider::Property alarm_present_t("Alarm.Present") ;
    cluster_queue *c_queue = dynamic_cast<cluster_queue*> (clusters[EventFlags::Cluster_Queue]) ;
    alarm_triggers_p.setValue(QVariant::fromValue(c_queue->alarm_triggers)) ;
    alarm_present_t.setValue(!c_queue->alarm_triggers.isEmpty()) ;
#if 0
    static ContextProvider::Property x239("UserAlarm.NextCookie") ;
    x239.setValue(239) ;
#endif
    context_changed = false ;
    log_debug() ;
  }

  cookie_t machine::add_event(const Maemo::Timed::event_io_t *eio, bool process_queue, const QString &credentials)
  {
    if(event_t *e = event_t::from_dbus_iface(eio))
    {
      e->attr.txt["CREDENTIALS"] = string(credentials.toUtf8().constData());
      request_state(events[e->cookie = cookie_t(next_cookie++)] = e, "START") ;
      if(process_queue)
        invoke_process_transition_queue() ;
      log_info("event cookie=%d", e->cookie.value()) ;
      return e->cookie ;
    }

    return cookie_t(0) ;
  }

  void machine::add_events(const Maemo::Timed::event_list_io_t &lst, QList<QVariant> &res, const QString &credentials)
  {
    bool valid = false ;
    for(int i=0; i<lst.ee.size(); ++i)
    {
      unsigned cookie = add_event(&lst.ee[i], false, credentials).value() ;
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

  bool machine::cancel(cookie_t c) // XXX need some clean up here?
  {
    queue_pause x(this) ;

    if(event_t *e = find_event(c))
    {
      io_state *s = dynamic_cast<io_state*> (e->st) ;
      log_assert(s) ;
      s->abort(e) ;
      return true ;
    }
    else
    {
      log_error("[%d]: cookie not found", c.value()) ;
      return false ;
    }
  }

  bool machine::alarm_gate(bool set, bool value)
  {
    filter_state *filter = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    log_assert(filter) ;
    if(set)
      value ? filter->open() : filter->close() ;
    return filter->is_open ;
  }

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

#if 0
  event_t::event_t(const event_io_t *eio)
  : ticker(eio->ticker),
    t(eio->t_year, eio->t_month, eio->t_day, eio->t_hour, eio->t_minute),
    attr(eio->attr),
    dialog(eio->dialog),
    actions(eio->actions),
    recrs(eio->recrs),
    trigger(0),
    last_triggered(0),
    to_be_snoozed(0)
  {
    tz = eio->t_zone.isEmpty() ? NULL : strdup(eio->t_zone.toStdString().c_str()) ;
    st = NULL ;
    dialog_req_watcher = NULL ;
  }
#endif

#if 0
  event_t::~event_t()
  {
    if(tz)
      free((void*)tz) ;
  }
#endif

#if 0
  event_t::event_t(const iodata::record *r)
  {
  }
#endif

  uint32_t machine::attr(uint32_t mask)
  {
    return flags & mask ;
  }

  iodata::record *machine::save()
  {
    // log_debug() ;
    iodata::record *r = new iodata::record ;
    iodata::array *q = new iodata::array ;

    // log_debug() ;
    for(map<cookie_t, event_t*>::const_iterator it=events.begin(); it!=events.end(); ++it)
    {
      // log_debug() ;
      q->add(it->second->save()) ;
      // log_debug() ;
    }

    // log_debug() ;
    r->add("events", q) ;
    // log_debug() ;
    r->add("next_cookie", next_cookie) ;
    r->add("default_snooze", default_snooze_value) ;
    // log_debug() ;
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    r->add("alarms", flt_alrm->is_open ? 1 : 0) ;

    return r ;
  }

  void machine::load(const iodata::record *r)
  {
    next_cookie = r->get("next_cookie")->value() ;
    default_snooze_value = r->get("default_snooze")->value() ;
    const iodata::array *a = r->get("events")->arr() ;
    for(unsigned i=0; i < a->size(); ++i)
    {
      const iodata::record *ee = a->get(i)->rec() ;
      cookie_t c(ee->get("cookie")->value()) ;
      event_t *e = new event_t ;
      events[e->cookie = c] = e ;
      // e->trigger = (ticker_t) ee->get("trigger")->value() ;
      // e->snoozed_trigger = (ticker_t) ee->get("snoozed_trigger")->value() ;

      e->ticker = ticker_t(ee->get("ticker")->value()) ;
#if 0
      e->t_year = ee->get("t_year")->value() ;
      e->t_month = ee->get("t_month")->value() ;
      e->t_day = ee->get("t_day")->value() ;
      e->t_hour = ee->get("t_hour")->value() ;
      e->t_minute = ee->get("t_minute")->value() ;
#else
      e->t.load(ee->get("t")->rec()) ;
#endif

#if 0
      string tz_s = ee->get("tz")->str() ;
      e->tz = tz_s.length() > 0 ? strdup(tz_s.c_str()) : NULL ;
#else
      e->tz = ee->get("tz")->str() ;
#endif
      e->attr.load(ee->get("attr")->rec()) ;
      e->flags = ee->get("flags")->decode(event_t::codec) ;
      iodata::load(e->recrs, ee->get("recrs")->arr()) ;
      iodata::load(e->actions, ee->get("actions")->arr()) ;
      iodata::load_int_array(e->snooze, ee->get("snooze")->arr()) ;
      iodata::load(e->b_attr, ee->get("b_attr")->arr()) ;
      e->last_triggered = ticker_t(ee->get("dialog_time")->value()) ;
      e->tsz_max = ee->get("tsz_max")->value() ;
      e->tsz_counter = ee->get("tsz_counter")->value() ;
#if 0
      e->dialog.load(ee->get("dialog")->rec()) ;
#endif

#if 0
      e->recr_count = ee->get("recr_count")->value() ;
#endif

#if 0
      log_debug("e=%p", e) ;
      log_debug("cookie=%d", e->cookie.value()) ;
      log_debug("From '%p'", e->get_state()) ;
#endif

#if 0
      request_state(e, ee->get("due_state")->value() ? "DUE" : "NEW") ;
#else
      const char *next_state = "START" ;

      if(e->flags & EventFlags::Empty_Recurring)
        e->invalidate_t() ;

      request_state(e, next_state) ;
#endif
    }
    filter_state *flt_alrm = dynamic_cast<filter_state*> (states["FLT_ALRM"]) ;
    if(r->get("alarms")->value())
      flt_alrm->open() ;
    else
      flt_alrm->close() ;
  }

  int machine::default_snooze(int new_value)
  {
    if(30 <= new_value) // TODO: make it configurierable?
    {
      default_snooze_value = new_value ;
      emit queue_to_be_saved() ;
    }
    return default_snooze_value ;
  }

  struct event_exception : public std::exception
  {
    string message ;
    event_exception(const string &msg) : message(msg) { }
   ~event_exception() throw() { }
  } ;

  void event_t::run_actions(uint32_t mask)
  {
    // log_debug("event=%d, actions.size()=%d", cookie.value(), actions.size()) ;
    for(unsigned i=0; i<actions.size(); ++i)
    {
      const action_t &a = actions[i] ;
      if((a.flags & mask)==0)
        continue ;
      log_info("executing action %d[%d]", cookie.value(), i) ;
      try
      {
        if(a.flags & (ActionFlags::DBus_Method | ActionFlags::DBus_Signal))
          execute_dbus(a) ;
        if(a.flags & ActionFlags::Run_Command)
          execute_command(a) ;
      }
      catch(const event_exception &e)
      {
        log_error("action %d[%d] failed: %s", cookie.value(), i, e.message.c_str()) ;
      }
    }
  }

  string event_t::find_action_attribute(const string &key, const action_t &a, bool exc)
  {
#if 0
    QString action_value = a.attr(key) ;
    return action_value.isEmpty() ? attr(key) : action_value ;
#else
    string value = a.attr(key) ;
    if(value.empty())
      value = attr(key) ;
    if(value.empty() && exc)
      throw event_exception(string("empty attribute '")+key+"'") ;
    return value ;
#endif
  }

  void event_t::add_strings(QMap<QString, QVariant> &x, const map<string,string> &y)
  {
    for(map<string,string>::const_iterator it=y.begin(); it!=y.end(); ++it)
      x.insert(string_std_to_q(it->first), QVariant::fromValue(string_std_to_q(it->second))) ;
  }

#if 0
  void event_t::execute_dbus(const action_t &a)
  {
    QString path = string_std_to_q(find_action_attribute("DBUS_PATH", a)) ;
    QString ifac = string_std_to_q(find_action_attribute("DBUS_INTERFACE", a, (a.flags & ActionFlags::DBus_Signal)!=0)) ;
    QDBusMessage m ;
    if(a.flags & ActionFlags::DBus_Method)
    {
      QString serv = string_std_to_q(find_action_attribute("DBUS_SERVICE", a)) ;
      QString meth = string_std_to_q(find_action_attribute("DBUS_METHOD", a)) ;
      m = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
    }
    else
    {
      QString sgnl = string_std_to_q(find_action_attribute("DBUS_SIGNAL", a)) ;
      m = QDBusMessage::createSignal(path, ifac, sgnl) ;
    }
    // QDBusConnection QDBusConnection::connectToBus ( BusType type, const QString & name);
    // void QDBusConnection::disconnectFromBus ( const QString & name )
    // BusType ctype = QDBusConnection::SessionBus;
    // if( a.flags & ActionFlags::Use_System_Bus ) ctype = QDBusConnection::SystemBus;
    // QString cname = "timed-private";
    // QDBusConnection c = QDBusConnection::connectToBus(ctype, cname);
    // c.send(m);
    // QDBusConnection::disconnectFromBus(cname);

    QDBusConnection c = (a.flags & ActionFlags::Use_System_Bus) ? QDBusConnection::systemBus() : QDBusConnection::sessionBus() ;
    QMap<QString,QVariant> param ;
    if(a.flags & ActionFlags::Send_Cookie)
      param["COOKIE"] = QString("%1").arg(cookie.value()) ;
    if(a.flags & ActionFlags::Send_Event_Attributes)
      add_strings(param, attr.txt) ;
    if(a.flags & ActionFlags::Send_Action_Attributes)
      add_strings(param, a.attr.txt) ;
    m << QVariant::fromValue(param) ;
    if(c.send(m))
      log_debug("[%d]: D-Bus Message sent", cookie.value()) ;
    else
      log_error("[%d]: Failed to send a message on D-Bus: %s", cookie.value(), c.lastError().message().toStdString().c_str()) ;
  }
#else
  void event_t::execute_dbus(const action_t &a)
  {
    bool error = true;
    int  child = fork_and_set_credentials(a, error);

    if( child != 0 )
    {
      // parent
    }
    else if( error )
    {
      // child init failed, just terminate
      // cause of error logged @ fork_and_set_credentials()
      _exit(1);
    }
    else
    {
      // child init ok

      // set up message to be sent
      QString path = string_std_to_q(find_action_attribute("DBUS_PATH", a)) ;
      QString ifac = string_std_to_q(find_action_attribute("DBUS_INTERFACE", a, (a.flags & ActionFlags::DBus_Signal)!=0)) ;

      QDBusMessage m ;

      if(a.flags & ActionFlags::DBus_Method)
      {
	QString serv = string_std_to_q(find_action_attribute("DBUS_SERVICE", a)) ;
	QString meth = string_std_to_q(find_action_attribute("DBUS_METHOD", a)) ;
	m = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
      }
      else
      {
	QString sgnl = string_std_to_q(find_action_attribute("DBUS_SIGNAL", a)) ;
	m = QDBusMessage::createSignal(path, ifac, sgnl) ;
      }

      QMap<QString,QVariant> param ;
      if(a.flags & ActionFlags::Send_Cookie)
      {
	param["COOKIE"] = QString("%1").arg(cookie.value()) ;
      }
      if(a.flags & ActionFlags::Send_Event_Attributes)
      {
	add_strings(param, attr.txt) ;
      }
      if(a.flags & ActionFlags::Send_Action_Attributes)
      {
	add_strings(param, a.attr.txt) ;
      }
      m << QVariant::fromValue(param) ;


      // TODO: is it safe to use dbus bindings after fork?
      // TODO: do we really get a fresh private connection or not?
      // TODO: should we use libdbus? or exec a helper?

      // send using fresh private connection
      QString cname = "timed-private";
      QDBusConnection::BusType ctype = QDBusConnection::SessionBus;

      if( a.flags & ActionFlags::Use_System_Bus ) ctype = QDBusConnection::SystemBus;

      QDBusConnection c = QDBusConnection::connectToBus(ctype, cname);

      int xc = 0;

      if( !c.send(m) )
      {
	log_child("#### [%d]: Failed to send a message on D-Bus: %s", cookie.value(), c.lastError().message().toStdString().c_str()) ;
	xc = 1;
      }
      else
      {
	log_child("#### [%d]: D-Bus Message sent", cookie.value()) ;

	// as we are about to exit immediately after queuing
	// and there seems to be no way to flush the connection
	// and be sure that we have actually transmitted the
	// message -> do a dummy synchronous query from dbus
	// daemon and hope ...

	pid_t   result    = -1;
	QString service   = "org.freedesktop.DBus";
	QString path      = "/org/freedesktop/DBus";
	QString interface = "org.freedesktop.DBus";
	QString method    = "GetConnectionUnixProcessID";

	QDBusMessage req  = QDBusMessage::createMethodCall(service,
							   path,
							   interface,
							   method);
	req << c.baseService();

	QDBusMessage rsp = c.call(req);

	if( rsp.type() == QDBusMessage::ReplyMessage )
	{
	  QList<QVariant> args = rsp.arguments();
	  if( !args.isEmpty() )
	  {
	    bool ok = false;
	    int reply = rsp.arguments().first().toInt(&ok);
	    log_warning("@@@ rsp.ok = %d, rsp.reply = %d", ok, reply);
	    if( ok ) result = reply;
	  }
	}

	// it should be us ...
	log_child("#### my pid: %d, connection owner pid: %d", getpid(), result);
      }

      QDBusConnection::disconnectFromBus(cname);

      _exit(xc);
    }
  }
#endif

  void event_t::prepare_command(const action_t &a, string &cmd, string &user)
  {
    user = find_action_attribute("USER", a, false) ;
    if(user.empty())
      user = "user" ;
    cmd = find_action_attribute("COMMAND", a) ;
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

#if 0
    QString cookie_val = QString("%1").arg(cookie.value()) ;
    if(a.attr(action_flags::Send_Cookie))
    {
      static QRegExp cookie_exp("(<COOKIE>)|\\b(COOKIE)\\b") ;
      while(cookie_exp.indexIn(cmd) != -1)
      {
        for(int i=1; i<=2; i++)
          if(int len = cookie_exp.cap(i).length())
          {
            int pos = cookie_exp.pos(i) ;
            cmd.replace(pos, len, cookie_val) ;
            break ;
          }
      }
    }

#endif


#if 0
    qDebug() << "cmd" << cmd ;
#endif

#if 0
    char *sys_cmd = strdup(cmd.toStdString().c_str()) ;
    log_assert(sys_cmd) ;
    int res = system(sys_cmd) ;// XXX: yes, yes, I know! TODO: fork + su as 'user' + exec
    log_info("Event [%d] in %s, executing: %s", cookie.value(), st->name, sys_cmd) ;
    if(res!=0)
      log_info("Event [%d] in %s, executing '%s' returning %d", cookie.value(), st->name, sys_cmd, res) ;
    free(sys_cmd) ;
#endif
  }

  int event_t::fork_and_set_credentials(const action_t &action, bool &error)
  {
    // assume failure
    int    pid  = -1;
    bool   err  = true;

    string user = find_action_attribute("USER", action, false) ;
    string cred = attr(string("CREDENTIALS"));

    struct passwd *pw = 0;

    // FIXME: remove debug logging later
    log_debug("as user: %s", user.c_str());
    log_debug("credentials: %s", cred.c_str());

    // bailout if credentials attribute is not set
    if( cred.empty() )
    {
      log_warning("no credential attribute, action skipped") ;
      goto cleanup;
    }

    // get user details if user attribute is set
    if( !user.empty() && !(pw = getpwnam(user.c_str())) )
    {
      // TODO: do log_xxx functions preserve errno?
      log_warning("getpwnam(%s) failed: %m", user.c_str()) ;
      goto cleanup;
    }

    // FORK: both child and parent will return with the
    // pid returned by fork(), the caller must inspect
    // also the error parameter to know whether the
    // child initialization was succesful or not

    // parent process

    if( (pid = fork()) != 0 )
    {
      if( pid > 0 )
      {
	// child was successfully started
	err = false;
	st->om->emit_child_created(cookie.value(), pid) ;
      }
      goto cleanup;
    }

    // child process

    // detach from session
    if( setsid() < 0 )
    {
      // TODO: do log_xxx functions preserve errno?
      log_child("#### setsid() failed: %m") ;
      goto cleanup;
    }

    // take stored client credentials in to use
    if( !credentials_set(QString::fromUtf8(cred.c_str())) )
    {
      log_child("#### credentials_set() failed") ;
      goto cleanup;
    }

    uid_t ruid, euid, suid;
    gid_t rgid, egid, sgid;

    ruid = euid = suid = -1;
    rgid = egid = sgid = -1;

    if( getresgid(&rgid, &egid, &sgid) < 0 )
    {
      log_child("#### getresgid() failed: %m") ;
      goto cleanup;
    }
    if( getresuid(&ruid, &euid, &suid) < 0 )
    {
      log_child("#### getresuid() failed: %m") ;
      goto cleanup;
    }
    log_child("#### uid: r=%d, e=%d, s=%d", ruid, euid, suid);
    log_child("#### gid: r=%d, e=%d, s=%d", rgid, egid, sgid);


    if( setresgid(egid, egid, egid) < 0 )
    {
      log_child("#### setresgid() failed: %m") ;
      goto cleanup;
    }
    if( setresuid(euid, euid, euid) < 0 )
    {
      log_child("#### setresuid() failed: %m") ;
      goto cleanup;
    }

    if( getresgid(&rgid, &egid, &sgid) < 0 )
    {
      log_child("#### getresgid() failed: %m") ;
      goto cleanup;
    }
    if( getresuid(&ruid, &euid, &suid) < 0 )
    {
      log_child("#### getresuid() failed: %m") ;
      goto cleanup;
    }
    log_child("#### uid: r=%d, e=%d, s=%d", ruid, euid, suid);
    log_child("#### gid: r=%d, e=%d, s=%d", rgid, egid, sgid);



    // if user attribute was not set, we will use the home directory
    // of the user id effective after setting the credentials
    if( pw == 0 )
    {
      uid_t uid = geteuid();

      if( !(pw = getpwuid(uid)) )
      {
	// TODO: do log_xxx functions preserve errno?
	log_child("#### getpwuid(%d) failed: %m", uid);
	goto cleanup;
      }
    }

    log_child("#### workdir: %s", pw->pw_dir);

    // set home directory as current working directory
    if( chdir(pw->pw_dir)<0 )
    {
      const char fallback[] = "/";

      log_child("#### chdir(\"%s\") failed: %m, trying \"%s\"", pw->pw_dir, fallback);

      if( chdir(fallback)<0 )
      {
	log_child("#### chdir(\"%s\") failed: %m", fallback);
	goto cleanup;
      }
    }

    // if user attribute was set, we will try to do the
    // uid and gid setting after setting the credentials
    // this allows actions added from root process to
    // be executed as user, but not the other way around
    if( !user.empty() )
    {
      if( setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) < 0 )
      {
	log_child("#### setresgid() failed: %m") ;
	goto cleanup;
      }

      if( setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid) < 0 )
      {
	log_child("#### setresuid() failed: %m") ;
	goto cleanup;
      }
    }

    if( getresgid(&rgid, &egid, &sgid) < 0 )
    {
      log_child("#### getresgid() failed: %m") ;
      goto cleanup;
    }
    if( getresuid(&ruid, &euid, &suid) < 0 )
    {
      log_child("#### getresuid() failed: %m") ;
      goto cleanup;
    }
    log_child("#### uid: r=%d, e=%d, s=%d", ruid, euid, suid);
    log_child("#### gid: r=%d, e=%d, s=%d", rgid, egid, sgid);

    // if we got here, all of the child process initialization
    // was succesfully completed
    err = false;

cleanup:
    return error = err, pid;
  }

#if 0
  void event_t::execute_command(const action_t &a)
  {
    string cmd, user ;
    prepare_command(a, cmd, user) ;

    string cred = attr(string("CREDENTIALS"));
    if( cred.empty() )
    {
      log_warning("skipped execute action without credentials attribute") ;
      return ;
    }
    // FIXME: remove debug logging later
    log_debug("execute: %s", cmd.c_str());
    log_debug("credentials: %s", cred.c_str());
    errno = 0 ;
// FIXME: what to do with the old setgid()/setuid() code?
// QUARANTINE     struct passwd *pw = getpwnam(user.c_str()) ;
// QUARANTINE     if(pw==NULL)
// QUARANTINE       throw event_exception((string)"getpwname() failed"+strerror(errno)) ;
    pid_t pid = fork() ;
    if(pid<0)
      throw event_exception((string)"fork() failed"+strerror(errno)) ;
    log_info("forked successfully, fork() returned '%d'", pid) ;
    if(pid>0)
    {
      st->om->emit_child_created(cookie.value(), pid) ;
      return ;
    }
    // child process
    try
    {
      if(setsid()<0)
        throw event_exception((string)"setsid() failed"+strerror(errno)) ;
      QString credentials = QString::fromUtf8(cred.c_str());
      if( !credentials_set(credentials) )
	throw event_exception((string)"creds_set('" + cred + ")") ;
      if(chdir("/")<0)
        throw event_exception((string)"chdir('/') failed"+strerror(errno)) ;

// FIXME: what to do with the old setgid()/setuid() code?
// QUARANTINE       if(user!="root")
// QUARANTINE       {
// QUARANTINE         if(chdir(pw->pw_dir)<0)
// QUARANTINE           throw event_exception((string)"chdir('"+pw->pw_dir+"') failed"+strerror(errno)) ;
// QUARANTINE         if(setgid(pw->pw_gid)<0)
// QUARANTINE           throw event_exception((string)"setgid() failed"+strerror(errno)) ;
// QUARANTINE         if(setuid(pw->pw_uid)<0)
// QUARANTINE           throw event_exception((string)"setuid() failed"+strerror(errno)) ;
// QUARANTINE       }
      execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL) ;
      throw event_exception((string)"execl('/bin/sh', '-c', '"+cmd+"') failed"+strerror(errno)) ;
    }
    catch(const event_exception &e)
    {
      log_error("event %d, child process failed: %s", cookie.value(), e.message.c_str()) ;
      _exit(1) ;
    }
  }
#else
  void event_t::execute_command(const action_t &a)
  {
    string cmd, user ;
    prepare_command(a, cmd, user) ;

    log_debug("execute: %s", cmd.c_str());

    bool error = true;
    int  child = fork_and_set_credentials(a, error);

    if( child != 0 )
    {
      // parent
      log_debug("execute: child pid = %d", child);
    }
    else if( error )
    {
      log_child("#### execute: child init failure");
      // cause of error logged @ fork_and_set_credentials()
      _exit(1);
    }
    else
    {
      log_child("#### execute: child init OK");
      // exec*() calls return only on failure
      log_child("#### execute: %s", cmd.c_str());
      execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL) ;

      // TODO: do log_xxx functions preserve errno?
      log_child("#### %s: failed: %m", cmd.c_str());
      _exit(1);
    }
  }
#endif

  iodata::record *event_t::save()
  {
    iodata::record *r = new iodata::record ;
    r->add("cookie", cookie.value()) ;
    r->add("ticker", ticker.value()) ;
    r->add("t", t.save()) ;
    r->add("tz", new iodata::bytes(tz)) ;
    r->add("attr", attr.save() ) ;
    r->add("flags", new iodata::bitmask(flags &~ EventFlags::Cluster_Mask, codec)) ;
    r->add("recrs", iodata::save(recrs)) ;
    r->add("actions", iodata::save(actions)) ;
//    r->add("dialog", dialog.save()) ;
    r->add("snooze", iodata::save_int_array(snooze)) ;
    r->add("b_attr", iodata::save(b_attr)) ;

    // io_state *iost = dynamic_cast<io_state*> (st) ;
#if 0
    r->add("dialog_time", iost->save_in_due_state() ? 0 : last_triggered.value()) ;
#else
    r->add("dialog_time", (flags & EventFlags::In_Dialog) ? last_triggered.value() : 0) ;
#endif
    r->add("tsz_max", tsz_max) ;
    r->add("tsz_counter", tsz_counter) ;
    return r ;
  }

  bool event_t::compute_recurrence()
  {
    log_assert(false, "obsolete function") ;
    return false ;
  }

#if 0
}
#endif
