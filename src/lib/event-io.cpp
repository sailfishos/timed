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

#include "interface.h"

#include "event-io.h"

bool Maemo::Timed::cred_modifier_io_t::operator==(const Maemo::Timed::cred_modifier_io_t &crio) const
{
  return token == crio.token && accrue == crio.accrue ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::attribute_io_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out << x.txt ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::attribute_io_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  return in >> x.txt ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::cred_modifier_io_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out << x.token << x.accrue ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::cred_modifier_io_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  return in >> x.token >> x.accrue ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::action_io_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out << x.flags << x.attr << x.cred_modifiers ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::action_io_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  return in >> x.flags >> x.attr >> x.cred_modifiers ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::button_io_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out << x.attr << x.snooze ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::button_io_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  return in >> x.attr >> x.snooze ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::recurrence_io_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out << x.mins << x.hour << x.mday << x.wday << x.mons << x.flags ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::recurrence_io_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  return in >> x.mins >> x.hour >> x.mday >> x.wday >> x.mons >> x.flags ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::event_io_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  // 1. alarm trigger time
  out << x.ticker ;
  out << x.t_year << x.t_month << x.t_day << x.t_hour << x.t_minute ;
  out << x.t_zone ;
  // 2. Attributes and flags
  out << x.attr << x.flags ;
  // 3. Dialog buttons
  out << x.buttons ;
  // 4. Actions
  out << x.actions ;
  // 5. Recurrence info
  out << x.recrs ;
  // 6. Additional parameters
  out << x.tsz_max << x.tsz_length << x.cred_modifiers ;
  return out ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::event_io_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  // 1. alarm trigger time
  in >> x.ticker ;
  in >> x.t_year >> x.t_month >> x.t_day >> x.t_hour >> x.t_minute ;
  in >> x.t_zone ;
  // 2. Attributes and flags
  in >> x.attr >> x.flags ;
  // 3. Dialog buttons
  in >> x.buttons ;
  // 4. Actions
  in >> x.actions ;
  // 5. Recurrence info
  in >> x.recrs ;
  // 6. Additional parameters
  in >> x.tsz_max >> x.tsz_length >> x.cred_modifiers ;
  return in ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::event_list_io_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out << x.ee ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::event_list_io_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  return in >> x.ee ;
}

register_qtdbus_metatype(Maemo::Timed::attribute_io_t, 1) ;
register_qtdbus_metatype(Maemo::Timed::cred_modifier_io_t, 7) ;
register_qtdbus_metatype(Maemo::Timed::action_io_t, 2) ;
register_qtdbus_metatype(Maemo::Timed::button_io_t, 3) ;
register_qtdbus_metatype(Maemo::Timed::recurrence_io_t, 4) ;
register_qtdbus_metatype(Maemo::Timed::event_io_t, 5) ;
register_qtdbus_metatype(Maemo::Timed::event_list_io_t, 6) ;
register_qtdbus_metatype(Q_List_uint, 8) ;
register_qtdbus_metatype(Q_Map_String_String, 9) ;
register_qtdbus_metatype(Q_Map_uint_String_String, 10) ;
