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
#ifndef CLUSTER_H
#define CLUSTER_H

struct machine_t ;
struct event_t ;

struct abstract_cluster_t
{
  machine_t *machine ;
  uint32_t bit ;
  string name ;
  abstract_cluster_t(machine_t *owner, uint32_t b, const char *n) : machine(owner), bit(b), name(n) { }
  virtual ~abstract_cluster_t() { }
  virtual void enter(event_t *e) = 0 ;
  virtual void leave(event_t *) { }
} ;

struct cluster_queue_t : public abstract_cluster_t
{
  QMap<QString,QVariant> alarm_triggers ;
  cluster_queue_t(machine_t *m) : abstract_cluster_t(m, EventFlags::Cluster_Queue, "QUEUE") { }
  virtual ~cluster_queue_t() { }
  void enter(event_t *e) ;
  void leave(event_t *e) ;
} ;

struct cluster_dialog_t : public abstract_cluster_t
{
  set<event_t *> bootup_events ;
  cluster_dialog_t(machine_t *m) : abstract_cluster_t(m, EventFlags::Cluster_Dialog, "DIALOG") { }
  virtual ~cluster_dialog_t() { }
  void enter(event_t *e) ;
  void leave(event_t *e) ;
  bool has_bootup_events() ;
} ;

#endif//CLUSTER_H
