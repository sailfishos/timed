/* ------------------------------------------------------------------------- *
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QMap>
#include <QString>

#include "../../src/lib/interface.h"
#include "../../src/lib/event-declarations.h"

#if 0
// we should be able to use const references, but ...
# define CONST const
#else
// ... in some cases we are missing compatible methods
# define CONST
#endif

#define sz(qs) (qs).toUtf8().constData()

#define numof(a) (sizeof(a)/sizeof*(a))


#if 0
# define debugf(FMT,ARGS...) printf("DEBUG: "FMT, ## ARGS)
#else
# define debugf(FMT,ARGS...) do { } while(0)
#endif

/* ------------------------------------------------------------------------- *
 * String helpers
 * ------------------------------------------------------------------------- */

/** Strings that qualify as bool true */
static const char * const bool_y[] =
{
  "true",  "yes", "on", "y", "t",  0
};

/** Strings that qualify as bool false */
static const char * const bool_n[] =
{
  "false", "no",  "off","n", "f",  0
};

/** Non-localized weekday names in struct tm compatible order */
static const char * const weekday_name[7] =
{
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

/** Non-localized month names in struct tm compatible order */
static const char * const month_name[12] =
{
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

/** Boolean to string helper */
static const char *repr_bool(bool val)
{
  return val ? *bool_y : *bool_n;
}

/** String to cookie helper */
static uint parse_cookie(const char *str)
{
  return strtoul(str, 0, 0);
}

/** String to integer number helper */
static int parse_int(const char *str)
{
  return strtol(str, 0, 0);
}

/** String to time_t helper */
static time_t parse_secs(const char *str)
{
  return strtol(str, 0, 0);
}

/** String to bool helper */
static bool parse_bool(const char *str)
{
  for( int i = 0; bool_y[i]; ++i )
  {
    if( !strcmp(bool_y[i], str) ) return true;
  }
  for( int i = 0; bool_n[i]; ++i )
  {
    if( !strcmp(bool_n[i], str) ) return false;
  }
  return strtol(str, 0, 0) != 0;
}

/** String to month number helper, compatible with struct tm indexing */
static int parse_month(const char *str)
{
  for( size_t i = 0; i < numof(month_name); ++i )
  {
    if( !strcasecmp(month_name[i], str) )
    {
      return (int)i;
    }
  }
  return (parse_int(str) + 11) % 12;
}

/** String to week day number helper, compatible with struct tm indexing */
static int parse_weekday(const char *str)
{
  for( size_t i = 0; i < numof(weekday_name); ++i )
  {
    if( !strcasecmp(weekday_name[i], str) )
    {
      return (int)i;
    }
  }
  return parse_int(str) % 7;
}

/** String to struct tm helper */
static bool parse_date_and_time(const char *str, struct tm *tm)
{
  bool ok = false;

  int Y=0, M=0, D=0, h=0, m=0, s=0;

  memset(tm, 0, sizeof *tm);

  if( sscanf(str, "%d-%d-%d %d:%d:%d", &Y,&M,&D, &h,&m,&s) >= 5 )
  {
    tm->tm_year = (Y >= 1900) ? (Y-1900) : Y;
    tm->tm_mon  = M - 1;
    tm->tm_mday = D;
    tm->tm_hour = h;
    tm->tm_min  = m;
    tm->tm_sec  = s;
    tm->tm_isdst = -1;
    mktime(tm);
    ok = true;
  }

  return ok;
}

/** String token extractor */
static char *slice(char *pos, char **ppos, int sep)
{
  // skip leading white space
  while( *pos != 0 && isspace(*pos) ) ++pos;

  // find end of token
  char *end = pos;
  for( ; *end != 0; ++end )
  {
    if( *end == sep )
    {
      *end++ = 0;
      break;
    }
  }

  // store parse position
  if( ppos ) *ppos = end;

  return pos;
}

/* ------------------------------------------------------------------------- *
 * Shared on-demand timed dbus interface object
 * ------------------------------------------------------------------------- */

/** Cached timed dbus interface object */
static Maemo::Timed::Interface *timed_dbus_ptr = 0;

/** Create and cache timed dbus interface object */
static Maemo::Timed::Interface &timed_dbus_init()
{
  if( timed_dbus_ptr == 0 )
  {
    timed_dbus_ptr = new Maemo::Timed::Interface();

    if( !timed_dbus_ptr->isValid() )
    {
      qWarning() << "invalid dbus interface:" << timed_dbus_ptr->lastError();
      exit(EXIT_FAILURE);
    }
  }
  return *timed_dbus_ptr;
}

/** Delete cached timed dbus interface object */
static void timed_dbus_quit(void)
{
  delete timed_dbus_ptr; timed_dbus_ptr = 0;
}

#define timed_dbus timed_dbus_init()

/* ------------------------------------------------------------------------- *
 * On demand event object
 * ------------------------------------------------------------------------- */

/** Alarm event under construction */
static Maemo::Timed::Event *cur_eve_ptr = 0;

/** Create and cache alarm event object */
static Maemo::Timed::Event &cur_eve_init(void)
{
  if( !cur_eve_ptr )
  {
    cur_eve_ptr = new Maemo::Timed::Event();
  }
  return *cur_eve_ptr;
}

/** Delete cached alarm event object */
static void cur_eve_quit()
{
  delete cur_eve_ptr; cur_eve_ptr = 0;
}

#define cur_eve cur_eve_init()

/* ------------------------------------------------------------------------- *
 * Lookup table for action flags
 * ------------------------------------------------------------------------- */

struct action_flag_t
{
  // name of the flag
  const char *name;

  // pointers to suitable methdods
  bool (Maemo::Timed::Event::Action::*get)(void) const;
  void (Maemo::Timed::Event::Action::*set)(void);
  void (Maemo::Timed::Event::Action::*clr)(void);

  // or helper functions
  bool (*get2)(const Maemo::Timed::Event::Action &act);
  void (*set2)(Maemo::Timed::Event &eve, Maemo::Timed::Event::Action &act);
  void (*clr2)(Maemo::Timed::Event &eve, Maemo::Timed::Event::Action &act);

};

/* Helper function for accessing sys button flags by index */
static bool action_get_sys_button(const Maemo::Timed::Event::Action &act,
                                  int n)
{
  return act.whenSysButtons().indexOf(n) != -1;
}

static void action_set_sys_button(Maemo::Timed::Event &eve,
                                  Maemo::Timed::Event::Action &act, int n)
{
  (void)eve; // event ref not needed for sys buttons
  act.whenSysButton(n);
}

static void action_clr_sys_button(Maemo::Timed::Event &eve,
                                  Maemo::Timed::Event::Action &act, int n)
{
  (void)eve; // event ref not needed for sys buttons
  act.clearWhenSysButton(n);
}

/* Helper function for accessing user button flags by index */
static bool action_get_usr_button(const Maemo::Timed::Event::Action &act,
                                  int n)
{
  return act.whenButtons().indexOf(n) != -1;
}

static void action_set_usr_button(Maemo::Timed::Event &eve,
                                  Maemo::Timed::Event::Action &act, int n)
{
  Maemo::Timed::Event::Button &button = eve.button(n) ;
  act.whenButton(button);
}

static void action_clr_usr_button(Maemo::Timed::Event &eve,
                                  Maemo::Timed::Event::Action &act, int n)
{
  Maemo::Timed::Event::Button &button = eve.button(n) ;
  act.clearWhenButton(button);
}

/* Generate lookup table compatible glue functions for sys button flags */
#define X(N)\
   static bool action_get_sys_button##N(const Maemo::Timed::Event::Action &act) {\
     return action_get_sys_button(act, N);\
   }\
   static void action_set_sys_button##N(Maemo::Timed::Event &eve,\
                                        Maemo::Timed::Event::Action &act) {\
     action_set_sys_button(eve, act, N);\
   }\
   static void action_clr_sys_button##N(Maemo::Timed::Event &eve,\
                                        Maemo::Timed::Event::Action &act) {\
     action_clr_sys_button(eve, act, N);\
   }
X(0) X(1) X(2)
#undef X

/* Generate lookup table compatible glue functions for user button flags */
#define X(N)\
   static bool action_get_usr_button##N(const Maemo::Timed::Event::Action &act) {\
     return action_get_usr_button(act, N);\
   }\
   static void action_set_usr_button##N(Maemo::Timed::Event &eve,\
                                        Maemo::Timed::Event::Action &act) {\
     action_set_usr_button(eve, act, N);\
   }\
   static void action_clr_usr_button##N(Maemo::Timed::Event &eve,\
                                        Maemo::Timed::Event::Action &act) {\
     action_clr_usr_button(eve, act, N);\
   }
X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8)
#undef X

static const action_flag_t action_flag_lut[] =
{
  {
    "sendCookie",
    &Maemo::Timed::Event::Action::sendCookieFlag,
    &Maemo::Timed::Event::Action::setSendCookieFlag,
    &Maemo::Timed::Event::Action::clearSendCookieFlag,
    0,0,0,
  },
  {
    "runCommand",
    &Maemo::Timed::Event::Action::runCommandFlag,
    &Maemo::Timed::Event::Action::runCommand,
    &Maemo::Timed::Event::Action::removeCommand,
    0,0,0,
  },
  {
    "dbusMethodCall",
    &Maemo::Timed::Event::Action::dbusMethodCallFlag,
    &Maemo::Timed::Event::Action::dbusMethodCall,
    &Maemo::Timed::Event::Action::removeDbusMethodCall,
    0,0,0,
  },
  {
    "sendEventAttributes",
    &Maemo::Timed::Event::Action::sendEventAttributesFlag,
    &Maemo::Timed::Event::Action::setSendEventAttributesFlag,
    &Maemo::Timed::Event::Action::clearSendEventAttributesFlag,
    0,0,0,
  },
  {
    "sendAttributes",
    &Maemo::Timed::Event::Action::sendAttributesFlag,
    &Maemo::Timed::Event::Action::setSendAttributesFlag,
    &Maemo::Timed::Event::Action::clearSendAttributesFlag,
    0,0,0,
  },
  {
    "dbusSignal",
    &Maemo::Timed::Event::Action::dbusSignalFlag,
    &Maemo::Timed::Event::Action::dbusSignal,
    &Maemo::Timed::Event::Action::removeDbusSignal,
    0,0,0,
  },
  {
    "useSystemBus",
    &Maemo::Timed::Event::Action::useSystemBusFlag,
    &Maemo::Timed::Event::Action::setUseSystemBusFlag,
    &Maemo::Timed::Event::Action::clearUseSystemBusFlag,
    0,0,0,
  },
  {
    "whenTriggered",
    &Maemo::Timed::Event::Action::whenTriggeredFlag,
    &Maemo::Timed::Event::Action::whenTriggered,
    &Maemo::Timed::Event::Action::clearWhenTriggeredFlag,
    0,0,0,
  },
  {
    "whenQueued",
    &Maemo::Timed::Event::Action::whenQueuedFlag,
    &Maemo::Timed::Event::Action::whenQueued,
    &Maemo::Timed::Event::Action::clearWhenQueuedFlag,
    0,0,0,
  },
  {
    "whenMissed",
    &Maemo::Timed::Event::Action::whenMissedFlag,
    &Maemo::Timed::Event::Action::whenMissed,
    &Maemo::Timed::Event::Action::clearWhenMissedFlag,
    0,0,0,
  },
  {
    "whenFinalized",
    &Maemo::Timed::Event::Action::whenFinalizedFlag,
    &Maemo::Timed::Event::Action::whenFinalized,
    &Maemo::Timed::Event::Action::clearWhenFinalizedFlag,
    0,0,0,
  },
  {
    "whenDue",
    &Maemo::Timed::Event::Action::whenDueFlag,
    &Maemo::Timed::Event::Action::whenDue,
    &Maemo::Timed::Event::Action::clearWhenDueFlag,
    0,0,0,
  },
  {
    "whenSnoozed",
    &Maemo::Timed::Event::Action::whenSnoozedFlag,
    &Maemo::Timed::Event::Action::whenSnoozed,
    &Maemo::Timed::Event::Action::clearWhenSnoozedFlag,
    0,0,0,
  },
  {
    "whenServed",
    &Maemo::Timed::Event::Action::whenServedFlag,
    &Maemo::Timed::Event::Action::whenServed,
    &Maemo::Timed::Event::Action::clearWhenServedFlag,
    0,0,0,
  },
  {
    "whenAborted",
    &Maemo::Timed::Event::Action::whenAbortedFlag,
    &Maemo::Timed::Event::Action::whenAborted,
    &Maemo::Timed::Event::Action::clearWhenAbortedFlag,
    0,0,0,
  },
  {
    "whenFailed",
    &Maemo::Timed::Event::Action::whenFailedFlag,
    &Maemo::Timed::Event::Action::whenFailed,
    &Maemo::Timed::Event::Action::clearWhenFailedFlag,
    0,0,0,
  },
  {
    "whenTranquil",
    &Maemo::Timed::Event::Action::whenTranquilFlag,
    &Maemo::Timed::Event::Action::whenTranquil,
    &Maemo::Timed::Event::Action::clearWhenTranquilFlag,
    0,0,0,
  },

  /* Generate entries for: "whenSysButton0" ... "whenSysButton2" */
#define X(btn)\
  {\
    "whenSysButton"#btn,\
    0,0,0,\
    action_get_sys_button##btn,\
    action_set_sys_button##btn,\
    action_clr_sys_button##btn,\
  },
  X(0) X(1) X(2)
#undef X

  /* Generate entries for: "whenButton0" ... "whenButton8" */
#define X(btn)\
  {\
    "whenButton"#btn,\
    0,0,0,\
    action_get_usr_button##btn,\
    action_set_usr_button##btn,\
    action_clr_usr_button##btn,\
  },
  X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8)
#undef X
  {
    0,0,0,0,0,0,0,
  }
};

/* ------------------------------------------------------------------------- *
 * Lookup table for event flags
 * ------------------------------------------------------------------------- */

struct event_flag_t
{
  // name of the flag
  const char *name;

  // pointers to methods
  bool (Maemo::Timed::Event::*get)(void) const;
  void (Maemo::Timed::Event::*set)(void);
  void (Maemo::Timed::Event::*clr)(void);

};

static const event_flag_t event_flag_lut[] =
{
  {
    "alarm",
    &Maemo::Timed::Event::alarmFlag,
    &Maemo::Timed::Event::setAlarmFlag,
    &Maemo::Timed::Event::clearAlarmFlag,
  },
  {
    "triggerIfMissed",
    &Maemo::Timed::Event::triggerIfMissedFlag,
    &Maemo::Timed::Event::setTriggerIfMissedFlag,
    &Maemo::Timed::Event::clearTriggerIfMissedFlag,
  },
  {
    "triggerWhenAdjusting",
    &Maemo::Timed::Event::triggerWhenAdjustingFlag,
    &Maemo::Timed::Event::setTriggerWhenAdjustingFlag,
    &Maemo::Timed::Event::clearTriggerWhenAdjustingFlag,
  },
  {
    "triggerWhenSettingsChanged",
    &Maemo::Timed::Event::triggerWhenSettingsChangedFlag,
    &Maemo::Timed::Event::setTriggerWhenSettingsChangedFlag,
    &Maemo::Timed::Event::clearTriggerWhenSettingsChangedFlag,
  },
  {
    "userMode",
    &Maemo::Timed::Event::userModeFlag,
    &Maemo::Timed::Event::setUserModeFlag,
    &Maemo::Timed::Event::clearUserModeFlag,
  },
  {
    "alignedSnooze",
    &Maemo::Timed::Event::alignedSnoozeFlag,
    &Maemo::Timed::Event::setAlignedSnoozeFlag,
    &Maemo::Timed::Event::clearAlignedSnoozeFlag,
  },
  {
    "reminder",
    &Maemo::Timed::Event::reminderFlag,
    &Maemo::Timed::Event::setReminderFlag,
    &Maemo::Timed::Event::clearReminderFlag,
  },
  {
    "boot",
    &Maemo::Timed::Event::bootFlag,
    &Maemo::Timed::Event::setBootFlag,
    &Maemo::Timed::Event::clearBootFlag,
  },
  {
    "keepAlive",
    &Maemo::Timed::Event::keepAliveFlag,
    &Maemo::Timed::Event::setKeepAliveFlag,
    &Maemo::Timed::Event::clearKeepAliveFlag,
  },
  {
    "singleShot",
    &Maemo::Timed::Event::singleShotFlag,
    &Maemo::Timed::Event::setSingleShotFlag,
    &Maemo::Timed::Event::clearSingleShotFlag,
  },
  {
    "backup",
    &Maemo::Timed::Event::backupFlag,
    &Maemo::Timed::Event::setBackupFlag,
    &Maemo::Timed::Event::clearBackupFlag,
  },
  {
    "suppressTimeoutSnooze",
    &Maemo::Timed::Event::doSuppressTimeoutSnooze,
    &Maemo::Timed::Event::suppressTimeoutSnooze,
    &Maemo::Timed::Event::allowTimeoutSnooze,
  },
  {
    "hideSnoozeButton1",
    &Maemo::Timed::Event::doHideSnoozeButton1,
    &Maemo::Timed::Event::hideSnoozeButton1,
    &Maemo::Timed::Event::showSnoozeButton1,

  },
  {
    "hideCancelButton2",
    &Maemo::Timed::Event::doHideCancelButton2,
    &Maemo::Timed::Event::hideCancelButton2,
    &Maemo::Timed::Event::showCancelButton2,
  },
  {
    0,0,0,0
  }
};

/* ------------------------------------------------------------------------- *
 * action helpers
 * ------------------------------------------------------------------------- */

/** Set action flag by name */
static bool action_set_flag(CONST Maemo::Timed::Event &eve,
                            Maemo::Timed::Event::Action &act,
                            const char *flag)
{
  for( size_t i = 0; action_flag_lut[i].name; ++i )
  {
    if( strcmp(action_flag_lut[i].name, flag) )
    {
      continue;
    }

    if( action_flag_lut[i].set )
    {
      (act.*action_flag_lut[i].set)();
    }
    else if( action_flag_lut[i].set2 )
    {
      action_flag_lut[i].set2(eve, act);
    }
    else
    {
      continue;
    }

    return true;
  }
  return false;
}

/* ------------------------------------------------------------------------- *
 * Event helpers
 * ------------------------------------------------------------------------- */

/** Set event flag by name */
static bool event_set_flag(Maemo::Timed::Event &eve,  const char *flag)
{
  for( size_t i = 0; event_flag_lut[i].name; ++i )
  {
    if( strcmp(event_flag_lut[i].name, flag) )
    {
      continue;
    }

    if( event_flag_lut[i].set )
    {
      (eve.*event_flag_lut[i].set)();
    }
    else
    {
      continue;
    }

    return true;
  }
  return false;
}

/** Output event attributes */
static void event_emit_attributes(const Maemo::Timed::Event &eve)
{
  QMapIterator<QString, QString> i(eve.attributes());
  while( i.hasNext() )
  {
    i.next();
    printf("  %s = '%s'\n", sz(i.key()), sz(i.value()));
  }
}

/** Output event triggering time */
static void event_emit_time(const Maemo::Timed::Event &eve)
{
  time_t t = eve.ticker();
  time_t n = time(0);

  if( t < n )
  {
    printf("  Tick: %ld (inactive)\n", (long)t);
  }
  else
  {
    struct tm tm;
    memset(&tm, 0, sizeof tm);
    localtime_r(&t, &tm);
    printf("  Tick: %ld (T%+ld @ %04d-%02d-%02d %02d:%02d:%02d %s)\n",
           (long)t,
           (long)(n-t),
           tm.tm_year + 1900,
           tm.tm_mon + 1,
           tm.tm_mday,
           tm.tm_hour,
           tm.tm_min,
           tm.tm_sec,
           tm.tm_zone);
  }
  printf("  Time: %04u-%02u-%02u %02u:%02u %s\n",
         eve.year(),
         eve.month(),
         eve.day(),
         eve.hour(),
         eve.minute(),
         sz(eve.timezone()));
}

/** Output event flags */
static void event_emit_flags(const Maemo::Timed::Event &eve)
{
  printf("  Flags:");
  for( size_t i = 0; event_flag_lut[i].name; ++i )
  {
    if( (eve.*event_flag_lut[i].get)() )
    {
      printf(" %s", event_flag_lut[i].name);
    }
  }
  printf("\n");
}

/** Output details of event buttons */
static void event_emit_buttons(CONST Maemo::Timed::Event &eve)
{
  for( int i = 0, n = eve.buttonsCount(); i < n; ++i )
  {
    const Maemo::Timed::Event::Button &button = eve.button(i);

    printf("  Button%d:\n", i);

    QMapIterator<QString, QString> i(button.attributes());
    while( i.hasNext() )
    {
      i.next();
      printf("    %s = '%s'\n", sz(i.key()), sz(i.value()));
    }

    printf("    Snooze: %d\n", button.snooze());
  }
}

/** Output details of event actions */
static void event_emit_actions(CONST Maemo::Timed::Event &eve)
{
  for( int i = 0, n = eve.actionsCount(); i < n; ++i )
  {
    const Maemo::Timed::Event::Action &act = eve.action(i);

    printf("  Action%d:\n", i);

    QMapIterator<QString, QString> i(act.attributes());
    while( i.hasNext() )
    {
      i.next();
      printf("    %s = '%s'\n", sz(i.key()), sz(i.value()));
    }

    printf("    Flags:");
    for( size_t i = 0; action_flag_lut[i].name; ++i )
    {
      bool flag = false;

      if( action_flag_lut[i].get )
      {
        flag = (act.*action_flag_lut[i].get)();
      }
      else if( action_flag_lut[i].get2 )
      {
        flag = action_flag_lut[i].get2(act);
      }
      if( flag )
      {
        printf(" %s", action_flag_lut[i].name);
      }
    }
    printf("\n");

    QStringList want = act.accruedCredentials();
    QStringList drop = act.droppedCredentials();
    if( !want.empty() )
    {
      printf("    AccruedCredentials: %s\n", sz(want.join(" ")));
    }
    if( !drop.empty() )
    {
      printf("    DroppedCredentials: %s\n", sz(drop.join(" ")));
    }
  }
}

/** Output details of event recurrence entries */
static void event_emit_recurrences(CONST Maemo::Timed::Event &eve)
{

  for( int i = 0, n = eve.recurrencesCount(); i < n; ++i )
  {
    const Maemo::Timed::Event::Recurrence &rec = eve.recurrence(i) ;

    printf("  Recurrence%d:\n", i);

    printf("    IsEmpty: %s\n", repr_bool(rec.isEmpty()));

    int mon  = rec.months();
    int mday = rec.daysOfMonth();
    int wday = rec.daysOfWeek();
    int hour = rec.hours();
    quint64 min = rec.minutes();

    if( mon != 0 )
    {
      printf("    Months:");
      for( int i = 0; i < 12; ++i )
      {
        if( mon & (1 << i) ) printf(" %s", month_name[i]);
      }
      printf("\n");
    }

    if( mday != 0 )
    {
      printf("    Days of month:");
      for( int i = 0; i <= 31; ++i )
      {
        if( mday & (1 << i) ) printf(" %d", i);
      }
      printf("\n");
    }

    if( wday != 0 )
    {
      printf("    Days of week:");
      for( int i = 0; i < 7; ++i )
      {
        if( wday & (1 << i) ) printf(" %s", weekday_name[i]);
      }
      printf("\n");
    }

    if( hour != 0 )
    {
      printf("    Hours:");
      for( int i = 0; i < 24; ++i )
      {
        if( hour & (1 << i) ) printf(" %d", i);
      }
      printf("\n");
    }

    if( min != 0 )
    {
      printf("    Minutes:");
      for( int i = 0; i < 24; ++i )
      {
        if( min & (1LL << i) ) printf(" %d", i);
      }
      printf("\n");
    }
  }
}

/** Output basic information about an event */
static void event_emit_info(const Maemo::Timed::Event &eve)
{
  event_emit_attributes(eve);
  event_emit_time(eve);
}

/** Output all information about an event */
static void event_emit_details(CONST Maemo::Timed::Event &eve)
{
  event_emit_info(eve);

  printf("  MaximalTimeoutSnoozeCounter: %d\n",
         eve.maximalTimeoutSnoozeCounter());
  printf("  TimeoutSnoozeLenght: %d\n",
         eve.timeoutSnoozeLenght());

  QStringList want = eve.accruedCredentials();
  QStringList drop = eve.droppedCredentials();
  if( !want.empty() )
  {
    printf("  AccruedCredentials: %s\n", sz(want.join(" ")));
  }
  if( !drop.empty() )
  {
    printf("  DroppedCredentials: %s\n", sz(drop.join(" ")));
  }

  event_emit_flags(eve);
  event_emit_recurrences(eve);
  event_emit_buttons(eve);
  event_emit_actions(eve);
}

/* ------------------------------------------------------------------------- *
 * Cookie helpers
 * ------------------------------------------------------------------------- */

/** Delete event from timed event queue */
static void cookie_cancel(uint cookie)
{
  QDBusReply<bool> res = timed_dbus.cancel_sync(cookie);

  if(!res.isValid())
  {
    qWarning() << "'cancel' call failed:" << timed_dbus.lastError();
  }
  else
  {
    printf("cookie %u deleted = %s\n", cookie, repr_bool(res.value()));
  }
}

/** Query an event from timed and list basic information about it */
static void cookie_emit_info(uint cookie)
{
  printf("Cookie %u\n", cookie);

  Maemo::Timed::Event::DBusReply res = timed_dbus.get_event_sync(cookie);

  if( !res.isValid() )
  {
    qWarning() << "'get_event' call failed:" << res.error() ;
  }
  else
  {
    event_emit_info(res.value());
  }
}

/** Query an event from timed and list all available information about it */
static void cookie_emit_details(uint cookie)
{
  printf("Cookie %u\n", cookie);

  Maemo::Timed::Event::DBusReply res = timed_dbus.get_event_sync(cookie);

  if( !res.isValid() )
  {
    qWarning() << "'get_event' call failed:" << res.error() ;
  }
  else
  {
    event_emit_details(res.value());
  }
}

/** Query a list of events from timed */
static QList<uint> cookies_get(void)
{
  QList<uint> res;

  QMap<QString,QVariant> match;
  QDBusReply< QList<QVariant> > reply = timed_dbus.query_sync(match);

  if( !reply.isValid() )
  {
    qWarning() << "'query' call failed:" << timed_dbus.lastError();
    goto cleanup;
  }

  foreach( QVariant var, reply.value() )
  {
    bool ok = true;
    uint cookie = var.toUInt(&ok);
    if( !ok )
    {
      qWarning() << var << " is not a valid cookie";
      continue;
    }
    res.append(cookie);
  }

cleanup:
  return res;

}

/* ------------------------------------------------------------------------- *
 * HANDLERS FOR COMMAND LINE OPTIONS
 * ------------------------------------------------------------------------- */

/** Handle option: --list */
static void do_list_all(void)
{
  int n = 0;
  foreach( uint cookie, cookies_get() )
  {
    printf("%s%u", n++ ? " " : "", cookie);
  }
  printf("\n");
}

/** Handle option: --info */
static void do_info_all(void)
{
  foreach( uint cookie, cookies_get() )
  {
    cookie_emit_info(cookie);
  }
}

/** Handle option: --show */
static void do_show_all(void)
{
  foreach( uint cookie, cookies_get() )
  {
    cookie_emit_details(cookie);
  }
}

/** Handle option: --ping */
static void do_ping_all(void)
{
  QDBusReply<QString> reply = timed_dbus.ping_sync();
  if(!reply.isValid())
  {
    qWarning() << "'ping' call failed" << timed_dbus.lastError();
  }
  else
  {
    printf("%s\n", sz(reply.value()));
  }
}

/** Handle option: --get-event=<cookie> */
static void do_show_one(char *args)
{
  uint cookie = parse_cookie(args);
  cookie_emit_details(cookie);
}

/** Handle option: --get-pid */
static void do_get_pid(void)
{
  QDBusReply<int> reply = timed_dbus.pid_sync();
  if( !reply.isValid() )
  {
    qWarning() << "'pid' call failed" << timed_dbus.lastError();
  }
  else
  {
    printf("%d\n", reply.value());
  }
}

/** Handle option: --set-snooze=<secs> */
static void do_set_snooze(char *args)
{
  int snooze = parse_int(args);
  QDBusReply<void> reply = timed_dbus.set_default_snooze_sync(snooze);

  if(! reply.isValid() )
  {
    qWarning() << "'set_default_snooze' call failed" << timed_dbus.lastError();
  }
  else
  {
    printf("default snooze set to %d\n", snooze);
  }
}

/** Handle option: --get-snooze */
static void do_get_snooze(void)
{
  QDBusReply<int> reply = timed_dbus.get_default_snooze_sync();
  if( !reply.isValid() )
  {
    qWarning() << "'get_default_snooze' call failed" << timed_dbus.lastError();
  }
  else
  {
    printf("%d\n", reply.value());
  }
}

/** Handle option: --get-enabled */
static void do_get_enabled(void)
{
  QDBusReply<bool> reply = timed_dbus.alarms_enabled_sync();
  if( !reply.isValid() )
  {
    qWarning() << "'alarms_enabled' call failed" << timed_dbus.lastError();
  }
  else
  {
    printf("%s\n", repr_bool(reply.value()));
  }
}

/** Handle option: --set-enabled=<bool> */
static void do_set_enabled(char *args)
{
  bool enabled = parse_bool(args);
  QDBusReply<void> reply = timed_dbus.enable_alarms_sync(enabled);

  if(! reply.isValid() )
  {
    qWarning() << "'enable_alarms' call failed" << timed_dbus.lastError();
  }
  else
  {
    printf("alarms enabled set to %s\n", repr_bool(enabled));
  }
}

/** Handle option: --cancel-event=<cookie> */
static void do_cancel_event(char *args)
{
  cookie_cancel(parse_cookie(args));
}

/** Handle option: --cancel-events */
static void do_cancel_events(void)
{
  foreach( uint cookie, cookies_get() )
  {
    cookie_cancel(cookie);
  }
}

/** Handle option: --add-button=<args> */
static void do_add_button(char *args)
{
  Maemo::Timed::Event::Button &btn = cur_eve.addButton();

  while( *args )
  {
    char *arg = slice(args, &args, ';');
    char *key = slice(arg, &arg, '=');
    char *val = slice(arg, &arg, 0);

    debugf("%s: '%s' = '%s'\n", __PRETTY_FUNCTION__, key, val);

    if( *key == 0 )
    {
      continue;
    }

    if( !strcmp(key, "snoozeDefault") )
    {
      btn.setSnoozeDefault();
      continue;
    }

    if( *val == 0 )
    {
      continue;
    }

    if( !strcmp(key, "snooze") )
    {
      btn.setSnooze(parse_secs(val));
    }
    else
    {
      btn.setAttribute(key, val);
    }
  }
}

/** Handle option: --add-action=<args> */
static void do_add_action(char *args)
{
  Maemo::Timed::Event::Action &act = cur_eve.addAction();

  while( *args )
  {
    char *arg = slice(args, &args, ';');
    char *key = slice(arg, &arg, '=');
    char *val = slice(arg, &arg, 0);

    debugf("%s: '%s' = '%s'\n", __PRETTY_FUNCTION__, key, val);

    if( *key == 0 )
    {
      continue;
    }

    if( *val == 0 )
    {
      if( !action_set_flag(cur_eve, act, key) )
      {
        fprintf(stderr, "unknown action flag: %s", key);
      }
      continue;
    }

    if( !strcmp(key, "runCommand") )
    {
      char *cmd = slice(val,&val,'@');
      char *usr = slice(val,&val,0);
      if( *usr )
      {
        act.runCommand(cmd, usr);
      }
      else
      {
        act.runCommand(cmd);
      }
    }
    else if( !strcmp(key, "dbusMethodCall") )
    {
      char *srv = slice(val,&val, ',');
      char *mtd = slice(val,&val, ',');
      char *obj = slice(val,&val, ',');
      char *ifc = slice(val,&val, ',');

      if( *srv && *mtd && *obj )
      {
        if( *ifc )
        {
          act.dbusMethodCall(srv, mtd, obj, ifc);
        }
        else
        {
          act.dbusMethodCall(srv, mtd, obj);
        }
      }
    }
    else if( !strcmp(key, "dbusSignal") )
    {
      char *op  = slice(val,&val, ',');
      char *ifc = slice(val,&val, ',');
      char *sg  = slice(val,&val, ',');

      if( *op && *ifc && *sg )
      {
        act.dbusSignal(op, ifc, sg);
      }
    }
    else if( !strcmp(key, "whenButton") )
    {
      int idx = parse_int(val);
      Maemo::Timed::Event::Button &btn = cur_eve.button(idx) ;
      act.whenButton(btn);
    }
    else if( !strcmp(key, "whenSysButton") )
    {
      act.whenSysButton(parse_int(val));
    }
    else
    {
      act.setAttribute(key, val);
    }
  }
}

/** Handle option: --add-recurrence=<args> */
static void do_add_recurrence(char *args)
{
  Maemo::Timed::Event::Recurrence &rec = cur_eve.addRecurrence();

  while( *args )
  {
    char *arg = slice(args, &args, ';');
    char *key = slice(arg, &arg, '=');
    char *val = slice(arg, &arg, 0);

    debugf("%s: '%s' = '%s'\n", __PRETTY_FUNCTION__, key, val);

    if( *key == 0 )
    {
      continue;
    }

    if( !strcmp(key, "lastDayOfMonth") )
    {
      rec.addLastDayOfMonth();
      continue;
    }

#define X(m) if( !strcmp(key, #m) ) { rec.m(); continue; }
    X(everyMonth)
    X(everyDayOfMonth)
    X(everyDayOfWeek)
#undef X

    if( *val == 0 )
    {
      continue;
    }

    if( !strcmp(key, "month") )
    {
      rec.addMonth(parse_month(val));
    }
    else if( !strcmp(key, "dayOfMonth") )
    {
      rec.addDayOfMonth(parse_int(val));
    }
    else if( !strcmp(key, "dayOfWeek") )
    {
      rec.addDayOfWeek(parse_weekday(val));
    }
    else if( !strcmp(key, "hour") )
    {
      rec.addHour(strtol(val, 0, 0));
    }
    else if( !strcmp(key, "minute") )
    {
      rec.addMinute(strtol(val, 0, 0));
    }
    else
    {
      qWarning() << "unknown recurrence setting:" << key << "=" << val;
    }
  }
  if( rec.isEmpty() )
  {
    qWarning() << "incomplete recurrence setting";
  }
}

/** Handle option: --add-event=<args> */
static void do_add_event(char *args)
{
  while( *args )
  {
    char *arg = slice(args, &args, ';');
    char *key = slice(arg, &arg, '=');
    char *val = slice(arg, &arg, 0);

    debugf("%s: '%s' = '%s'\n", __PRETTY_FUNCTION__, key, val);

    if( *key == 0 )
    {
      continue;
    }

    if( *val == 0 )
    {
      if( event_set_flag(cur_eve, key) )
      {
        fprintf(stderr, "unknown event flag: %s\n", key);
      }
      continue;
    }

    if( !strcmp(key, "ticker") )
    {
      time_t t = time(0) + parse_secs(val);
      cur_eve.setTicker(t);
    }
    else if( !strcmp(key, "time") )
    {
      struct tm tm;

      if( parse_date_and_time(val, &tm) )
      {
        int Y = tm.tm_year + 1900;
        int M = tm.tm_mon + 1;
        int D = tm.tm_mday;
        int h = tm.tm_hour;
        int m = tm.tm_min;

	debugf("Setting time: %04d-%02d-%02d %02d:%02d\n", Y,M,D, h,m);
        cur_eve.setTime(Y,M,D, h,m);
      }
      else
      {
        qWarning() << "could not parse date + time";
      }
    }
    else if( !strcmp(key, "timezone") )
    {
      cur_eve.setTimezone(val);
    }
    else
    {
      cur_eve.setAttribute(key, val);
    }
  }

  QDBusReply<uint> res = timed_dbus.add_event_sync(cur_eve);
  if(!res.isValid())
  {
    qWarning() << "'add_event' call failed:" << res.error().message();
  }
  else
  {
    uint cookie = res.value();

    if( cookie == 0 )
    {
      fprintf(stderr, "event adding failed, dumping event data:\n");
      event_emit_details(cur_eve);
    }
    else
    {
      printf("event added, cookie is %d\n", cookie);
    }
  }

  // mark as done
  cur_eve_quit();
}

/* ------------------------------------------------------------------------- *
 * COMMAND LINE OPTIONS
 * ------------------------------------------------------------------------- */

/** Lookup table for long command line options */
static const struct option OPT_L[] =
{
  {"help",           0, 0, 'h'},
  {"usage",          0, 0, 'h'},

  {"list",           0, 0, 'l'},
  {"info",           0, 0, 'i'},
  {"show",           0, 0, 'L'},
  {"ping",           0, 0, 'p'},

  {"add-button",     1, 0, 'b'}, // <args>
  {"add-action",     1, 0, 'a'}, // <args>
  {"add-recurrence", 1, 0, 'r'}, // <args>
  {"add-event",      1, 0, 'e'}, // <args>
  {"get-event",      1, 0, 'g'}, // <cookie>

  {"cancel-event",   1, 0, 'c'}, // <cookie>
  {"cancel-events",  0, 0, 'C'},

  {"set-snooze",     1, 0, 003}, // <secs>
  {"get-snooze",     0, 0, 004},
  {"set-enabled",    1, 0, 001}, // <bool>
  {"get-enabled",    0, 0, 002},
  {"get-pid",        0, 0, 005},

  // sentinel
  {0, 0, 0, 0}
};

/** Lookup table for short command line options */
static const char OPT_S[] =
"h"  // --help
"l"  // --list
"i"  // --info
"p"  // --ping
"L"  // --show
"c:" // --cancel-event=<cookie>
"C"  // --cancel-events
"g:" // --get-event=<cookie>
"b:" // --add-button=<cookie>
"a:" // --add-action=<cookie>
"r:" // --add-recurrence=<cookie>
"e:" // --add-event=<cookie>
;

/** Usage information made available via --help option */
static const char USAGE[] =
"NAME\n"
"  timedclient  --  timed test & debugging tool\n"
"\n"
"SYNOPSIS\n"
"  timedclient <options>\n"
"\n"
"OPTIONS\n"
"  --help   -h                         --  This help text"
"\n"
"  --list   -l                         --  List cookies\n"
"  --info   -i                         --  List cookies + key attributes\n"
"  --show   -L                         --  List cookies + all attributes\n"
"  --ping   -p                         --  Show raw alarm queue content\n"
"\n"
"  --add-button=<args>     -b<args>    --  Add button to event\n"
"  --add-action=<args>     -a<args>    --  Add action to event\n"
"  --add-recurrence=<args> -r<args>    --  Add recurrence to event\n"
"  --add-event=<args>      -e<args>    --  Send event to timed\n"
"  --get-event=<cookie>    -g<cookie>  --  Show details of event\n"
"\n"
"  --cancel-event=<cookie> -c<cookie>  --  Cancel one event\n"
"  --cancel-events         -C          --  Cancel all events\n"
"\n"
"  --set-snooze=<secs>                 --  Set default snooze value\n"
"  --get-snooze                        --  Query default snooze value\n"
"\n"
"  --set-enabled=<bool>                --  Enable/Disable alarms\n"
"  --get-enabled                       --  Query enabled status\n"
"\n"
"  --get-pid                           --  Query PID of timed process\n"
"\n"
"EXAMPLES\n"
"  Add two button alarm that triggers every tuesday 10:00\n"
"    timedclient\\\n"
"      -b'TITLE=button0' -b'TITLE=button1'\\\n"
"      -a'whenButton=0;runCommand=echo [COOKIE]button0'\\\n"
"      -a'whenButton=1;runCommand=echo [COOKIE]button1'\\\n"
"      -r'hour=10;minute=0;dayOfWeek=Tue;everyDayOfMonth;everyMonth;'\\\n"
"      -e'APPLICATION=test;TITLE=Hello'\n"
"\n"
"  Add one button alarm that triggers after 10 seconds\n"
"    timedclient -b'TITLE=button0'\\\n"
"                -e'APPLICATION=test;TITLE=Hello;ticker=10'\n"
"\n"
"BUTTON ARGS\n"
"  snooze=<secs>\n"
"  snoozeDefault\n"
"\n"
"  If none of the above is matched, assumes generic attribute\n"
"  of <ATTRIB_KEY>=<ATTRIB_VALUE> form, added via setAttribute()\n"
"\n"
"ACTION ARGS\n"
"  dbusMethodCall[=<service>,<method>,<object>[,<interface>]]\n"
"  dbusSignal[=<object>,<interface>,<signal>]\n"
"  runCommand[=<command>[@<user>]]\n"
"  whenButton=<button_index>\n"
"  whenSysButton=<button_index>\n"
"  sendAttributes\n"
"  sendCookie\n"
"  sendEventAttributes\n"
"  useSystemBus\n"
"  whenAborted\n"
"  whenDue\n"
"  whenFailed\n"
"  whenFinalized\n"
"  whenMissed\n"
"  whenQueued\n"
"  whenServed\n"
"  whenSnoozed\n"
"  whenTranquil\n"
"  whenTriggered\n"
"\n"
"  If none of the above is matched, assumes generic attribute\n"
"  of <ATTRIB_KEY>=<ATTRIB_VALUE> form, added via setAttribute()\n"
"\n"
"RECURRENCE ARGS\n"
"  dayOfMonth=<mday>\n"
"  dayOfWeek=<wday>\n"
"  hour=<hour>\n"
"  minute=<min>\n"
"  month=<month>\n"
"  lastDayOfMonth\n"
"  everyDayOfMonth\n"
"  everyDayOfWeek\n"
"  everyMonth\n"
"\n"
"EVENT ARGS\n"
"  ticker=<secs>\n"
"  time=<yyyy-mm-dd hh:mm[:ss]>\n"
"  timezone=<tz>\n"
"  hideCancelButton2\n"
"  hideSnoozeButton1\n"
"  alarm\n"
"  alignedSnooze\n"
"  boot\n"
"  keepAlive\n"
"  reminder\n"
"  singleShot\n"
"  triggerIfMissed\n"
"  userMode\n"
"  suppressTimeoutSnooze\n"
"\n"
"  If none of the above is matched, assumes generic attribute\n"
"  of <ATTRIB_KEY>=<ATTRIB_VALUE> form, added via setAttribute()\n"
"\n"
"NOTES\n"
"  You need to define buttons, actions and recurrence items before\n"
"  adding the alarm, i.e. pass options in order -bxxx -axxx -rxxx -exxx.\n"
"\n"
"  At minimum an alarm must have APPLICATION and TITLE attributes,\n"
"  and the alarm time specified via ticker, time or recurrence items.\n"
"\n"
"  A recurrence item is not valid unless it defines all fields, i.e. some\n"
"  values must be set for month, day of month, day of week, hour and minute.\n"
"\n"
"AUTHOR\n"
"  Simo Piiroinen <simo.piiroinen@jollamobile.com>\n"
"\n"
"COPYRIGHT \n"
"  Copyright(C) 2013 Jolla Ltd.\n"
"  This is free software; see the source for copying conditions.\n"
;

/* ------------------------------------------------------------------------- *
 * PROGRAM ENTRY POINT
 * ------------------------------------------------------------------------- */

int
main(int argc, char **argv)
{
  int xc = EXIT_FAILURE;

  QCoreApplication app(argc, argv);

  for( ;; )
  {
    int opt = getopt_long(argc, argv, OPT_S, OPT_L, 0);
    if( opt == -1 ) break;
    switch( opt )
    {
    case 'l': do_list_all();             break;
    case 'i': do_info_all();             break;
    case 'L': do_show_all();             break;
    case 'p': do_ping_all();             break;
    case 'g': do_show_one(optarg);       break;
    case 'b': do_add_button(optarg);     break;
    case 'a': do_add_action(optarg);     break;
    case 'r': do_add_recurrence(optarg); break;
    case 'e': do_add_event(optarg);      break;
    case 'c': do_cancel_event(optarg);   break;
    case 'C': do_cancel_events();        break;
    case 001: do_set_enabled(optarg);    break;
    case 002: do_get_enabled();          break;
    case 003: do_set_snooze(optarg);     break;
    case 004: do_get_snooze();           break;
    case 005: do_get_pid();              break;

    default:
      fprintf(stderr, "?? getopt returned character code 0x%x ??\n", opt);
      goto cleanup;

    case '?':
      // getopt has already emitted explanation
      goto cleanup;

    case 'h':
      printf("%s\n", USAGE);
      exit(EXIT_SUCCESS);
    }
  }

  if( optind < argc )
  {
    fprintf(stderr, "non-option argv-elements present:");
    while (optind < argc)
    {
      fprintf(stderr, " %s", argv[optind++]);
    }
    fprintf(stderr, "\n");
    goto cleanup;
  }

  if( cur_eve_ptr )
  {
    fprintf(stderr, "unfinalized alarm event left behind\n");
    goto cleanup;
  }

  xc = EXIT_SUCCESS;

cleanup:

  timed_dbus_quit();

  cur_eve_quit();

  return xc;
}
