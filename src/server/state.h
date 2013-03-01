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
#ifndef STATE_H
#define STATE_H

#include <stdint.h>

#include <algorithm>
#include <string>
#include <set>

#include <QObject>
#include <QTimer>

#include "timeutil.h"
#include "wrappers.h"
#include "flags.h"

struct machine_t ;
struct event_t ;

/* Abstract states */

struct abstract_state_t
{
  abstract_state_t(const std::string &state_name, machine_t *owner) ;
  virtual ~abstract_state_t() { }
  machine_t *machine ;
  std::string s_name ;
  uint32_t action_mask ;
  const char *name() const { return s_name.c_str() ; }
  uint32_t get_action_mask() { return action_mask ; }
  void set_action_mask(uint32_t a) { action_mask = a ; }
  void go_to(event_t *e) ;
  virtual void enter(event_t *) ;
  virtual void leave(event_t *) { }
  virtual uint32_t cluster_bits() { return 0 ; }
  virtual void resolve_names() { }
} ;

struct abstract_io_state_t : public QObject, public abstract_state_t
{
  std::set<event_t *> events ;
  abstract_io_state_t(const std::string &state_name, machine_t *owner) : abstract_state_t(state_name, owner) { }
  void enter(event_t *) ;
  void leave(event_t *) ;
  virtual void abort(event_t *) ;
  virtual void abort_all(abstract_state_t *st) ; // move all events to (usually WAITING) state st
  virtual ~abstract_io_state_t() { }
  Q_OBJECT ;
} ;

struct abstract_gate_state_t : public abstract_io_state_t
{
  abstract_gate_state_t(const std::string &state_name, const std::string &next_state_name, machine_t *owner) ;
  virtual ~abstract_gate_state_t() { }
  std::string s_next_state ;
  abstract_state_t *next_state ;
  void resolve_names() ;
  bool is_open ;
  bool is_closed() { return not is_open ; }
  void enter(event_t *) ;
  Q_OBJECT ;
public Q_SLOTS:
  void close() ;
  virtual void open() ;
Q_SIGNALS:
  void closed() ;
  void opened() ;
} ;

struct abstract_concentrating_state_t : public abstract_gate_state_t
{
  abstract_concentrating_state_t(const std::string &state_name, const std::string &next_state_name, machine_t *owner)
    : abstract_gate_state_t(state_name, next_state_name, owner) { }
  virtual ~abstract_concentrating_state_t() { }
  Q_OBJECT ;
public Q_SLOTS:
  void open() ;
} ;

struct abstract_filter_state_t : public abstract_gate_state_t
{
  abstract_filter_state_t(const std::string &state_name, const std::string &retry_state_name, const std::string &thru_state_name, machine_t *owner) ;
  virtual ~abstract_filter_state_t() { }
  std::string s_thru_state ;
  abstract_state_t *thru_state ;
  void resolve_names() ;
  virtual bool filter(event_t *) = 0 ;
  void enter(event_t *) ;
  Q_OBJECT ;
Q_SIGNALS:
  void closed(abstract_filter_state_t *state) ;
private Q_SLOTS:
  void emit_close() { emit closed(this) ; }
} ;

/* States */

struct state_start_t : public abstract_state_t
{
  state_start_t(machine_t *owner) : abstract_state_t("START", owner) { }
  virtual ~state_start_t() { }
  void enter(event_t *e) ;
} ;

struct state_epoch_t : public abstract_gate_state_t
{
  Q_OBJECT ;
public:
  static const char *lost() { return "/var/cache/timed/SYSTEM_TIME_LOST" ; }
  state_epoch_t(machine_t *owner) ;
  virtual ~state_epoch_t() { }
  void abort_all(abstract_state_t *st) ; // not moving all the events to WAITING
public Q_SLOTS:
  void open() ;
} ;

struct state_waiting_t : public abstract_gate_state_t
{
  Q_OBJECT ;
public:
  state_waiting_t(machine_t *owner) ;
  virtual ~state_waiting_t() { }
} ;

struct state_new_t : public abstract_state_t
{
  state_new_t(machine_t *owner) : abstract_state_t("NEW", owner) { }
  virtual ~state_new_t() { }
  void enter(event_t *e) ;
} ;

struct state_scheduler_t : public abstract_state_t
{
  state_scheduler_t(machine_t *owner) : abstract_state_t("SCHEDULER", owner) { }
  virtual ~state_scheduler_t() { }
  void enter(event_t *e) ;
} ;

struct state_qentry_t : public abstract_state_t
{
  state_qentry_t(machine_t *owner) : abstract_state_t("QENTRY", owner) { }
  virtual ~state_qentry_t() { }
  void enter(event_t *e) ;
  uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
} ;

struct state_queued_t : public abstract_io_state_t
{
  int pause_x ;
  state_queued_t(machine_t *owner) ;
  virtual ~state_queued_t() ;
  void enter(event_t *e) ;
  void leave(event_t *e) ;
  void timer_start() ;
  void timer_stop() ;
  QTimer *alarm_timer ;
  typedef std::pair<ticker_t, event_t *> event_pair ;
  ticker_t next_bootup() ;
  ticker_t next_event_without_bootflag();
  std::set<event_pair> queue ;
  std::set<event_pair> bootup ;
  uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
  Q_OBJECT ;
public Q_SLOTS:
  void engine_pause(int dx) ;
  void alarm_timeout() ;
  void filter_closed(abstract_filter_state_t *f_st) ;
Q_SIGNALS:
  void sleep() ;
} ;

struct state_missed_t : public abstract_state_t
{
  state_missed_t(machine_t *owner) : abstract_state_t("MISSED", owner) { }
  virtual ~state_missed_t() { }
  void enter(event_t *e) ;
} ;

struct state_due_t : public abstract_state_t
{
  state_due_t(machine_t *owner) : abstract_state_t("DUE", owner) { }
  virtual ~state_due_t() { }
  void enter(event_t *e) ;
} ;

struct state_skipped_t : public abstract_state_t
{
  state_skipped_t(machine_t *owner) : abstract_state_t("SKIPPED", owner) { }
  virtual ~state_skipped_t() { }
  void enter(event_t *e) ;
} ;

struct state_flt_conn_t : public abstract_filter_state_t
{
  state_flt_conn_t(machine_t *owner) : abstract_filter_state_t("FLT_CONN", "QENTRY", "FLT_ALRM", owner) { }
  virtual ~state_flt_conn_t() { }
  uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
  bool filter(event_t *) ;
  Q_OBJECT ;
} ;

struct state_flt_alrm_t : public abstract_filter_state_t
{
  state_flt_alrm_t(machine_t *owner) : abstract_filter_state_t("FLT_ALRM", "QENTRY", "FLT_USER", owner) { }
  virtual ~state_flt_alrm_t() { }
  uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
  bool filter(event_t *) ;
  Q_OBJECT ;
} ;

struct state_flt_user_t : public abstract_filter_state_t
{
  state_flt_user_t(machine_t *owner) : abstract_filter_state_t("FLT_USER", "QENTRY", "QUEUED", owner) { }
  virtual ~state_flt_user_t() { }
  uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
  bool filter(event_t *) ;
  Q_OBJECT ;
} ;

struct state_snoozed_t : public abstract_state_t
{
  state_snoozed_t(machine_t *owner) : abstract_state_t("SNOOZED", owner) { }
  virtual ~state_snoozed_t() { }
  void enter(event_t *e) ;
} ;

struct state_recurred_t : public abstract_state_t
{
  state_recurred_t(machine_t *owner) : abstract_state_t("RECURRED", owner) { }
  virtual ~state_recurred_t() { }
  void enter(event_t *e) ;
  ticker_t apply_pattern(const broken_down_t &t, int wday, const recurrence_pattern_t *p) ;
} ;

struct state_armed_t : public abstract_gate_state_t
{
  state_armed_t(machine_t *owner) : abstract_gate_state_t("ARMED", "TRIGGERED", owner) { }
  virtual ~state_armed_t() { }
private:
  Q_OBJECT ;
} ;

struct state_triggered_t : public abstract_state_t
{
  state_triggered_t(machine_t *owner) : abstract_state_t("TRIGGERED", owner) { }
  virtual ~state_triggered_t() { }
  void enter(event_t *e) ;
} ;

struct state_button_t : public abstract_state_t
{
  signed int no ;
  // static QString init_name(int n) ;
  state_button_t(machine_t *owner, int n) ;
  virtual ~state_button_t() { }
  void enter(event_t *e) ;
} ;

struct state_served_t : public abstract_state_t
{
  state_served_t(machine_t *owner) : abstract_state_t("SERVED", owner) { }
  virtual ~state_served_t() { }
  void enter(event_t *e) ;
} ;

struct state_tranquil_t : public abstract_io_state_t
{
  state_tranquil_t(machine_t *owner) : abstract_io_state_t("TRANQUIL", owner) { }
  virtual ~state_tranquil_t() { }
  void abort_all(abstract_state_t *st) ; // not moving all the events to WAITING
} ;

struct state_removed_t : public abstract_state_t
{
  state_removed_t(machine_t *owner) : abstract_state_t("REMOVED", owner) { }
  virtual ~state_removed_t() { }
  void enter(event_t *e) ;
} ;

struct state_finalized_t : public abstract_state_t
{
  state_finalized_t(machine_t *owner) : abstract_state_t("FINALIZED", owner) { }
  virtual ~state_finalized_t() { }
  void enter(event_t *e) ;
} ;

struct state_aborted_t : public abstract_state_t
{
  state_aborted_t(machine_t *owner) : abstract_state_t("ABORTED", owner) { }
  virtual ~state_aborted_t() { }
  void enter(event_t *e) ;
} ;

struct state_dlg_wait_t : public abstract_gate_state_t
{
public:
  state_dlg_wait_t(machine_t *owner) : abstract_gate_state_t("DLG_WAIT", "DLG_CNTR", owner) { }
  virtual ~state_dlg_wait_t() { }
  void enter(event_t *e) ;
  uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
Q_SIGNALS:
  void voland_needed() ;
private:
  Q_OBJECT ;
} ;

struct state_dlg_cntr_t : public abstract_concentrating_state_t
{
  state_dlg_cntr_t(machine_t *owner) ; /* : abstract_concentrating_state_t("DLG_CNTR", "DLG_REQU", owner), s_back_state("DLG_WAIT") { } */ ;
  string s_back_state ;
  abstract_state_t *back_state ;
  void resolve_names() ;
  virtual ~state_dlg_cntr_t() { }
  uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
  void request_voland() ;
public Q_SLOTS:
  void open() ;
  void send_back() ;
private:
  Q_OBJECT ;
} ;

struct state_dlg_requ_t : public abstract_gate_state_t
{
  state_dlg_requ_t(machine_t *owner) : abstract_gate_state_t("DLG_REQU", "DLG_WAIT", owner) { }
  virtual ~state_dlg_requ_t() { }
  void enter(event_t *e) ;
  uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
  void abort(event_t *e) ;
  Q_OBJECT ;
public Q_SLOTS:
} ;

struct state_dlg_user_t : public abstract_gate_state_t
{
  state_dlg_user_t(machine_t *owner) : abstract_gate_state_t("DLG_USER", "DLG_WAIT", owner) { }
  virtual ~state_dlg_user_t() { }
  uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
  void abort(event_t *e) ;
  Q_OBJECT ;
public Q_SLOTS:
} ;

struct state_dlg_resp_t : public abstract_state_t
{
  state_dlg_resp_t(machine_t *owner) : abstract_state_t("DLG_RESP", owner) { }
  virtual ~state_dlg_resp_t() { }
} ;


#endif//STATE_H
