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
#include <QString>
#include <QStringList>
#include <QMap>

#include <timed/event>

#include "pretty-print.h"

int Indent::step = 2 ;
QList<Indent*> Indent::stack ;

void Indent::setStep(int s)
{
  global().step = s ;
}

Indent::Indent()
  : local(0)
{
  global().stack << this ;
}

Indent::Indent(int l, int s)
  : local(l)
{
  step = s ;
}

Indent::~Indent()
{
  stack.removeLast() ;
}

Indent & Indent::global()
{
  static Indent globalIndent(0, 2) ;
  return globalIndent ;
}

Indent & Indent::operator++()
{
  ++local ;
  return *this ;
}

Indent & Indent::operator--()
{
  if(--local < 0)
    local = 0 ;
  return *this ;
}

QDebug operator<<(QDebug dbg, const Indent  &ind)
{
  int n = ind.global().local ;
  QList<Indent*>::const_iterator it ;
  for(it = ind.stack.begin(); it != ind.stack.end(); ++it)
  {
    n+=(*it)->local ;
  }
  n *= ind.step ;
  return dbg.nospace() << qPrintable(QString(n, ' ')) ;
}


QDebug operator<<(QDebug dbg, const QStringList &strList)
{
  if(strList.isEmpty())
  {
    dbg.nospace() << "empty" ;
  }
  else
  {
    QStringList::const_iterator it ;
    for(it = strList.begin(); it != strList.end(); ++it)
    {
      if(it != strList.begin())
      {
        dbg.nospace() << ", " ;
      }
      dbg.nospace() << *it ;
    }
  }
  return dbg.space() ;
}

QDebug print_attributes(QDebug dbg, QMap<QString, QString> attr)
{
  Indent ind ;
  dbg.nospace() << ind << "Attributes:" << endl ;
  QMap<QString, QString>::const_iterator it ;
  ++ind ;
  for(it = attr.begin(); it != attr.end(); ++it)
  {
    dbg.nospace() << ind << it.key() << ": " << it.value() << endl ;
  }
  return dbg.nospace() ;
}

QDebug print_time(QDebug dbg, const Maemo::Timed::Event &event)
{
  Indent ind ;
  if(event.year() >= 1970)
  {
    dbg.nospace() << ind << "Time:" ;
    dbg.space() << event.year() << "-" << event.month() << "-" << event.day() ;
    dbg.space() << event.hour() << ":" << event.minute() ;
    dbg.space() << event.timezone() << endl ;
  }
  else
  {
    dbg.nospace() << ind << "Ticker: " << event.ticker() << endl ;
  }
  return dbg.nospace() ;
}

QDebug print_flags(QDebug dbg, const Maemo::Timed::Event &event)
{
  Indent ind ;
  dbg.nospace() << ind << "Flags:" << endl ;
  ++ind ;
  dbg.nospace() << ind << "Alarm                         : " << event.alarmFlag() << endl ;
  dbg.nospace() << ind << "TriggerIfMissed               : " << event.triggerIfMissedFlag() << endl ;
  dbg.nospace() << ind << "TriggerWhenAdjustingFlag      : " << event.triggerWhenAdjustingFlag() << endl ;
  dbg.nospace() << ind << "TriggerWhenSettingsChangedFlag: " << event.triggerWhenSettingsChangedFlag() << endl ;
  dbg.nospace() << ind << "UserModeFlag                  : " << event.userModeFlag() << endl ;
  dbg.nospace() << ind << "AlignedSnoozeFlag             : " << event.alignedSnoozeFlag() << endl ;
  dbg.nospace() << ind << "ReminderFlag                  : " << event.reminderFlag() << endl ;
  dbg.nospace() << ind << "BootFlag                      : " << event.bootFlag() << endl ;
  dbg.nospace() << ind << "KeepAliveFlag                 : " << event.keepAliveFlag() << endl ;
  dbg.nospace() << ind << "SingleShotFlag                : " << event.singleShotFlag() << endl ;
  dbg.nospace() << ind << "BackupFlag                    : " << event.backupFlag() << endl ;
  dbg.nospace() << ind << "SuppressTimeoutSnooze         : " << event.doSuppressTimeoutSnooze() << endl ;
  dbg.nospace() << ind << "HideSnoozeButton              : " << event.doHideSnoozeButton1() << endl ;
  dbg.nospace() << ind << "HideCancelButton              : " << event.doHideCancelButton2() << endl ;
  return dbg.nospace() ;
}

QDebug print_snooze(QDebug dbg, const Maemo::Timed::Event &event)
{
  Indent ind ;
  dbg.nospace() << ind << "MaximalTimeoutSnoozeCounter: " << event.maximalTimeoutSnoozeCounter() << endl ;
  dbg.nospace() << ind << "TimeoutSnoozeLenght: " << event.timeoutSnoozeLenght() << endl ;
  return dbg.nospace() ;
}

QDebug print_credentials(QDebug dbg, const QStringList &dropped, const QStringList &accrued)
{
  Indent ind ;
  dbg.nospace() << ind << "DroppedCredentials: " << dropped << endl ;
  dbg.nospace() << ind << "AccruedCredentials: " << accrued << endl ;
  return dbg.nospace() ;
}

QDebug operator<<(QDebug dbg, const Maemo::Timed::Event::Button &button)
{
  Indent ind ;
  print_attributes(dbg, button.attributes()) ;
  dbg.nospace() << ind << "Snooze: " << button.snooze() << endl ;
  return dbg.nospace() ;
}

QDebug print_button(QDebug dbg, int number, const Maemo::Timed::Event::Button &button)
{
  Indent ind ;
  dbg.nospace() << ind << "Button " << number << ":" << endl ;
  ++ind ;
  dbg.nospace() << button ;
  return dbg.nospace() ;
}

QDebug print_buttons(QDebug dbg, const Maemo::Timed::Event &event)
{
  Indent ind ;
  if(event.buttonsCount() < 1)
  {
    dbg.nospace() << ind << "Buttons: no" << endl ;
  }
  else
  {
    for(int i = 0; i <event.buttonsCount() ; ++i)
    {
      print_button(dbg, i, const_cast<Maemo::Timed::Event &>(event).button(i)) ;
    }
  }
  return dbg.nospace() ;
}

QDebug operator<<(QDebug dbg, const QVector<int> &vectorInt)
{
  if(vectorInt.isEmpty())
  {
    dbg.nospace() << "no" ;
  }
  else
  {
    QVector<int>::const_iterator it ;
    for(it = vectorInt.begin(); it != vectorInt.end(); ++it)
    {
      if(it != vectorInt.begin())
      {
        dbg.nospace() << ", " ;
      }
      dbg.nospace() << *it ;
    }
  }
  return dbg.space() ;
}

QDebug print_commands(QDebug dbg, const Maemo::Timed::Event::Action &action)
{
  if(action.runCommandFlag())
  {
    Indent ind ;
    dbg.nospace() << ind << "Run command:" << endl ;
    ++ind ;
    dbg.nospace() << ind << "RunCommandLine: " << action.runCommandLine() << endl ;
    dbg.nospace() << ind << "RunCommandUser: " << action.runCommandUser() << endl ;
  }
  if(action.dbusMethodCallFlag())
  {
    Indent ind ;
    dbg.nospace() << ind << "DBus method call:" << endl ;
    ++ind ;
    dbg.nospace() << ind << "DbusMethodCallService  : " << action.dbusMethodCallService() << endl ;
    dbg.nospace() << ind << "DbusMethodCallPath     : " << action.dbusMethodCallPath() << endl ;
    dbg.nospace() << ind << "DbusMethodCallInterface: " << action.dbusMethodCallInterface() << endl ;
    dbg.nospace() << ind << "DbusMethodCallMethod   : " << action.dbusMethodCallMethod() << endl ;
  }
  if(action.dbusSignalFlag())
  {
    Indent ind ;
    dbg.nospace() << ind << "DBus signal:" << endl ;
    ++ind ;
    dbg.nospace() << ind << "DbusSignalPath     : " << action.dbusSignalPath() << endl ;
    dbg.nospace() << ind << "DbusSignalInterface: " << action.dbusSignalInterface() << endl ;
    dbg.nospace() << ind << "DbusSignalName     : " << action.dbusSignalName() << endl ;
  }
  return dbg.nospace() ;
}

QDebug print_flags(QDebug dbg, const Maemo::Timed::Event::Action &action)
{
  Indent ind ;
  dbg.nospace() << ind << "Flags:" << endl ;
  ++ind ;
  dbg.nospace() << ind << "SendCookieFlag         : " << action.sendCookieFlag() << endl ;
  dbg.nospace() << ind << "SendAttributesFlag     : " << action.sendAttributesFlag() << endl ;
  dbg.nospace() << ind << "SendEventAttributesFlag: " << action.sendEventAttributesFlag() << endl ;
  dbg.nospace() << ind << "UseSystemBusFlag       : " << action.useSystemBusFlag() << endl ;
  dbg.nospace() << ind << "WhenQueuedFlag         : " << action.whenQueuedFlag() << endl ;
  dbg.nospace() << ind << "WhenDueFlag            : " << action.whenDueFlag() << endl ;
  dbg.nospace() << ind << "WhenMissedFlag         : " << action.whenMissedFlag() << endl ;
  dbg.nospace() << ind << "WhenTriggeredFlag      : " << action.whenTranquilFlag() << endl ;
  dbg.nospace() << ind << "WhenSnoozedFlag        : " << action.whenSnoozedFlag() << endl ;
  dbg.nospace() << ind << "WhenServedFlag         : " << action.whenServedFlag() << endl ;
  dbg.nospace() << ind << "WhenAbortedFlag        : " << action.whenAbortedFlag() << endl ;
  dbg.nospace() << ind << "WhenFailedFlag         : " << action.whenFailedFlag() << endl ;
  dbg.nospace() << ind << "WhenFinalizedFlag      : " << action.whenFinalizedFlag() << endl ;
  dbg.nospace() << ind << "WhenTranquilFlag       : " << action.whenTranquilFlag() << endl ;
  dbg.nospace() << ind << "WhenAppButtons         : " << action.whenButtons() << endl ;
  dbg.nospace() << ind << "WhenSysButtons         : " << action.whenSysButtons() << endl ;
  return dbg.nospace() ;
}

QDebug operator<<(QDebug dbg, const Maemo::Timed::Event::Action &action)
{
  Indent ind ;
  print_attributes(dbg, action.attributes()) ;
  print_commands(dbg, action) ;
  print_flags(dbg, action) ;
  print_credentials(dbg, action.droppedCredentials(), action.accruedCredentials()) ;
  return dbg.nospace() ;
}

QDebug print_action(QDebug dbg, int number, const Maemo::Timed::Event::Action &action)
{
  Indent ind ;
  dbg.nospace() << ind << "Action " << number << ":" << endl ;
  ++ind ;
  dbg.nospace() << action ;
  return dbg.nospace() ;
}

QDebug print_actions(QDebug dbg, const Maemo::Timed::Event &event)
{
  Indent ind ;
  if(event.actionsCount() < 1)
  {
    dbg.nospace() << ind << "Actions: no" << endl ;
  }
  else
  {
    for(int i = 0; i <event.actionsCount() ; ++i)
    {
      print_action(dbg, i, const_cast<Maemo::Timed::Event &>(event).action(i)) ;
    }
  }
  return dbg.nospace() ;
}

QDebug print_months(QDebug dbg, int months)
{
  Indent ind ;
  const int everyMonth = 07777 ;
  int testMonths = months & everyMonth ;
  dbg.nospace() << ind << "Months: " ;
  if(testMonths == everyMonth)
  {
    dbg.nospace() << "every" ;
  }
  else if (testMonths == 0)
  {
    dbg.nospace() << "no" ;
  }
  else
  {
    QStringList monthNames ;
    monthNames << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec" ;
    bool wasPrinted = false ;
    for(int i = 0; i < 12; ++i, months >>= 1)
    {
      if(months & 1u)
      {
        if(wasPrinted)
        {
          dbg.nospace() << ", " ;
        }
        dbg.nospace() << monthNames[i] ;
        wasPrinted = true ;
      }
    }
  }
  return dbg.nospace() << endl ;
}

QDebug print_day_of_month(QDebug dbg, int days_of_month)
{
  Indent ind ;
  const int everyDayOfMonth = 0xFFFFFFFF ;
  int testDaysOfMonth = days_of_month & everyDayOfMonth ;
  dbg.nospace() << ind << "Days of month: " ;
  if(testDaysOfMonth == everyDayOfMonth)
  {
    dbg.nospace() << "every" ;
  }
  else if (testDaysOfMonth == 0)
  {
    dbg.nospace() << "no" ;
  }
  else
  {
    bool wasPrinted = false ;
    for(int i = 0; i < 12; ++i, days_of_month >>= 1)
    {
      if(days_of_month & 1u)
      {
        if(wasPrinted)
        {
          dbg.nospace() << ", " ;
        }
        if(i == 0)
        {
          dbg.nospace() << "last" ;
        }
        else
        {
          dbg.nospace() << i ;
        }
        wasPrinted = true ;
      }
    }
  }
  return dbg.nospace() << endl ;
}

QDebug print_day_of_week(QDebug dbg, int days_of_week)
{
  Indent ind ;
  const int everyDayOfWeek = 0177 ;
  int testDaysOfWeek = days_of_week & everyDayOfWeek ;
  dbg.nospace() << ind << "Week days: " ;
  if(testDaysOfWeek == everyDayOfWeek)
  {
    dbg.nospace() << "every" ;
  }
  else if (testDaysOfWeek == 0)
  {
    dbg.nospace() << "no" ;
  }
  else
  {
    QStringList weekDaysNames ;
    weekDaysNames << "Sun" << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat" ;
    bool wasPrinted = false ;
    for(int i = 0; i < 7; ++i, days_of_week >>= 1)
    {
      if(days_of_week & 1u)
      {
        if(wasPrinted)
        {
          dbg.nospace() << ", " ;
        }
        dbg.nospace() << weekDaysNames[i] ;
        wasPrinted = true ;
      }
    }
  }
  return dbg.nospace() << endl ;
}

QDebug print_hours(QDebug dbg, int hours)
{
  Indent ind ;
  const int everyHour = 077777777 ;
  int testHours = hours & everyHour ;
  dbg.nospace() << ind << "Hours: " ;
  if(testHours == everyHour)
  {
    dbg.nospace() << "every" ;
  }
  else if (testHours == 0)
  {
    dbg.nospace() << "no" ;
  }
  else
  {
    bool wasPrinted = false ;
    for(int i = 0; i < 24; ++i, hours >>= 1)
    {
      if(hours & 1u)
      {
        if(wasPrinted)
        {
          dbg.nospace() << ", " ;
        }
        dbg.nospace() << i ;
        wasPrinted = true ;
      }
    }
  }
  return dbg.nospace() << endl ;
}

QDebug print_minutes(QDebug dbg, quint64 minutes)
{
  Indent ind ;
  const quint64 everyMinute = 0x0FFFFFFFFFFFFFFFull ;
  quint64 testMinutes = minutes & everyMinute ;
  dbg.nospace() << ind << "Minutes: " ;
  if(testMinutes == everyMinute)
  {
    dbg.nospace() << "every" ;
  }
  else if (testMinutes == 0)
  {
    dbg.nospace() << "no" ;
  }
  else
  {
    bool wasPrinted = false ;
    for(int i = 0; i < 60; ++i, minutes >>= 1)
    {
      if(minutes & 1ull)
      {
        if(wasPrinted)
        {
          dbg.nospace() << ", " ;
        }
        dbg.nospace() << i ;
        wasPrinted = true ;
      }
    }
  }
  return dbg.nospace() << endl ;
}

QDebug print_flags(QDebug dbg, const Maemo::Timed::Event::Recurrence &recurrence)
{
  Indent ind ;
  dbg.nospace() << ind << "Flags:" << endl ;
  ++ind ;
  dbg.nospace() << ind << "isEmpty        : " << recurrence.isEmpty() << endl ;
  dbg.nospace() << ind << "FillingGapsFlag: " << recurrence.fillingGapsFlag() << endl ;
  return dbg.nospace() ;
}

QDebug operator<<(QDebug dbg, const Maemo::Timed::Event::Recurrence &recurrence)
{
  Indent ind ;
  print_months(dbg, recurrence.months()) ;
  print_day_of_month(dbg, recurrence.daysOfMonth()) ;
  print_day_of_week(dbg, recurrence.daysOfWeek()) ;
  print_hours(dbg, recurrence.hours()) ;
  print_minutes(dbg, recurrence.minutes()) ;
  print_flags(dbg, recurrence) ;
  return dbg.nospace() ;
}

QDebug print_recurrence(QDebug dbg, int number, const Maemo::Timed::Event::Recurrence &recurrence)
{
  Indent ind ;
  dbg.nospace() << ind << "Recurrence " << number << ":" << endl ;
  ++ind ;
  dbg.nospace() << recurrence ;
  return dbg.nospace() ;
}

QDebug print_recurrences(QDebug dbg, const Maemo::Timed::Event &event)
{
  Indent ind ;
  if(event.recurrencesCount() < 1)
  {
    dbg.nospace() << ind << "Recurrences: no" << endl ;
  }
  else
  {
    for(int i = 0; i <event.recurrencesCount() ; ++i)
    {
      print_recurrence(dbg, i, const_cast<Maemo::Timed::Event &>(event).recurrence(i)) ;
    }
  }
  return dbg.nospace() ;
}

QDebug operator<<(QDebug dbg, const Maemo::Timed::Event &event)
{
  Indent ind ;
  dbg.nospace() << ind << "========= Event ==========" << endl ;
  ++ind ;
  print_time(dbg, event) ;
  print_attributes(dbg, event.attributes()) ;
  print_flags(dbg, event) ;
  print_snooze(dbg, event) ;
  print_credentials(dbg, event.droppedCredentials(), event.accruedCredentials()) ;
  print_actions(dbg, event) ;
  print_buttons(dbg, event) ;
  print_recurrences(dbg, event) ;
  --ind ;
  dbg.nospace() << ind << "==========================" << endl ;
  return dbg.nospace() ;
}

