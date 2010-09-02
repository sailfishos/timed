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
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#include <timed/qmacro.h>

#include "nanotime.h"

nanotime_t nanotime_t::systime_at_zero() // TODO: rename it to start_up_time()
{
  nanotime_t s = systime_now() ;
  nanotime_t m = monotonic_now() ;
  if(s.is_invalid() || m.is_invalid())
    return nanotime_t() ; // invalid
  return s-m ;
}

nanotime_t nanotime_t::systime_now()
{
  struct timeval tv ;
  int res = gettimeofday(&tv, NULL) ;
  if(res<0)
    return nanotime_t() ; // invalid
  nanotime_t t(tv.tv_sec, tv.tv_usec*1000) ; // micro to nano
  t.fix_overflow() ;
  return t ;
}

nanotime_t nanotime_t::monotonic_now()
{
  struct timespec tv ;
  int res = clock_gettime(CLOCK_MONOTONIC, &tv) ;
  nanotime_t t = nanotime_t::from_timespec(tv) ;
  if(res<0)
    t.invalidate() ;
  else
    t.fix_overflow() ;
  return t ;
}

int nanotime_t::set_systime(const nanotime_t &t)
{
  if(t.is_invalid() || !t.is_normalized())
  {
    errno = EINVAL ;
    return -1 ;
  }
  int micro = t.nano() / 1000 ;
  bool next_micro = 500 <= t.nano() - 1000*micro ;
  struct timeval tv = { t.sec(), micro } ;
  if(next_micro && ++tv.tv_usec==NANO/1000)
    ++tv.tv_sec, tv.tv_usec = 0 ;
  int res = settimeofday(&tv, NULL) ;
  return res ;
}

int nanotime_t::set_systime_at_zero(const nanotime_t &t)
{
  if(t.is_invalid() || !t.is_normalized())
  {
    errno = EINVAL ;
    return -1 ;
  }
  nanotime_t m=monotonic_now() ;
  if(m.is_invalid())
    return -1 ;
  return set_systime(t+m) ;
}


QDBusArgument &operator<<(QDBusArgument &out, const nanotime_t &x)
{
  qdbusargument_structrure_wrapper o(out) ;
  return out << x.sec() << x.nano() ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, nanotime_t &x)
{
  qdbusargument_structrure_wrapper_const i(in) ;
  uint32_t ns ;
  int32_t s ;
  in >> s >> ns ;
  x = nanotime_t(s, ns) ;
  return in ;
}

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
register_qtdbus_metatype(nanotime_t, 0) ;
#endif
