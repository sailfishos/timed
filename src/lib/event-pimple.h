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
#ifndef EVENT_PIMPLE_H
#define EVENT_PIMPLE_H

#include <memory>
#include <vector>

#include "../server/event.h"
#include "event-io.h"

struct Maemo::Timed::event_pimple_t
{
 ~event_pimple_t() ;
  struct event_io_t eio ;
  event_pimple_t() {} ;
  event_pimple_t(const event_io_t &e_io) ;
  std::vector<event_action_pimple_t*> a ;
  std::vector<event_button_pimple_t*> b ;
  std::vector<event_recurrence_pimple_t*> r ;
} ;

struct Maemo::Timed::event_action_pimple_t
{
  unsigned action_no ;
  std::auto_ptr<Event::Action> ptr ;
  event_io_t *eio ;
  action_io_t *aio() { return & eio->actions[action_no] ; }
} ;

struct Maemo::Timed::event_button_pimple_t
{
  unsigned button_no ;
  std::auto_ptr<Event::Button> ptr ;
  event_io_t *eio ;
  button_io_t *bio() { return & eio->buttons[button_no] ; }
} ;

struct Maemo::Timed::event_recurrence_pimple_t
{
  unsigned recurrence_no ;
  std::auto_ptr<Event::Recurrence> ptr ;
  event_io_t *eio ;
  recurrence_io_t *rio() { return & eio->recrs[recurrence_no] ; }
} ;

struct Maemo::Timed::event_list_pimple_t
{
 ~event_list_pimple_t() ;
 std::vector<Event *> events ;
} ;

#endif
