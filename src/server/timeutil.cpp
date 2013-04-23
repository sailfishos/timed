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
#include <errno.h>
#include <string>
#include <stdlib.h>
using namespace std ;

#include "../common/log.h"

#include "event.h"
#include "timeutil.h"
#include "wrappers.h"

ticker_t now()
{
  return ticker_t(time(NULL)) ;
}

ticker_t ticker_align(ticker_t start, int step, ticker_t target)
{
  // log_debug("start=%ld, step=%d, target=%ld", start.value(), step, target.value()) ;
  log_assert(step>0) ;
  log_assert(start.is_valid()) ;
  log_assert(target.is_valid()) ;

  ticker_t result ;

  if(start<=target)
  {
    int delta = target - start ;
    int x = delta/step + 1 ;
    result = start + x*step ;
    // log_debug("delta=%d, x=%d, result=%ld", delta, x, result.value()) ;
  }
  else
  {
    int delta = start - target ;
    int x = delta/step - (delta%step==0) ;
    result = start + (-x)*step ;
    // log_debug("delta=%d, x=%d, result=%ld", delta, x, result.value()) ;
  }
  log_assert((start-result) % step == 0) ;
  log_assert(target<result) ;
  log_assert(result-target<=step) ; // 1 <= result-target <= step

  return result ;
}

void switch_timezone::constructor(const char *new_tz)
{
  bool new_is_empty = new_tz==NULL || *new_tz=='\0' ;

  const char *env = getenv("TZ") ;
  if(env==NULL)
    old_tz = NULL ;
  else
  {
    old_tz = strdup(env) ;
    log_assert(old_tz!=NULL) ;
  }

  int res = new_is_empty ? unsetenv("TZ") : setenv("TZ", new_tz, true) ;

  if(res<0)
    log_critical("Can't change TZ environment: %s", strerror(errno)) ;

  tzset() ;
}

switch_timezone::switch_timezone(const char *value)
{
  constructor(value) ;
}

switch_timezone::switch_timezone(const string &s)
{
  constructor(s.c_str()) ;
}

switch_timezone::~switch_timezone()
{
  int res = old_tz ? setenv("TZ", old_tz, true) : unsetenv("TZ") ;

  if(res<0)
    log_critical("Can't change TZ environment: %s", strerror(errno)) ;

  if(old_tz)
    free(old_tz) ;

  tzset() ;
}

ticker_t mktime_oversea(struct tm *t, const string &s)
{
  switch_timezone tmp(s) ;
  return ticker_t(mktime(t)) ;
}

ticker_t mktime_local(struct tm *t)
{
  return ticker_t(mktime(t)) ;
}

bool zone_info(int *seconds_east_of_gmt, const string &zone, string *abbreviation, ticker_t x)
{
  string tz ;
  if(!zone.empty())
    tz = (string) ":" + zone ;
  switch_timezone tmp(tz) ;
  time_t t = x.is_valid() ? x.value() : time(NULL) ;
  struct tm tm ;
  if(localtime_r(&t, &tm)==0)
    return false ;
  if(seconds_east_of_gmt)
    *seconds_east_of_gmt = tm.tm_gmtoff ;
  if(abbreviation)
    *abbreviation = tm.tm_zone ;
  // log_debug("tm_zone='%s'", tm.tm_zone) ;
  return true ;
}

string broken_down_t::str() const
{
  const unsigned buflen = 1024 ;
  char buf[buflen+1] ;
  unsigned res = snprintf(buf, buflen, "%04d-%02d-%02d %02d:%02d", year, month, day, hour, minute) ;
  if(res>buflen)
    log_error("output was truncated, need %d bytes", res) ;
  return string(buf) ;
}

string tm_str(const struct tm *tm)
{
  ostringstream os ;
#define X(x) #x"=" << tm->tm_##x
#define Y(x) X(x) << ", "
  os << "{" << Y(year) Y(mon) Y(mday) Y(hour) Y(min) Y(sec) Y(wday) Y(yday) X(isdst) << "}" ;
#undef X
#undef Y
  return os.str() ;
}

iodata::record *broken_down_t::save()
{
  iodata::record *r = new iodata::record ;
  r->add("year", year) ;
  r->add("month", month) ;
  r->add("day", day) ;
  r->add("hour", hour) ;
  r->add("minute", minute) ;
  return r ;
}

void broken_down_t::load(const iodata::record *r)
{
  year = r->get("year")->value() ;
  month = r->get("month")->value() ;
  day = r->get("day")->value() ;
  hour = r->get("hour")->value() ;
  minute = r->get("minute")->value() ;
}

void broken_down_t::increment_day()
{
  ++ day ;
  if(day<=month_length(year,month))
    return ;
  day = 1 ;
  ++ month ;
  if(month<=12)
    return ;
  month = 1 ;
  ++ year ;
  if(year < YEARX)
    return ;
  year = month = day = 0 ;
}

struct tm *broken_down_t::to_struct_tm(struct tm *dest /* = NULL */) const
{
  if(dest==NULL)
    dest = new tm ;
  dest->tm_sec = dest->tm_wday = dest->tm_yday = 0 ;
  dest->tm_isdst = -1 ;
  dest->tm_year = year - 1900 ;
  dest->tm_mon = month - 1 ;
  dest->tm_mday = day ;
  dest->tm_hour = hour ;
  dest->tm_min = minute ;
  return dest ;
}

void broken_down_t::from_struct_tm(const struct tm *tm)
{
  year = tm->tm_year + 1900 ;
  month = tm->tm_mon + 1 ;
  day = tm->tm_mday ;
  hour = tm->tm_hour ;
  minute = tm->tm_min ;
#if 1
  // should we do it here ?..
  if(!is_valid())
    year = month = day = hour = minute = 0 ;
#endif
}

bool broken_down_t::same_struct_tm(const struct tm *tm) const
{
#if 1
  // should we do it here ?..
  if(!is_valid())
    return false ;
#endif
  return
    (int)year == tm->tm_year + 1900 &&
    (int)month == tm->tm_mon + 1 &&
    (int)day == tm->tm_mday &&
    (int)hour == tm->tm_hour &&
    (int)minute == tm->tm_min ;
}

bool broken_down_t::find_a_good_day(const recurrence_pattern_t *p, int &wday, bool &today, unsigned max_year)
{
  log_debug("wday=%d, today_is_ok=%s, max_year=%d", wday, today?"yes":"no", max_year) ;
#define _next do { today = false ; increment_day() ; if(++wday==7) wday = 0 ; } while(0)
  log_assert(0<=wday && wday<7) ;
  // unsigned y = year ;
  if(!today)
    _next ;
  while(year<=max_year && is_valid())
  {
    log_debug("year=%d, month=%d, day=%d, wday=%d", year, month, day, wday) ;
    bool month_is_ok = p->mons & (1<<(month-1)) ;
    if(month_is_ok)
    {
      bool mday_is_ok = p->mday & (1u<<day) ;
      if(!mday_is_ok) // last hope: last day of month
        mday_is_ok = (day==month_length(year, month) && (p->mday & 1)) ;
      if(mday_is_ok && (p->wday & (1<<wday)))
        return true ;
    }
    else // jump to the last day of month
    {
      wday -= day ;
      wday += day = month_length(year,month) ;
      wday %= 7 ;
    }
    _next ;
  }
  return false ;
#undef _next
}

void broken_down_t::increment_min(unsigned int amount)
{
  const unsigned int D = 24*60 ;
  log_assert(amount<=D) ;
  unsigned int now = 60*hour + minute + amount ;
  if(now>=D)
  {
    now -= D ;
    increment_day() ;
  }
  hour = now / 60 ;
  minute = now % 60 ;
}

bool broken_down_t::is_a_regular_day() const
{
  broken_down_t first = *this, last = *this ;
  first.hour = first.minute = 0 ;
  last.hour = 23, last.minute = 59 ;
  struct tm tm_first, tm_last ;
  first.to_struct_tm(&tm_first) ;
  last.to_struct_tm(&tm_last) ;
  time_t first_time = mktime(&tm_first), last_time = mktime(&tm_last) ;
  if (first_time < 0 or last_time < 0)
    return false ;
  if (last_time - first_time != 60*(23*60+59))
    return false ;
  if (tm_first.tm_isdst != tm_last.tm_isdst)
    return false ;
  if (tm_first.tm_gmtoff != tm_last.tm_gmtoff)
    return false ;
  if (strcmp(tm_first.tm_zone, tm_last.tm_zone)!=0)
    return false ;
  return true ;
}

#define _set(res, mask, shift) res |= ((x&(mask))!=0)<<(shift)
static inline int log2_64(uint64_t x)
{
  int res = 0 ;
  _set(res, 0xAAAAAAAAAAAAAAAAll, 0) ;
  _set(res, 0xCCCCCCCCCCCCCCCCll, 1) ;
  _set(res, 0xF0F0F0F0F0F0F0F0ll, 2) ;
  _set(res, 0xFF00FF00FF00FF00ll, 3) ;
  _set(res, 0xFFFF0000FFFF0000ll, 4) ;
  _set(res, 0xFFFFFFFF00000000ll, 5) ;
  return res ;
}
static inline int log2_32(uint32_t x)
{
  int res = 0 ;
  _set(res, 0xAAAAAAAA, 0) ;
  _set(res, 0xCCCCCCCC, 1) ;
  _set(res, 0xF0F0F0F0, 2) ;
  _set(res, 0xFF00FF00, 3) ;
  _set(res, 0xFFFF0000, 4) ;
  // log_debug("x=%08X, log2=%d", x, res) ;
  return res ;
}
#undef _set


bool broken_down_t::find_a_good_minute(const recurrence_pattern_t *p)
{
  log_assert(p->mins && p->hour) ;
  log_assert(p->mins < (uint64_t)1<<60) ;
  log_assert(p->hour < (uint32_t)1<<24) ;
  uint64_t this_hour_mask = ~ ( ((uint64_t)1<<(minute)) - 1 ) ;
  if((1 << hour) & p->hour)
    if(uint64_t x = p->mins & this_hour_mask)
    {
#define _remove_upper_bits(x) (((((x)-1)^(x)) + 1) >> 1)
      x = _remove_upper_bits(x) ;
      minute = log2_64(x) ;
      return true ;
    }
  if(hour>=23)
    return false ;
  uint32_t this_day_mask = ~ ( ((uint32_t)2<<(hour)) - 1 ) ;
  if(uint32_t x = p->hour & this_day_mask)
  {
    x = _remove_upper_bits(x) ;
    hour = log2_32(x) ;
    uint64_t y = p->mins ;
    y = _remove_upper_bits(y) ;
#undef _remove_upper_bits
    minute = log2_64(y) ;
    return true ;
  }
  else
    return false ;
}

bool broken_down_t::find_a_good_minute_with_increment(const recurrence_pattern_t *p, bool increment_flag)
{
  if (increment_flag)
  {
    if (hour==23 and minute==59)
      return false ;
    increment_min(1) ;
  }
  return find_a_good_minute(p) ;
}

time_t broken_down_t::mktime_strict(int dst /* = -1 */) const
{
  struct tm *tm = to_struct_tm() ;
  if (tm==NULL)
    return (time_t) -1 ;
  tm->tm_isdst = dst ;
  time_t t = mktime(tm) ;
  if (not same_struct_tm(tm))
    t = (time_t) -1 ;
  if (dst != -1 and dst != tm->tm_isdst)
    t = (time_t) -1 ;
  delete tm ;
  return t ;
}

void broken_down_t::from_time_t(const ticker_t &ticker, int *wday)
{
  time_t time = ticker.value() ;
  struct tm tm ;
  localtime_r(&time, &tm) ;
  if(wday)
    *wday = tm.tm_wday ;
  from_struct_tm(&tm) ;
}

string dst_signature(time_t t)
{
  struct tm tm ;
  if(localtime_r(&t, &tm)==NULL)
    return "" ;
  ostringstream os ;
  os << ((tm.tm_isdst<0) ? "n/a;" : tm.tm_isdst ? "dst;" : "nodst;") ;
  os << tm.tm_gmtoff << ";" ;
  os << tm.tm_zone ;
  return os.str() ;
}
