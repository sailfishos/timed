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
#ifndef AUTOMATA_H
#define AUTOMATA_H

#include <set>
#include <deque>
using namespace std ;

#include <QDBusPendingCallWatcher>

#include <qmlog>

#include "timed/event-io.h"
#include "timed/nanotime.h"

#include "wrappers.h"
#include "credentials.h"
#include "timeutil.h"

  struct state ;
  struct io_state ;
  struct gate_state ;
  struct filter_state ;
  struct abstract_cluster ;
  struct machine ;
  struct queue_pause ;

struct Timed ;

// some states:
struct state_epoch ;

struct event_t ;

  struct state
  {
    state(const char *, machine *) ;
    machine *om ;
    const char *name ;
    uint32_t action_mask ;
    uint32_t get_action_mask() { return action_mask ; }
    void set_action_mask(uint32_t a) { action_mask = a ; }
    virtual void enter(event_t *) ;
    virtual void leave(event_t *) ;
    virtual uint32_t cluster_bits() { return 0 ; }
  } ;

  struct io_state : public QObject, public state
  {
    set <event_t *> events ;
    io_state(const char *, machine *, QObject *p = NULL) ;
    void enter(event_t *) ;
    void leave(event_t *) ;
    virtual void abort(event_t *) ;
    // virtual bool save_in_due_state() = 0 ;
    Q_OBJECT ;
  } ;

  struct gate_state : public io_state
  {
    gate_state(const char *name, const char *nxt, machine *, QObject *p=NULL) ;
    const char *nxt_state ;
    bool is_open ;
    void enter(event_t *) ;
    Q_OBJECT ;
  public Q_SLOTS:
    void close() ;
    virtual void open() ;
  Q_SIGNALS:
    void closed() ;
    void opened() ;
  } ;

  struct filter_state : public gate_state
  {
    filter_state(const char *name, const char *retry, const char *nxt, machine *, QObject *p=NULL) ;
    const char *next ;
    virtual bool filter(event_t *) = 0 ;
    void enter(event_t *) ;
    Q_OBJECT ;
  Q_SIGNALS:
    void closed(filter_state *state) ;
  private Q_SLOTS:
    void emit_close() { emit closed(this) ; }
  } ;

  struct abstract_cluster
  {
    machine *om ;
    uint32_t bit ;
    const char *name ;
    abstract_cluster(machine *m, uint32_t b, const char *n) : om(m), bit(b), name(strdup(n)) { }
    virtual void enter(event_t *e) = 0 ;
    virtual void leave(event_t *) { }
  } ;

  struct machine : public QObject
  {
    machine(const Timed *daemon) ;

    const Timed *owner ;
    int next_cookie ;
    uint32_t flags ;
    map<string, state*> states ;
    map<cookie_t, event_t*> events ;
    map<uint32_t, abstract_cluster*> clusters ;
    deque <pair <event_t*, state*> > transition_queue ;
    ticker_t transition_start_time ;
    bool context_changed ;
    map<QDBusPendingCallWatcher *, event_t *> watcher_to_event ;
    map<int, state*> button_states ;
#if 0
    int default_snooze_value ;
#endif
    int dialog_discard_threshold ;
    int32_t signalled_bootup ;
    state_epoch *epoch ;

    bool transition_in_progress() { return transition_start_time.is_valid() ; }
    ticker_t transition_started() { return transition_start_time ; }
    uint32_t attr(uint32_t mask) ;
    cookie_t add_event(const Maemo::Timed::event_io_t *, bool process_queue, const credentials_t *pcreds, const QDBusMessage *message) ;
    void add_events(const Maemo::Timed::event_list_io_t &lst, QList<QVariant> &res, const QDBusMessage &message) ;
    void query(const QMap<QString,QVariant> &words, QList<QVariant> &res ) ;
    void get_event_attributes(cookie_t c, QMap<QString,QVariant> &a) ;
    bool cancel_by_cookie(cookie_t c) ;
    void cancel_event(event_t *e) ;
    event_t *find_event(cookie_t c) ;
    void alarm_gate(bool value) ;
    bool dialog_response(cookie_t c, int value) ;
    Q_INVOKABLE void process_transition_queue() ;
    void update_rtc_alarm() ;
    ticker_t calculate_bootup() ;
    void send_bootup_signal() ;
    void invoke_process_transition_queue() ;
    void reshuffle_queue(const nanotime_t &back) ;
    void request_state(event_t *e, state *st) ;
    void request_state(event_t *e, const char *state_name) ;
    void send_queue_context() ;
    Q_OBJECT ;
  public Q_SLOTS:
    void call_returned(QDBusPendingCallWatcher *) ; // rename ?
  Q_SIGNALS:
    void engine_pause(int dx) ;
    void voland_registered() ;
    void voland_unregistered() ;
    void queue_to_be_saved() ;
    void voland_needed() ;
    void next_bootup_event(int) ;
    void child_created(unsigned, int) ;
  public:
    void emit_child_created(unsigned cookie, int pid) { emit child_created(cookie, pid) ; }
#if 0
    int default_snooze(int new_value=0) ;
#endif
    queue_pause *initial_pause ;
    void emit_engine_pause(int dx) { emit engine_pause(dx) ; }
    void start() ; //  { delete initial_pause ; initial_pause = NULL ; process_transition_queue() ; }
    void device_mode_detected(bool user_mode) ;
    bool is_epoch_open() ;
    void open_epoch() ;
    iodata::record *save(bool for_backup) ;
    void load(const iodata::record *) ;
    void load_events(const iodata::array *events_data, bool trusted_source, bool use_cookies) ;
    void cancel_backup_events() ;
  } ;

  struct queue_pause
  {
    machine *om ;
    queue_pause(machine *m) : om(m) { om->emit_engine_pause(+1) ; }
   ~queue_pause() { om->emit_engine_pause(-1) ; }
  } ;

#endif
