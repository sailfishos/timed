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
#include "../common/log.h"
#include "event.h"
#include "machine.h"
#include "cluster.h"

void cluster_queue_t::enter(event_t *e)
{
  log_debug() ;
  bool empty_r = e->flags & EventFlags::Empty_Recurring ;
  bool alarm = e->flags & EventFlags::Alarm ;
  if(alarm && !empty_r)
  {
    QString key = QString("%1").arg(e->cookie.value()) ;
    uint64_t value = (uint64_t)nanotime_t::NANO * e->trigger.value() ;
    alarm_triggers.insert(key, value) ;
    log_debug("inserted %s=>%lld, state=%s", key.toStdString().c_str(), value, e->state->name()) ;
    machine->context_changed = true ;
  }
}

void cluster_queue_t::leave(event_t *e)
{
  bool empty_r = e->flags & EventFlags::Empty_Recurring ;
  bool alarm = e->flags & EventFlags::Alarm ;
  log_debug() ;
  if(alarm && !empty_r)
  {
    log_debug() ;
    QString key = QString("%1").arg(e->cookie.value()) ;
    alarm_triggers.remove(key) ;
    log_debug("removed %s=>'' state=%s", key.toStdString().c_str(), e->state ? e->state->name() : "null") ;
    machine->context_changed = true ;
  }
}

void cluster_dialog_t::enter(event_t *e)
{
  bool is_bootup = e->flags & EventFlags::Boot ;
  if(is_bootup)
    log_debug("insertng [%d]  to  cluster_dialog", e->cookie.value()) ;
  if(is_bootup)
    bootup_events.insert(e) ;
}

void cluster_dialog_t::leave(event_t *e)
{
  if(bootup_events.count(e))
    log_debug("removing [%d] from cluster_dialog", e->cookie.value()) ;
  bootup_events.erase(e) ;
}

bool cluster_dialog_t::has_bootup_events()
{
  return not bootup_events.empty() ;
}
