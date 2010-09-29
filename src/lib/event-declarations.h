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
#ifndef MAEMO_TIMED_EVENT_DECLARATIONS_H
#define MAEMO_TIMED_EVENT_DECLARATIONS_H

#include <time.h>

#include <QString>
#include <QMap>
#include <QDBusMetaType>

#include <timed/qmacro.h>

namespace Maemo
{
  namespace Timed
  {
    class Event ;

    struct event_pimple_t ;
    struct event_action_pimple_t ;
    struct event_button_pimple_t ;
    struct event_recurrence_pimple_t ;
    struct event_list_pimple_t ;
  }
} ;

class Maemo::Timed::Event
{
  struct event_pimple_t *p ;
public:
  class Action ;
  class Button ;
  class Recurrence ;
  class List ;

  Event() ;
  void setTicker(time_t ticker) ;
  void setTime(unsigned year, unsigned month, unsigned day, unsigned hour, unsigned minute) ;
  void setTimezone(const QString &timezone) ;
  Action &addAction() ;
  Button &addButton() ;
  static unsigned getMaximalButtonAmount() ;
  static unsigned getSysButtonAmount() ;
  Recurrence &addRecurrence() ;
  void setAlarmFlag() ;
  void setTriggerIfMissedFlag() ;
  void setUserModeFlag() ;
  void setAlignedSnoozeFlag() ;
  void setReminderFlag() ;
  void setBootFlag() ;
  void setFakeFlag() ; // deprecated name
  void setKeepAliveFlag() ;
  void setSingleShotFlag() ;
  void suppressTimeoutSnooze() ;
  void hideSnoozeButton1() ;
  void hideCancelButton2() ;
  void setMaximalTimeoutSnoozeCounter(int tsz_max_counter) ;
  void setTimeoutSnooze(int tsz_length) ;
  void setAttribute(const QString &key, const QString &value) ;
  void credentialDrop(const QString &token) ;
  void credentialAccrue(const QString &token) ;
  int check(QString *err, bool exc) const ;
 ~Event() ;
private:
  friend class Interface ;
  friend class List ;
  QVariant dbus_output(const char *) const ;
} ;

class Maemo::Timed::Event::Action
{
  friend class Event ;
  struct event_action_pimple_t *p ;
public:
  void setAttribute(const QString &key, const QString &value) ;
  void runCommand() ;
  void runCommand(const QString &cmd) ;
  void runCommand(const QString &cmd, const QString &user) ;
  void dbusMethodCall() ;
  void dbusMethodCall(const QString &srv, const QString &mtd, const QString &obj, const QString &ifc=QString()) ;
  void dbusSignal() ;
  void dbusSignal(QString op, QString ifc, QString sg) ;
  void setSendCookieFlag() ;
  void setSendAttributesFlag() ;
  void setSendEventAttributesFlag() ;
  void setUseSystemBusFlag() ;
  void whenQueued() ;
  void whenDue() ;
  void whenMissed() ;
  void whenTriggered() ;
  void whenSnoozed() ;
  void whenServed() ;
  void whenAborted() ;
  void whenFailed() ;
  void whenFinalized() ;
  void whenTranquil() ;
  void whenButton(const Event::Button &x) ;
  void whenSysButton(int x) ;
  void credentialDrop(const QString &token) ;
  void credentialAccrue(const QString &token) ;
} ;

class Maemo::Timed::Event::Button
{
  struct event_button_pimple_t *p ;
  friend class Event ;
public:
  void setAttribute(const QString &key, const QString &value) ;
  void setSnooze(int sec) ;
  void setSnoozeDefault() ;
} ;

class Maemo::Timed::Event::Recurrence
{
  class event_recurrence_pimple_t *p ;
  friend class Event ;
public:
  bool isEmpty() const ;
  void addMonth(int x) ;
  void everyMonth() ;
  void addDayOfMonth(int x) ;
  void addLastDayOfMonth() ;
  void everyDayOfMonth() ;
  void addDayOfWeek(int x) ;
  void everyDayOfWeek() ;
  void addHour(int x) ;
  void addMinute(int x) ;
} ;

class Maemo::Timed::Event::List
{
public:
  List() ;
 ~List() ;
  Event & append() ;
private:
  event_list_pimple_t *p ;
  friend class Interface ;
  QVariant dbus_output() const ;
} ;

#if NOT_DONE_YET
void foo()
{
  Maemo::Timed::Event::Array A ;
  for(int i=0; i<10000; ++i)
  {
    Maemo::Timed::Event &e = A.append() ;
    e.setTicker(...) ;
    e.setWhatever(...) ;
  }
  Maemo::Timed::Interface timed ;
  QDBusReply<QList<uint>> res = timed.add_events_sync(A) ;
  if(res.iValid())
  {
    QList<uint> &cookies ;
    for(int i=0; i<cookies.size(); ++i)
      if(cookies[i])
        printf("event %d accepted, cookie=%d", i, cookies[i]) ;
      else
        printf("event %d rejected", i) ;
  }
}

#endif // NOT_DONE_YET



#endif
