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
#include <sys/time.h>
#include <time.h>

#include "wall-declarations.h"
#include "qmacro.h"
#include "../common/log.h"
#include "wall-info.h"

register_qtdbus_metatype(Maemo::Timed::WallClock::Info, 0) ;

Maemo::Timed::WallClock::Info::Info() // invalid object
{
  p = new wall_info_pimple_t ;
}

Maemo::Timed::WallClock::Info::~Info()
{
  if(p)
    delete p ;
}

Maemo::Timed::WallClock::Info::Info(wall_info_pimple_t *pp)
{
  p = pp ;
}

Maemo::Timed::WallClock::Info::Info(const Info &x)
{
  p = NULL ;
  *this = x ;
}

const Maemo::Timed::WallClock::Info & Maemo::Timed::WallClock::Info::operator=(const Info &x)
{
  if (this==&x)
    return *this ;
  if(p)
    delete p ;
  if(x.p)
  {
    p = new wall_info_pimple_t ;
    *p = *x.p ;
  }
  else
    p = NULL ;
  return *this ;
}

bool Maemo::Timed::WallClock::Info::flagTimeNitz() const
{
  return p->flag_time_nitz ;
}

bool Maemo::Timed::WallClock::Info::flagLocalCellular() const
{
  return p->flag_local_cellular ;
}

bool Maemo::Timed::WallClock::Info::flagAutoDst() const
{
  return p->flag_auto_dst ;
}

bool Maemo::Timed::WallClock::Info::flagFormat24() const
{
  return p->flag_format_24 ;
}

time_t Maemo::Timed::WallClock::Info::utc() const
{
  int src = p->data_sources[wall_info_pimple_t::CL] ;
  enum Maemo::Timed::WallClock::UtcSource esrc = UtcSource(src) ;
  return utc(esrc) ;
}

QString Maemo::Timed::WallClock::Info::etcLocaltime() const
{
  return p->localtime_symlink ;
}

QString Maemo::Timed::WallClock::Info::humanReadableTz() const
{
  return p->human_readable_tz ;
}

int Maemo::Timed::WallClock::Info::secondsEastOfGmt() const
{
  return p->seconds_east_of_gmt ;
}

QString Maemo::Timed::WallClock::Info::tzAbbreviation() const
{
  return p->abbreviation ;
}

enum Maemo::Timed::WallClock::UtcSource Maemo::Timed::WallClock::Info::utcSource() const
{
  return (UtcSource) p->data_sources[wall_info_pimple_t::CL] ;
}

enum Maemo::Timed::WallClock::TimezoneSource Maemo::Timed::WallClock::Info::timezoneSource() const
{
  return (TimezoneSource) p->data_sources[wall_info_pimple_t::ZO] ;
}

enum Maemo::Timed::WallClock::OffsetSource Maemo::Timed::WallClock::Info::offsetSource() const
{
  return (OffsetSource) p->data_sources[wall_info_pimple_t::OF] ;
}

bool Maemo::Timed::WallClock::Info::utcAvailable(enum UtcSource s) const
{
  const nanotime_t &at_zero = p->clocks[(int)s] ;
  return ! at_zero.is_invalid() ;
}

bool Maemo::Timed::WallClock::Info::timezoneAvailable(enum TimezoneSource s) const
{
  const QString &tz = p->zones[(int)s] ;
  return ! tz.isEmpty() ;
}

bool Maemo::Timed::WallClock::Info::offsetAvailable(enum OffsetSource s) const
{
  return p->offsets[(int)s] != -1 ;
}

time_t Maemo::Timed::WallClock::Info::utc(enum UtcSource s) const
{
  const nanotime_t &at_zero = p->clocks[(int)s] ;
  if(at_zero.is_invalid())
    return time_t(-1) ;
  nanotime_t now = at_zero + nanotime_t::monotonic_now() ;
  return now.to_time_t() ;
}

QString Maemo::Timed::WallClock::Info::timezone(enum TimezoneSource s) const
{
  return p->zones[(int)s] ;
}

int Maemo::Timed::WallClock::Info::offset(enum OffsetSource s) const
{
  return p->offsets[(int)s] ;
}

time_t Maemo::Timed::WallClock::Info::clockDiff() const
{
  return p->systime_diff.to_time_t() ;
}

struct timespec Maemo::Timed::WallClock::Info::clockDiffNano() const
{
  return p->systime_diff.to_timespec() ;
}

bool Maemo::Timed::WallClock::Info::nitzSupported() const
{
  return p->nitz_supported ;
}

QString Maemo::Timed::WallClock::Info::defaultTimezone() const
{
  return p->default_timezone ;
}

QString Maemo::Timed::WallClock::Info::str() const
{
  if(p==NULL)
    return "{ NULL }" ;
  QString res ;
  QTextStream os(&res) ;
  os << "{" ;
#define _x(x) #x<<"="<<(p->flag_##x?"true":"false")<<", "
  os << _x(time_nitz) << _x(local_cellular) << _x(auto_dst) << _x(format_24) ;
#undef _x
#define _x(x) #x<<"="<<p->x<<", "
  os << _x(clocks) << _x(zones) << _x(offsets) << _x(data_sources) ;
  os << _x(localtime_symlink) << _x(human_readable_tz) ;
#undef _x
  os << "seconds_east_of_gmt=" << p->seconds_east_of_gmt << "=" ;
  if(p->seconds_east_of_gmt%3600)
    os << (double)p->seconds_east_of_gmt / 3600.0 ;
  else
    os << p->seconds_east_of_gmt / 3600 ;
  os << "h" ;
  os << " (" << p->abbreviation << ")" ;
  os << " clock_back=" << p->systime_diff ;
  os << " nitz_supported=" << p->nitz_supported ;
  os << " default_timezone=" << p->default_timezone.toStdString().c_str() ;
  os << "}" << flush ;
  return res ;
}


QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::WallClock::Info &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out
    << x.p->flag_time_nitz << x.p->flag_local_cellular << x.p->flag_auto_dst << x.p->flag_format_24
    << x.p->clocks << x.p->zones << x.p->offsets
    << x.p->data_sources << x.p->systime_diff
    << x.p->localtime_symlink << x.p->human_readable_tz << x.p->seconds_east_of_gmt << x.p->abbreviation
    << x.p->nitz_supported << x.p->default_timezone ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::WallClock::Info &x)
{
  using Maemo::Timed::WallClock::wall_info_pimple_t ;
  qdbusargument_structrure_wrapper_const i(in) ;
  in
    >> x.p->flag_time_nitz >> x.p->flag_local_cellular >> x.p->flag_auto_dst >> x.p->flag_format_24
    >> x.p->clocks >> x.p->zones >> x.p->offsets
    >> x.p->data_sources >> x.p->systime_diff
    >> x.p->localtime_symlink >> x.p->human_readable_tz >> x.p->seconds_east_of_gmt >> x.p->abbreviation
    >> x.p->nitz_supported >> x.p->default_timezone ;

  bool rubbish = false ;

  if(x.p->data_sources.size()!=wall_info_pimple_t::N_DATA)
    x.p->data_sources.resize(wall_info_pimple_t::N_DATA), rubbish=true ;

#define _check(data,N_data,DT) \
  if(x.p->data.size() != wall_info_pimple_t::N_data) \
    x.p->data.resize(wall_info_pimple_t::N_data), rubbish = true ; \
  if(x.p->data_sources[wall_info_pimple_t::DT]<0 || x.p->data_sources[wall_info_pimple_t::DT]>=wall_info_pimple_t::N_data) \
    x.p->data_sources[wall_info_pimple_t::DT] = 0, rubbish = true ;

  _check(clocks, N_CLOCKS, CL) ;
  _check(zones,  N_ZONES,  ZO) ;
  _check(offsets,N_OFFSETS,OF) ;
#undef _check

  if(rubbish)
    log_error("not valid input submitted via dbus (Maemo::Timed::WallClock::Info)") ;

  return in ;
}
