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
#ifndef STATES_H
#define STATES_H

#include <QObject>
#include <QTimer>
#include <ContextProvider>

#include <qm/log>

#include "automata.h"
#include "timeutil.h"
#include "flags.h"

  struct state_start : public state
  {
    state_start(machine *am) : state("START",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_epoch : public gate_state
  {
    Q_OBJECT ;
    static const char *lost ;
  public:
    state_epoch(machine *am) ;
  public Q_SLOTS:
    void open() ; // a virtual slot
  } ;

  struct state_new : public state
  {
    state_new(machine *am) : state("NEW",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_scheduler : public state
  {
    state_scheduler(machine *am) : state("SCHEDULER",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_qentry : public state
  {
    state_qentry(machine *am) : state("QENTRY",am) {}
    void enter(event_t *e) ;
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
  } ;

  struct state_queued : public io_state
  {
    int pause_x ;
    state_queued(machine *am) ;
    void enter(event_t *e) ;
    void leave(event_t *e) ;
    void timer_start() ;
    void timer_stop() ;
    QTimer *alarm_timer ;
    typedef pair<ticker_t, event_t *> event_pair ;
    ticker_t next_bootup() ;
    ticker_t next_rtc_bootup() ;
    set<event_pair> queue ;
    set<event_pair> bootup ;
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
    Q_OBJECT ;
  public Q_SLOTS:
    void engine_pause(int dx) ;
    void alarm_timeout() ;
    void filter_closed(filter_state *f_st) ;
  Q_SIGNALS:
    void sleep() ;
  } ;


  struct state_missed : public state
  {
    state_missed(machine *am) : state("MISSED",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_due : public state
  {
    state_due(machine *am) : state("DUE",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_skipped : public state
  {
    state_skipped(machine *am) : state("SKIPPED",am) {}
    void enter(event_t *e) { om->request_state(e, "SERVED") ; }
  } ;

  struct state_flt_conn : public filter_state
  {
    state_flt_conn(machine *am) : filter_state("FLT_CONN", "QENTRY", "FLT_ALRM", am) { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
    bool filter(event_t *) ;
    Q_OBJECT ;
  } ;

  struct state_flt_alrm : public filter_state
  {
    state_flt_alrm(machine *am) : filter_state("FLT_ALRM", "QENTRY", "FLT_USER", am) { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
    bool filter(event_t *) ;
    Q_OBJECT ;
  } ;

  struct state_flt_user : public filter_state
  {
    state_flt_user(machine *am) : filter_state("FLT_USER", "QENTRY", "QUEUED", am) {}
    uint32_t cluster_bits() { return EventFlags::Cluster_Queue ; }
    bool filter(event_t *) ;
    Q_OBJECT ;
  } ;

  struct state_snoozed : public state
  {
    state_snoozed(machine *am) : state("SNOOZED",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_recurred : public state
  {
    state_recurred(machine *am) : state("RECURRED",am) {}
    void enter(event_t *e) ;
    ticker_t apply_pattern(broken_down_t &t, int wday, const recurrence_pattern_t *p) ;
  } ;

  struct state_armed : public gate_state
  {
    state_armed(machine *am) : gate_state("ARMED", "TRIGGERED", am) { }
  private:
    Q_OBJECT ;
  } ;

  struct state_triggered : public state
  {
    state_triggered(machine *am) : state("TRIGGERED",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_button : public state
  {
    signed no ;
    static QString init_name(signed n) ;
    state_button(machine *am, signed n) ;
    void enter(event_t *e) ;
  } ;

  struct state_served : public state
  {
    state_served(machine *am) : state("SERVED",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_tranquil : public io_state
  {
    state_tranquil(machine *am) : io_state("TRANQUIL", am) { }
  } ;

  struct state_removed : public state
  {
    state_removed(machine *am) : state("REMOVED",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_finalized : public state
  {
    state_finalized(machine *am) : state("FINALIZED",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_aborted : public state
  {
    state_aborted(machine *am) : state("ABORTED",am) {}
    void enter(event_t *e) ;
  } ;

  struct state_dlg_wait : public gate_state
  {
  public:
    state_dlg_wait(machine *am) : gate_state("DLG_WAIT", "DLG_CNTR", am) { }
    void enter(event_t *e) ;
    uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
  Q_SIGNALS:
    void voland_needed() ;
  private:
    Q_OBJECT ;
  } ;

  struct state_dlg_cntr : public concentrating_state
  {
    const char *back ;
    state_dlg_cntr(machine *am) : concentrating_state("DLG_CNTR", "DLG_REQU", am), back("DLG_WAIT") { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
    void request_voland() ;
  public Q_SLOTS:
    void open() ;
    void send_back() ;
  private:
    Q_OBJECT ;
  } ;

  struct state_dlg_requ : public gate_state
  {
    state_dlg_requ(machine *am) : gate_state("DLG_REQU", "DLG_WAIT", am) { }
    void enter(event_t *e) ;
    uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
    void abort(event_t *e) ;
    Q_OBJECT ;
  public Q_SLOTS:
  } ;

  struct state_dlg_user : public gate_state
  {
    state_dlg_user(machine *am) : gate_state("DLG_USER", "DLG_WAIT", am) { }
    uint32_t cluster_bits() { return EventFlags::Cluster_Dialog ; }
    void abort(event_t *e) ;
    Q_OBJECT ;
  public Q_SLOTS:
  } ;

  struct state_dlg_resp : public state
  {
    state_dlg_resp(machine *am) : state("DLG_RESP", am) { }
  } ;

  struct cluster_queue : public abstract_cluster
  {
    QMap<QString,QVariant> alarm_triggers ;
    cluster_queue(machine *m) : abstract_cluster(m, EventFlags::Cluster_Queue, "QUEUE") { }
    void enter(event_t *e) ;
    void leave(event_t *e) ;
  } ;

  struct cluster_dialog : public abstract_cluster
  {
    set<event_t *> bootup_events ;
    cluster_dialog(machine *m) : abstract_cluster(m, EventFlags::Cluster_Dialog, "DIALOG") { }
    void enter(event_t *e) ;
    void leave(event_t *e) ;
    bool has_bootup_events() ;
  } ;

#endif // STATES_H
