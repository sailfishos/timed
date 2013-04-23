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
#ifndef MACHINE_H
#define MACHINE_H

#include <stdint.h>

#include <set>
#include <map>
#include <deque>

#include <QObject>

#include "../lib/event-io.h"
#include "../lib/nanotime.h"

#include "credentials.h"
#include "wrappers.h"
#include "state.h"
#include "cluster.h"

struct abstract_cluster_t ;
struct Timed ;
struct event_t ;

struct machine_t : public QObject
{
  machine_t(const Timed *owner) ;
  virtual ~machine_t() ;

  const Timed *timed ;
  struct pause_t ;

  int next_cookie ;
  uint32_t flags ;
  std::map<cookie_t, event_t*> events ;
  std::map<uint32_t, abstract_cluster_t*> clusters ;
  std::deque <pair <event_t*, abstract_state_t*> > transition_queue ;
  ticker_t transition_start_time ;
  nanotime_t transition_time_adjustment ;
  bool context_changed ;
  int dialog_discard_threshold ;
  int32_t signalled_bootup ;
  int32_t signalled_non_boot_event;

  bool transition_in_progress() { return transition_start_time.is_valid() ; }
  ticker_t transition_started() { return transition_start_time ; }
  uint32_t attr(uint32_t mask) ;
  cookie_t add_event(const Maemo::Timed::event_io_t *, bool process_queue, const credentials_t *pcreds, const QDBusMessage *message) ;
  void add_events(const Maemo::Timed::event_list_io_t &lst, QList<QVariant> &res, const QDBusMessage &message) ;
  void query(const QMap<QString,QVariant> &words, QList<QVariant> &res ) ;
  void get_event_attributes(cookie_t c, QMap<QString,QVariant> &a) ;

  void get_attributes_by_cookie(uint cookie, QMap<QString,QString> &a) ;
  void get_attributes_by_cookies(const QList<uint> &cookies, QMap<uint, QMap<QString,QString> > &a) ;
  void get_cookies_by_attributes(const QMap<QString,QString> &words, QList<uint> &res) ;

  bool cancel_by_cookie(cookie_t c) ;
  void cancel_events(const QList<uint> &cookies, QList<uint> &failed) ;
  void cancel_event(event_t *e) ;
  event_t *find_event(cookie_t c) ;
  void alarm_gate(bool open) ;
  bool dialog_response(cookie_t c, int value) ;
  Q_INVOKABLE void process_transition_queue() ;
  ticker_t calculate_bootup() ;
  void send_bootup_signal() ;
  void invoke_process_transition_queue() ;
  void reshuffle_queue(const nanotime_t &back) ;
  void request_state(event_t *e, abstract_state_t *st) ;
  void register_event(event_t *e) ;
  void unregister_event(event_t *e) ;
  bool is_event_registered(event_t *e) ;
  void send_queue_context() ;
  Q_OBJECT ;
public Q_SLOTS:
  void online_state_changed(bool connected) ;
Q_SIGNALS:
  void engine_pause(int dx) ;
  void voland_registered() ;
  void voland_unregistered() ;
  void queue_to_be_saved() ;
  void voland_needed() ;
  void next_bootup_event(int, int);
  void child_created(unsigned, int) ;
public:
  void emit_child_created(unsigned cookie, int pid) { emit child_created(cookie, pid) ; }
  pause_t *initial_pause ;
  void emit_engine_pause(int dx) { emit engine_pause(dx) ; }
  void start() ; //  { delete initial_pause ; initial_pause = NULL ; process_transition_queue() ; }
  void device_mode_detected(bool user_mode) ;
  bool is_epoch_open() ;
  void open_epoch() ;
  iodata::record *save(bool for_backup) ;
  void load(const iodata::record *) ;
  void load_events(const iodata::array *events_data, bool trusted_source, bool use_cookies) ;
  void cancel_backup_events() ;
  abstract_state_t *state_by_name(const string &name) ;
  void freeze() ;
  void unfreeze() ;
  bool is_frozen() ;
private:
  string s_states() ;
  string s_transition_queue() ;

public:
  // states
  std::set<abstract_state_t*> states ;
  std::map<int, state_button_t*> buttons ;
  state_start_t *state_start ;
  state_epoch_t *state_epoch ;
  state_waiting_t *state_waiting ;
  state_new_t *state_new ;
  state_scheduler_t *state_scheduler ;
  state_qentry_t *state_qentry ;
  state_queued_t *state_queued ;
  state_missed_t *state_missed ;
  state_due_t *state_due ;
  state_skipped_t *state_skipped ;
  state_flt_conn_t *state_flt_conn ;
  state_flt_alrm_t *state_flt_alrm ;
  state_flt_user_t *state_flt_user ;
  state_snoozed_t *state_snoozed ;
  state_recurred_t *state_recurred ;
  state_armed_t *state_armed ;
  state_triggered_t *state_triggered ;
  state_served_t *state_served ;
  state_tranquil_t *state_tranquil ;
  state_removed_t *state_removed ;
  state_finalized_t *state_finalized ;
  state_aborted_t *state_aborted ;
  state_dlg_wait_t *state_dlg_wait ;
  state_dlg_cntr_t *state_dlg_cntr ;
  state_dlg_requ_t *state_dlg_requ ;
  state_dlg_user_t *state_dlg_user ;
  state_dlg_resp_t *state_dlg_resp ;

  // clusters
  cluster_queue_t *cluster_queue ;
  cluster_dialog_t *cluster_dialog ;

} ;

struct machine_t::pause_t
{
  machine_t *machine ;
  pause_t(machine_t *owner) { (machine=owner)->emit_engine_pause(+1) ; }
 ~pause_t() { machine->emit_engine_pause(-1) ; }
} ;

struct request_watcher_t : public QObject
{
  Q_OBJECT ;
  machine_t *machine ;
  set<event_t*> events ;
  QDBusPendingCallWatcher *w ;
public:
  request_watcher_t(machine_t *owner) ;
 ~request_watcher_t() ;
  void watch(const QDBusPendingCall &async_call) ;
  void attach(event_t *e) ;
  void detach(event_t *e) ;
private:
  void detach_not_destroy(event_t *e) ;
private Q_SLOTS:
  void call_returned(QDBusPendingCallWatcher *w) ;
} ;

#endif//MACHINE_H
