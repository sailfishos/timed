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
#define NEW_EXTERNAL_LOG 1

#if NEW_EXTERNAL_LOG
# include <qm/log>
#else

#ifndef LOG_H
#define LOG_H

#include <cstdio>

#include <memory>

/* Verbosity levels, the upper boundary could be set at compile time.
 *
 * 0 INTERNAL --- produced by failing log_assert(...)
 * 1 CRITICAL --- programm can continue, but some stuff is lost/can't be done
 * 2 ERROR --- incorrect input
 * 3 WARNING --- tolerable input, should be corrected
 * 4 INFO --- just some blah blah
 * 5 DEBUG --- verbose info
 *
 */

#define LOG_LEVEL_INTERNAL 0
#define LOG_LEVEL_CRITICAL 1
#define LOG_LEVEL_ERROR    2
#define LOG_LEVEL_WARNING  3
#define LOG_LEVEL_INFO     4
#define LOG_LEVEL_DEBUG    5

#ifndef LOG_MAX_LEVEL
#define LOG_MAX_LEVEL 5
#endif

#ifndef LOG_MAX_LOCATION
#define LOG_MAX_LOCATION ((1<<LOG_LEVEL_DEBUG)|(1<<LOG_LEVEL_INTERNAL))
#endif

#ifndef LOG_ASSERTION
#define LOG_ASSERTION 1
#endif

#if LOG_MAX_LEVEL<1 || LOG_MAX_LEVEL>5
#error LOG_MAX_LEVEL outside of [0..5]
#endif

struct log_t
{
  void message(int level, const char *fmt, ...) __attribute__((format(printf,3,4))) ;
  void message(int level, int line, const char *file, const char *func, const char *fmt, ...) __attribute__((format(printf,6,7))) ;
  void log_failed_assertion(const char *assertion, int line, const char *file, const char *func, const char *fmt, ...) __attribute__((format(printf,6,7))) ;
  int level(int new_level=-1) ;
  log_t(bool global, int new_level=-1, int location_mask=-1) ;
private:
  static bool use_syslog, use_stderr ;
  static FILE *fp ;
  static const char *prg_name ;
  static const char *level_name(int level) ;
  static int syslog_level_id(int level) ;
  void log_generic(int level, bool show_level, const char *fmt, ...) __attribute__((format(printf,4,5))) ;
  void vlog_generic(int level, bool show_level, const char *fmt, va_list args) ;
  void log_location(int level, bool message_follows, int line, const char *file, const char *func) ;

  int verbosity_level ;
  int location_mask ;
  log_t *prev ;
  friend void ::log_init(const char *name, const char *path, bool sys, bool std) ;
} ;

extern log_t *current_log ;
void log_init(const char *name, const char *path, bool sys, bool std) ;

#define LOG_LEVEL(args) std::auto_ptr<log_t> current_log_ptr(new log_t(true,args))

#define LOG_LOCAL(args) std::auto_ptr<log_t> current_log(new log_t(false,args))

#define LOG_LOCATION __LINE__,__FILE__,__PRETTY_FUNCTION__

#if LOG_MAX_LEVEL >= LOG_LEVEL_CRITICAL
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_CRITICAL)
#  define log_critical(...) current_log->message(LOG_LEVEL_CRITICAL, LOG_LOCATION, "\x01"__VA_ARGS__)
# else
#  define log_critical(...) current_log->message(LOG_LEVEL_CRITICAL, "\x01"__VA_ARGS__)
# endif
#else
# define log_critical(...) (void)(0)
#endif

#if LOG_MAX_LEVEL >= LOG_LEVEL_ERROR
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_ERROR)
#  define log_error(...) current_log->message(LOG_LEVEL_ERROR, LOG_LOCATION, "\x01"__VA_ARGS__)
# else
#  define log_error(...) current_log->message(LOG_LEVEL_ERROR, "\x01"__VA_ARGS__)
# endif
#else
# define log_error(...) (void)(0)
#endif

#if LOG_MAX_LEVEL >= LOG_LEVEL_WARNING
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_WARNING)
#  define log_warning(...) current_log->message(LOG_LEVEL_WARNING, LOG_LOCATION, "\x01"__VA_ARGS__)
# else
#  define log_warning(...) current_log->message(LOG_LEVEL_WARNING, "\x01"__VA_ARGS__)
# endif
#else
# define log_warning(...) (void)(0)
#endif

#if LOG_MAX_LEVEL >= LOG_LEVEL_INFO
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_INFO)
#  define log_info(...) current_log->message(LOG_LEVEL_INFO, LOG_LOCATION, "\x01"__VA_ARGS__)
# else
#  define log_info(...) current_log->message(LOG_LEVEL_INFO, "\x01"__VA_ARGS__)
# endif
#else
# define log_info(...) (void)(0)
#endif

#if LOG_MAX_LEVEL >= LOG_LEVEL_DEBUG
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_DEBUG)
#  define log_debug(...) current_log->message(LOG_LEVEL_DEBUG, LOG_LOCATION, "\x01"__VA_ARGS__)
# else
#  define log_debug(...) current_log->message(LOG_LEVEL_DEBUG, "\x01"__VA_ARGS__)
# endif
#else
# define log_debug(...) (void)(0)
#endif

#if LOG_ASSERTION
# define log_assert(x, ...) do { if(!(x)) current_log->log_failed_assertion(#x, LOG_LOCATION, "\x01"__VA_ARGS__) ; } while(0)
#else
# define log_assert(...) (void)(0)
#endif

#endif // LOG_H

#endif // NEW_EXTERNAL_LOG
