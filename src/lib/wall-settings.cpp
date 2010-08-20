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
#include <timed/qmacro.h>
#include <timed/wallclock>
#include "timed/exception.h"

#include "daemon/flags.h"

#include "wall-settings.h"

#include "nanotime.h"


Maemo::Timed::WallClock::Settings::Settings()
{
  p = new wall_settings_pimple_t ;
}

Maemo::Timed::WallClock::Settings::~Settings()
{
  delete p ;
}

void Maemo::Timed::WallClock::Settings::setTimeNitz()
{
  if(p->opcodes & WallOpcode::Op_Set_Time_Mask)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Time_Nitz ;
}

void Maemo::Timed::WallClock::Settings::setTimeManual()
{
  if(p->opcodes & WallOpcode::Op_Set_Time_Mask)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Time_Manual ;
}

void Maemo::Timed::WallClock::Settings::setTimeManual(time_t value)
{
  if(p->opcodes & WallOpcode::Op_Set_Time_Mask)
    p->valid = false ;
  if(value<=0)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Time_Manual_Val ;
  p->time_at_zero = nanotime_t::from_time_t(value)-nanotime_t::monotonic_now() ;
}

void Maemo::Timed::WallClock::Settings::setOffsetCellular()
{
  if(p->opcodes & WallOpcode::Op_Set_Zone_Mask)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Offset_Cellular ;
}

void Maemo::Timed::WallClock::Settings::setOffsetManual()
{
  if(p->opcodes & WallOpcode::Op_Set_Zone_Mask)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Offset_Manual ;
}

void Maemo::Timed::WallClock::Settings::setOffsetManual(int off)
{
  if(p->opcodes & WallOpcode::Op_Set_Zone_Mask)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Offset_Manual_Val ;
  p->offset = off ;
}

void Maemo::Timed::WallClock::Settings::setTimezoneCellular()
{
  if(p->opcodes & WallOpcode::Op_Set_Zone_Mask)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Timezone_Cellular ;
}

void Maemo::Timed::WallClock::Settings::setTimezoneCellular(const QString &fbz)
{
  if(p->opcodes & WallOpcode::Op_Set_Zone_Mask)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Timezone_Cellular_Fbk ;
  p->zone = fbz ;
}

void Maemo::Timed::WallClock::Settings::setTimezoneManual(const QString &fbz)
{
  if(p->opcodes & WallOpcode::Op_Set_Zone_Mask)
    p->valid = false ;
  p->opcodes |= WallOpcode::Op_Set_Timezone_Manual ;
  p->zone = fbz ;
}

void Maemo::Timed::WallClock::Settings::setFlag24(bool flag24)
{
  if(p->opcodes & WallOpcode::Op_Set_Format_12_24_Mask)
    p->valid = false ;
  if(flag24)
    p->opcodes |= WallOpcode::Op_Set_24 ;
  else
    p->opcodes |= WallOpcode::Op_Set_12 ;
}

bool Maemo::Timed::WallClock::Settings::check() const
{
  return p->opcodes != 0 && p->valid ;
}

QString Maemo::Timed::WallClock::Settings::str() const
{
  if(p==NULL)
    return "{ NULL }" ;
  QString res ;
  QTextStream os(&res) ;
  os << "{ opcodes='" ;
  bool first = true ;
  using namespace WallOpcode ;
#define _x(x) if(p->opcodes&x){if(!first)os<<"+";first=false;os<<#x;}
  _x(Op_Set_Time_Nitz) ;
  _x(Op_Set_Time_Manual) ;
  _x(Op_Set_Time_Manual_Val) ;
  _x(Op_Set_Offset_Cellular) ;
  _x(Op_Set_Offset_Manual) ;
  _x(Op_Set_Offset_Manual_Val) ;
  _x(Op_Set_Timezone_Cellular) ;
  _x(Op_Set_Timezone_Cellular_Fbk) ;
  _x(Op_Set_Timezone_Manual) ;
  _x(Op_Set_24) ;
  _x(Op_Set_12) ;
#undef _x
  os << "', " ;
  os << " time_at_zero=" << p->time_at_zero.sec() << "," << p->time_at_zero.nano() ;
  os << "', " ;
  os << " offset=" << p->offset << ", " ;
  os << " zone='" << p->zone << "'" ;
  os << " valid=" << p->valid << "}" << flush ;
  return res ;
}

QVariant Maemo::Timed::WallClock::Settings::dbus_output(const char *pretty) const
{
  if(!p->valid)
    throw Maemo::Timed::Exception(pretty, "settings aren't valid") ;
  return QVariant::fromValue(*p) ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::WallClock::wall_settings_pimple_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out << x.opcodes << x.time_at_zero << x.offset << x.zone ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::WallClock::wall_settings_pimple_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  return in >> x.opcodes >> x.time_at_zero >> x.offset >> x.zone ;
}

register_qtdbus_metatype(Maemo::Timed::WallClock::wall_settings_pimple_t, 0) ;
#if 0
#include <QDebug>

static struct nanotime_initializer_t
{
  nanotime_initializer_t()
  {
    qDebug() << __PRETTY_FUNCTION__ ;
    register_qtdbus_metatype(nanotime_t) ;
  }
} nanotime_initializer ;
#else
// register_qtdbus_metatype(nanotime_t, 0) ; XXX already registered somewhere
#endif
