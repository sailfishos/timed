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
#ifndef NEW_EXTERNAL_LOG

#define _BSD_SOURCE

#include <cstdarg>
#include <cassert>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <ctime>

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <syslog.h>

#include <algorithm>
using namespace std ;

#include "log.h"

bool log_t::use_syslog=false, log_t::use_stderr=false ;
FILE *log_t::fp=NULL ;
const char *log_t::prg_name = "<anonymous>" ;

log_t *current_log = NULL ;

void log_init(const char *name, const char *path, bool sys, bool std)
{
  static bool init_called = false ;
  if(init_called)
    log_critical("log_init() was already called earlier") ;
  init_called = true ;
  log_t::use_syslog = sys, log_t::use_stderr = std, log_t::prg_name = name ;
  if(path!=NULL && (log_t::fp = fopen(path, "aw"))==NULL)
  {
    log_t::use_stderr = true ;
    log_critical("Can't write to '%s': %s", path, strerror(errno)) ;
    if(std==false)
      log_warning("Stderr logging enabled") ;
  }
  if(sys)
  {
    openlog(log_t::prg_name, LOG_PID|LOG_NDELAY /*LOG_CONS|LOG_PERROR*/, LOG_DAEMON) ;
  }
  current_log = new log_t(true, LOG_MAX_LEVEL, LOG_MAX_LOCATION) ;
}

log_t::log_t(bool global, int new_level, int new_mask)
{
  if(new_level<0)
  {
    log_assert(global) ;
    verbosity_level = current_log->verbosity_level ;
  }
  else
    verbosity_level = new_level ;
  if(new_mask<0)
    location_mask = global ? current_log->location_mask : LOG_MAX_LOCATION ;
  else
    location_mask = new_mask ;
  if(global)
  {
    prev = current_log ;
    current_log = this ;
  }
  else
    prev = NULL ;
}

const char *log_t::level_name(int level)
{
  static const char *names[] =
  {
    "INTERNAL ERROR", "CRITICAL ERROR", "ERROR", "WARNING", "INFO", "DEBUG"
  } ;
  assert(0<=level) ;
  assert((unsigned)level<sizeof(names)/sizeof(*names)) ;
  return names[level] ;
}

int log_t::syslog_level_id(int level)
{
  static int syslog_names[] =
  {
    LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG
  } ;
  assert(0<=level) ;
  assert((unsigned)level<sizeof(syslog_names)/sizeof(*syslog_names)) ;
  return syslog_names[level] ;
}

void log_t::vlog_generic(int level, bool show_level, const char *fmt, va_list args)
{
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;
  assert(level<=verbosity_level) ;

  if(fmt[0]=='\x01')
    ++fmt ;

  const char *str_level1 = show_level ? level_name(level) : "" ;
  const char *str_level2 = show_level ? ": " : "" ;
  bool has_newline = fmt[strlen(fmt)-1]=='\n' ;

  if(use_stderr)
  {
    fprintf(stderr, "%s[%d]: ", prg_name, getpid()) ;
    fprintf(stderr, "%s%s", str_level1, str_level2) ;
    vfprintf(stderr, fmt, args) ;
    if(!has_newline)
      fprintf(stderr, "\n") ;
  }

  if(fp)
  {
    const int time_length = 32 ;
    char buffer[time_length+1] ;

    struct timeval tv ;
    gettimeofday(&tv, NULL) ;
    struct timespec nano ;
    clock_gettime(CLOCK_MONOTONIC, &nano) ;

    time_t t = tv.tv_sec ;
    struct tm time_tm ;
    localtime_r(&t, &time_tm) ;
    strftime(buffer, time_length, "%F %T", &time_tm) ;

    fprintf(fp, "[%ld.%03ld %s.%03ld] [%s(%d)] %s%s", nano.tv_sec, nano.tv_nsec/1000000, buffer, tv.tv_usec/1000, prg_name, getpid(), str_level1, str_level2) ;
    vfprintf(fp, fmt, args) ;
    if(!has_newline)
      fprintf(fp, "\n") ;
    fflush(fp) ;
  }

  if(use_syslog)
  {
    vsyslog(LOG_DAEMON|syslog_level_id(level), fmt, args) ;
  }
}

void log_t::log_location(int level, bool message_follows, int line, const char *file, const char *func)
{
  if(message_follows)
    log_generic(level, false, "%s at %s:%d in %s:", level_name(level), file, line, func) ;
  else
    log_generic(level, true, "%s:%d in %s.", file, line, func) ;
}

void log_t::log_generic(int level, bool show_level, const char *fmt, ...)
{
  va_list args ;
  va_start(args, fmt) ;
  vlog_generic(level, show_level, fmt, args) ;
  va_end(args) ;
}

void log_t::message(int level, const char *fmt, ...)
{
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;
  if(level>verbosity_level)
    return ;

  va_list args ;
  va_start(args, fmt) ;
  vlog_generic(level, true, fmt, args) ;
  va_end(args) ;
}

void log_t::message(int level, int line, const char *file, const char *func, const char *fmt, ...)
{
  assert((1<<level) & LOG_MAX_LOCATION) ;
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;
  if(level>verbosity_level)
    return ;

  bool have_a_message = fmt[1]!='\0', location_shown = false ;

  if((1<<level)&location_mask) // show location
  {
    log_location(level, have_a_message, line, file, func) ;
    location_shown = true ;
  }

  if(have_a_message)
  {
    va_list args ;
    va_start(args, fmt) ;
    vlog_generic(level, !location_shown, fmt, args) ;
    va_end(args) ;
  }
}

void log_t::log_failed_assertion(const char *assertion, int line, const char *file, const char *func, const char *fmt, ...)
{
  log_location(LOG_LEVEL_INTERNAL, true, line, file, func) ;
  bool message_follows = fmt[1]!='\0' ;
  log_generic(LOG_LEVEL_INTERNAL, false, "Assertion failed: %s%s.", assertion, message_follows?". Detailed info follows":"") ;
  if(message_follows)
  {
    va_list args ;
    va_start(args, fmt) ;
    vlog_generic(LOG_LEVEL_INTERNAL, true, fmt, args) ;
    va_end(args) ;
  }
  abort() ; do { sleep(1) ; } while (true) ;
}

#if 0
int main()
{
  log_init("log_test", "log.log", true, true) ;
  log_info("info, 2x2=%d", 4) ;
  log_debug("info, 2x2=%d", 5) ;
  log_debug() ;
  log_assert(false, "information: %d", 239*239) ;
  return 0 ;
}
#endif


#endif // NEW_EXTERNAL_LOG
