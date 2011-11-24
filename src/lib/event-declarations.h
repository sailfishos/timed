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
    struct event_io_t ;
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
  time_t ticker() const ;
  void setTime(unsigned year, unsigned month, unsigned day, unsigned hour, unsigned minute) ;
  unsigned year() const ;
  unsigned month() const ;
  unsigned day() const ;
  unsigned hour() const ;
  unsigned minute() const ;
  void setTimezone(const QString &timezone) ;
  const QString &timezone() const ;
  Action &addAction() ;
  int actionsCount() const ;
  Action &action(int index) ;
  Button &addButton() ;
  int buttonsCount() const ;
  Button &button(int index) ;
  static unsigned getMaximalButtonAmount() ;
  static unsigned getSysButtonAmount() ;
  Recurrence &addRecurrence() ;
  int recurrencesCount() const ;
  Recurrence &recurrence(int index) ;
  void setAlarmFlag() ;
  bool alarmFlag() const ;
  void setTriggerIfMissedFlag() ;
  bool triggerIfMissedFlag() const ;
  void setTriggerWhenAdjustingFlag() ;
  bool triggerWhenAdjustingFlag() const ;
  void setTriggerWhenSettingsChangedFlag() ;
  bool triggerWhenSettingsChangedFlag() const ;
  void setUserModeFlag() ;
  bool userModeFlag() const ;
  void setAlignedSnoozeFlag() ;
  bool alignedSnoozeFlag() const ;
  void setReminderFlag() ;
  bool reminderFlag() const ;
  void setBootFlag() ;
  bool bootFlag() const ;
  void setFakeFlag() ; // deprecated name
  void setKeepAliveFlag() ;
  bool keepAliveFlag() const ;
  void setSingleShotFlag() ;
  bool singleShotFlag() const ;
  void setBackupFlag() ; // to be implemented
  bool backupFlag() const ;
  void suppressTimeoutSnooze() ;
  bool doSuppressTimeoutSnooze() const ;
  void hideSnoozeButton1() ;
  bool doHideSnoozeButton1() const ;
  void hideCancelButton2() ;
  bool doHideCancelButton2() const ;
  void setMaximalTimeoutSnoozeCounter(int tsz_max_counter) ;
  int maximalTimeoutSnoozeCounter() const ;
  void setTimeoutSnooze(int tsz_length) ;
  int timeoutSnoozeLenght() const ;
  void setAttribute(const QString &key, const QString &value) ;
  const QMap<QString, QString> &attributes() const ;
  void credentialDrop(const QString &token) ;
  QStringList droppedCredentials() const ;
  void credentialAccrue(const QString &token) ;
  QStringList accruedCredentials() const ;
  int check(QString *err, bool exc) const ;
 ~Event() ;
private:
  friend class Interface ;
  friend class EventDBusReply ;
  friend class EventDBusPendingReply ;
  friend class List ;
  Event(const event_io_t& eio) ;
  QVariant dbus_output(const char *) const ;
  Action * getAction(event_action_pimple_t *pa) const ;
  Button * getButton(event_button_pimple_t *pb) const ;
  Recurrence * getRecurrence(event_recurrence_pimple_t *pr) const ;
} ;

class Maemo::Timed::Event::Action
{
  friend class Event ;
  struct event_action_pimple_t *p ;
public:
  const QMap<QString, QString> &attributes() const ;
  void setAttribute(const QString &key, const QString &value) ;
  void runCommand() ;
  void runCommand(const QString &cmd) ;
  void runCommand(const QString &cmd, const QString &user) ;
  bool runCommandFlag() const ;
  const QString runCommandLine() const ;
  const QString runCommandUser() const ;
  void dbusMethodCall() ;
  void dbusMethodCall(const QString &srv, const QString &mtd, const QString &obj, const QString &ifc=QString()) ;
  bool dbusMethodCallFlag() const ;
  const QString dbusMethodCallService() const ;
  const QString dbusMethodCallMethod() const ;
  const QString dbusMethodCallPath() const ;
  const QString dbusMethodCallInterface() const ;
  void dbusSignal() ;
  void dbusSignal(QString op, QString ifc, QString sg) ;
  bool dbusSignalFlag() const ;
  const QString dbusSignalPath() const ;
  const QString dbusSignalName() const ;
  const QString dbusSignalInterface() const ;
  void setSendCookieFlag() ;
  bool sendCookieFlag() const ;
  void setSendAttributesFlag() ;
  bool sendAttributesFlag() const ;
  void setSendEventAttributesFlag() ;
  bool sendEventAttributesFlag() const ;
  void setUseSystemBusFlag() ;
  bool useSystemBusFlag() const ;
  void whenQueued() ;
  bool whenQueuedFlag() const ;
  void whenDue() ;
  bool whenDueFlag() const ;
  void whenMissed() ;
  bool whenMissedFlag() const ;
  void whenTriggered() ;
  bool whenTriggeredFlag() const ;
  void whenSnoozed() ;
  bool whenSnoozedFlag() const ;
  void whenServed() ;
  bool whenServedFlag() const ;
  void whenAborted() ;
  bool whenAbortedFlag() const ;
  void whenFailed() ;
  bool whenFailedFlag() const ;
  void whenFinalized() ;
  bool whenFinalizedFlag() const ;
  void whenTranquil() ;
  bool whenTranquilFlag() const ;
  void whenButton(const Event::Button &x) ;
  QVector<int> whenButtons() const ;
  void whenSysButton(int x) ;
  QVector<int> whenSysButtons() const ;
  void credentialDrop(const QString &token) ;
  QStringList droppedCredentials() const ;
  void credentialAccrue(const QString &token) ;
  QStringList accruedCredentials() const ;
} ;

class Maemo::Timed::Event::Button
{
  struct event_button_pimple_t *p ;
  friend class Event ;
public:
  void setAttribute(const QString &key, const QString &value) ;
  void removeAttribute(const QString &key) ;
  void clearAttributes() ;
  const QMap<QString, QString> &attributes() const ;
  void setSnooze(int sec) ;
  int snooze() const ;
  void setSnoozeDefault() ;
} ;

class Maemo::Timed::Event::Recurrence
{
  class event_recurrence_pimple_t *p ;
  friend class Event ;
public:
  bool isEmpty() const ;
  int months() const ;
  void addMonth(int x) ;
  void everyMonth() ;
  void removeMonth(int x) ;
  void removeEveryMonth() ;
  int daysOfMonth() const ;
  void addDayOfMonth(int x) ;
  void addLastDayOfMonth() ;
  void everyDayOfMonth() ;
  void removeDayOfMonth(int x) ;
  void removeLastDayOfMonth() ;
  void removeEveryDayOfMonth() ;
  int daysOfWeek() const ;
  void addDayOfWeek(int x) ;
  void everyDayOfWeek() ;
  void removeDayOfWeek(int x) ;
  void removeEveryDayOfWeek() ;
  int hours() const ;
  void addHour(int x) ;
  void removeHour(int x) ;
  void removeEveryHour() ;
  quint64 minutes() const ;
  void addMinute(int x) ;
  void removeMinute(int x) ;
  void removeEveryMinute() ;
  bool fillingGapsFlag() const ;
  void setFillingGapsFlag() ;
  void clearFillingGapsFlag() ;
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
