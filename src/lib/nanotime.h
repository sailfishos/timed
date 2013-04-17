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
#ifndef NANOTIME_H
#define NANOTIME_H

#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include <string>

#include <QTextStream>

#include "qmacro.h"

struct nanotime_t
{
  int32_t s ;
  uint32_t ns ;
  static const uint32_t NANO = 1000000000 ;
  static const int32_t EPOCH = 2145916800 ; // Fri Jan  1 00:00:00 UTC 2038
  nanotime_t() { invalidate() ; /*static_assert(sizeof(tv_sec)==4) ; static_assert(sizeof(tv_nsec)==4) ; */}
  nanotime_t(int32_t sec) { s=sec, ns=0 ; }
  nanotime_t(int32_t sec, uint32_t nsec) { s=sec, ns=nsec ; }
  nanotime_t(const nanotime_t &x) { ns=x.ns, s=x.s ; }
  const nanotime_t &operator= (const nanotime_t &x) { ns=x.ns, s=x.s ; return *this ;}
  void set(int32_t sec) { s=sec, ns=0 ; }
  void set(int32_t sec, uint32_t nsec) { s=sec, ns=nsec ; }
  void fix_overflow() { while(!is_normalized()) ns-=NANO, ++s ; }
  void fix_underflow() { while(!is_normalized()) ns+=NANO, --s ; }
  const nanotime_t &operator+=(const nanotime_t &x) { s+=x.s, ns+=x.ns ; fix_overflow() ; return *this ; }
  const nanotime_t &operator-=(const nanotime_t &x) { s-=x.s, ns-=x.ns ; fix_underflow() ; return *this ; }
  nanotime_t operator+(const nanotime_t &x) const { nanotime_t y=*this ; return y+=x ; }
  nanotime_t operator-(const nanotime_t &x) const { nanotime_t y=*this ; return y-=x ; }
  nanotime_t operator-() const { return ns ? nanotime_t(-s-1,NANO-ns) : nanotime_t(-s,0) ; }
  nanotime_t div2() const ;
  bool operator<(int32_t x) const { return sec() < x ; }
  bool operator<(const nanotime_t x) const { return sec()<x.sec() or (sec()==x.sec() and nano()<x.nano()) ; }
  bool operator>(const nanotime_t x) const { return sec()>x.sec() or (sec()==x.sec() and nano()>x.nano()) ; }
  bool operator<=(const nanotime_t x) const { return not operator > (x) ; }
  bool operator>=(const nanotime_t x) const { return not operator < (x) ; }

  bool is_normalized() const { return NANO > nano() ; }
  void invalidate() { s=~0, ns=~0 ; }
  bool is_invalid() const { return !~(s&ns) || sec()>EPOCH ; }
  bool is_zero() const { return !(s|ns) ; }
  int32_t sec() const { return s ; }
  uint32_t nano() const { return ns ; }

  time_t to_time_t() const { return is_invalid() ? -1 : sec()+(NANO/2<=nano()) ; }
  struct timespec to_timespec() const { struct timespec tv ; tv.tv_sec=s, tv.tv_nsec=ns; return tv ; }

  static nanotime_t from_time_t(time_t x) { return nanotime_t(x,0) ; }
  static nanotime_t from_timespec(const struct timespec &tv) { return nanotime_t(tv.tv_sec, tv.tv_nsec) ; }
  static nanotime_t systime_at_zero() ;
  static nanotime_t systime_now() ;
  static nanotime_t monotonic_now() ;
  static int set_systime(const nanotime_t &) ;
  static int set_systime_at_zero(const nanotime_t &) ;

  std::string str() const ;
} ;

inline QTextStream& operator<<(QTextStream &os, const nanotime_t &x)
{
  return os << x.str().c_str() ;
}

inline std::string nanotime_t::str() const
{
  if(is_invalid())
    return "N/A" ;

  char buf[40] ;
  if(sec()<0)
  {
    int32_t second = sec() + (nano() > 0) ;
    sprintf(buf, "-%d.%09d", -second, nano()>0 ? NANO-nano() : 0) ;
  }
  else
    sprintf(buf, "%d.%09d", sec(), nano()) ;

  return buf ;
}

// to be moved somewrere sometime
template <typename T>
QTextStream& operator<<(QTextStream &os, const QVector<T> &x)
{
  os << "[" ;
  for(int i=0; i<x.size(); ++i)
    os << (i?", ":"") << x[i] ;
  os << "]" ;
  return os ;
}

declare_qtdbus_io (nanotime_t) ;

#endif
