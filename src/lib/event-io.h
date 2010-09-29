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
#ifndef EVENT_IO_H
#define EVENT_IO_H

#include <stdint.h>

#include <QVector>
#include <QString>
#include <QMap>

#include "qmacro.h"

namespace Maemo
{
  namespace Timed
  {
    struct attribute_io_t ;
    struct cred_modifier_io_t ;
    struct action_io_t ;
    struct button_io_t ;
    struct recurrence_io_t ;
    struct event_io_t ;
    struct event_list_io_t ;
  }
}

struct Maemo::Timed::attribute_io_t
{
  QMap<QString, QString> txt ;
} ;

struct Maemo::Timed::cred_modifier_io_t
{
  QString token ;
  bool accrue ;
  cred_modifier_io_t() : accrue(false) { }
} ;

struct Maemo::Timed::action_io_t
{
  attribute_io_t attr ;
  uint32_t flags ;
  QVector<cred_modifier_io_t> cred_modifiers ;
  action_io_t() { flags = 0 ; }
} ;

struct Maemo::Timed::button_io_t
{
  attribute_io_t attr ;
  uint32_t snooze ;
  button_io_t() { snooze = 0 ; }
} ;

struct Maemo::Timed::recurrence_io_t
{
  uint64_t mins ;
  uint32_t hour ;
  uint32_t mday ;
  uint32_t wday ;
  uint32_t mons ;
  recurrence_io_t() { mins = hour = mday = wday = mons = 0 ; }
} ;

struct Maemo::Timed::event_io_t
{
  // 1. alarm trigger time
  int32_t ticker ;
  uint32_t t_year, t_month, t_day, t_hour, t_minute ;
  QString t_zone ;
  // 2. Attributes and flags
  attribute_io_t attr ;
  uint32_t flags ;
  // 3. Dialog buttons
  QVector<button_io_t> buttons ;
  // 4. Actions
  QVector<action_io_t> actions ;
  // 5. Recurrence info
  QVector<recurrence_io_t> recrs ;
  // 6. Additional parameters
  int32_t tsz_max, tsz_length ;
  QVector<cred_modifier_io_t> cred_modifiers ;
  event_io_t() { ticker = t_year = t_month = t_day = t_hour = t_minute = flags = tsz_max = tsz_length = 0 ; }
#if 0
 ~event_io_t() { }
#define _x(a) a(x.a)
  event_io_t(const event_io_t &x) :
    _x(ticker), _x(t_year), _x(t_month), _x(t_day), _x(t_hour), _x(t_minute), _x(t_zone),
    _x(attr), _x(flags), _x(buttons), _x(actions), _x(recrs)
  {
    qDebug() << __PRETTY_FUNCTION__ ;
  }
#undef _x
#endif
} ;

struct Maemo::Timed::event_list_io_t
{
  QVector<event_io_t> ee ;
} ;

declare_qtdbus_io(Maemo::Timed::attribute_io_t) ;
declare_qtdbus_io(Maemo::Timed::cred_modifier_io_t) ;
declare_qtdbus_io(Maemo::Timed::action_io_t) ;
declare_qtdbus_io(Maemo::Timed::button_io_t) ;
declare_qtdbus_io(Maemo::Timed::recurrence_io_t) ;
declare_qtdbus_io(Maemo::Timed::event_io_t) ;
declare_qtdbus_io(Maemo::Timed::event_list_io_t) ;

#endif
