/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
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
#include <qm/log>

#include "daemon/flags.h"

#include "exception.h"
#include "qmacro.h"

#include "event-pimple.h"
#include "daemon/timeutil.h"

static void set_attribute(const char *pretty, Maemo::Timed::attribute_io_t &attr, const QString &key, const QString &value)
{
  if(key.isEmpty())
    throw Maemo::Timed::Exception(pretty, "empty attrubute key") ;
  if(value.isEmpty())
    throw Maemo::Timed::Exception(pretty, "empty attribute value") ;
  if(attr.txt.count(key))
    throw Maemo::Timed::Exception(pretty, "attribute key already defined") ;
  attr.txt.insert(key, value) ;
}

static void check_interval(const char *pretty, int value, int min, int max)
{
  if(value<min || max<value)
    throw Maemo::Timed::Exception(pretty, "value out of range") ;
}

Maemo::Timed::Event::Event()
{
  p = new event_pimple_t ;
}

Maemo::Timed::Event::~Event()
{
  delete p ;
}

void Maemo::Timed::Event::setTicker(time_t ticker)
{
  p->eio.ticker = ticker ;
}

void Maemo::Timed::Event::setTime(unsigned year, unsigned month, unsigned day, unsigned hour, unsigned minute)
{
  check_interval(__PRETTY_FUNCTION__, year, 1970, 2037) ;
  check_interval(__PRETTY_FUNCTION__, month, 1, 12) ;
  check_interval(__PRETTY_FUNCTION__, day, 1, broken_down_t::month_length(year,month)) ;
  check_interval(__PRETTY_FUNCTION__, hour, 0, 23) ;
  check_interval(__PRETTY_FUNCTION__, minute, 0, 59) ;
  p->eio.t_year = year ;
  p->eio.t_month = month ;
  p->eio.t_day = day ;
  p->eio.t_hour = hour ;
  p->eio.t_minute = minute ;
}

void Maemo::Timed::Event::setTimezone(const QString &timezone)
{
  p->eio.t_zone = timezone ;
}

Maemo::Timed::Event::Action & Maemo::Timed::Event::addAction()
{
  Maemo::Timed::Event::Action *ea = new Maemo::Timed::Event::Action ;
  Maemo::Timed::event_action_pimple_t *pa = new Maemo::Timed::event_action_pimple_t ;
  ea->p = pa ;
  pa->action_no = p->a.size() ;
  pa->ptr = std::auto_ptr<Action> (ea) ;
  pa->eio = & p->eio ;
  p->a.push_back(pa) ;
  p->eio.actions.resize(pa->action_no+1) ;
  return *ea ;
}

Maemo::Timed::Event::Button & Maemo::Timed::Event::addButton()
{
  setReminderFlag() ;
  if(p->b.size() >= Max_Number_of_App_Buttons)
    throw Exception(__PRETTY_FUNCTION__, "too many application buttons") ;
  Maemo::Timed::Event::Button *eb = new Maemo::Timed::Event::Button ;
  Maemo::Timed::event_button_pimple_t *pb = new Maemo::Timed::event_button_pimple_t ;
  eb->p = pb ;
  pb->button_no = p->b.size() ;
  pb->ptr = std::auto_ptr<Button> (eb) ;
  pb->eio = & p->eio ;
  p->b.push_back(pb) ;
  p->eio.buttons.resize(pb->button_no+1) ;
  return *eb ;
}

unsigned Maemo::Timed::Event::getMaximalButtonAmount()
{
  return Maemo::Timed::Max_Number_of_App_Buttons ;
}

unsigned Maemo::Timed::Event::getSysButtonAmount()
{
  return Maemo::Timed::Number_of_Sys_Buttons ;
}

Maemo::Timed::Event::Recurrence & Maemo::Timed::Event::addRecurrence()
{
  Maemo::Timed::Event::Recurrence *er = new Maemo::Timed::Event::Recurrence ;
  Maemo::Timed::event_recurrence_pimple_t *pr = new Maemo::Timed::event_recurrence_pimple_t ;
  er->p = pr ;
  pr->recurrence_no = p->r.size() ;
  pr->ptr = std::auto_ptr<Recurrence> (er) ;
  pr->eio = & p->eio ;
  p->r.push_back(pr) ;
  p->eio.recrs.resize(pr->recurrence_no+1) ;
  return *er ;
}

void Maemo::Timed::Event::setAlarmFlag()
{
  p->eio.flags |= EventFlags::Alarm ;
}

void Maemo::Timed::Event::setTriggerIfMissedFlag()
{
  p->eio.flags |= EventFlags::Trigger_If_Missed ;
}

void Maemo::Timed::Event::setUserModeFlag()
{
  p->eio.flags |= EventFlags::User_Mode ;
}

void Maemo::Timed::Event::setAlignedSnoozeFlag()
{
  p->eio.flags |= EventFlags::Aligned_Snooze ;
}

void Maemo::Timed::Event::setReminderFlag()
{
  p->eio.flags |= EventFlags::Reminder ;
}

void Maemo::Timed::Event::setBootFlag()
{
  p->eio.flags |= EventFlags::Boot ;
}

void Maemo::Timed::Event::setKeepAliveFlag()
{
  p->eio.flags |= EventFlags::Keep_Alive ;
}

void Maemo::Timed::Event::setSingleShotFlag()
{
  p->eio.flags |= EventFlags::Single_Shot ;
}

void Maemo::Timed::Event::setBackupFlag()
{
  log_debug("backup flag set for event, to be implemented yet: nothing wrong") ;
  p->eio.flags |= EventFlags::Backup ;
}

void Maemo::Timed::Event::setFakeFlag()
{
  setKeepAliveFlag() ;
  qDebug() << "The method" << __PRETTY_FUNCTION__ << "is deprecated and will be removed, please use setKeepAliveFlag() instead!" ;
}

void Maemo::Timed::Event::suppressTimeoutSnooze()
{
  p->eio.flags |= EventFlags::Suppress0 ;
}

void Maemo::Timed::Event::hideSnoozeButton1()
{
  p->eio.flags |= EventFlags::Hide1 ;
}

void Maemo::Timed::Event::hideCancelButton2()
{
  p->eio.flags |= EventFlags::Hide2 ;
}

void Maemo::Timed::Event::setMaximalTimeoutSnoozeCounter(int tsz_max_counter)
{
  p->eio.tsz_max = tsz_max_counter ;
}

void Maemo::Timed::Event::setTimeoutSnooze(int tsz_length)
{
  if(tsz_length<10)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  p->eio.tsz_length = tsz_length ;
}

void Maemo::Timed::Event::setAttribute(const QString &key, const QString &value)
{
  set_attribute(__PRETTY_FUNCTION__, p->eio.attr, key, value) ;
}

static void add_cred_modifier(QVector<Maemo::Timed::cred_modifier_io_t> &x, const QString &token, bool accrue)
{
  int i = x.size() ;
  x.resize(i+1) ;
  x[i].token = token ;
  x[i].accrue = accrue ;
}

void Maemo::Timed::Event::credentialDrop(const QString &token)
{
  add_cred_modifier(p->eio.cred_modifiers, token, false) ;
}

void Maemo::Timed::Event::credentialAccrue(const QString &token)
{
  add_cred_modifier(p->eio.cred_modifiers, token, true) ;
}

Maemo::Timed::event_pimple_t::~event_pimple_t()
{
  for(unsigned i=0; i<a.size(); ++i)
    delete a[i] ;
  for(unsigned i=0; i<b.size(); ++i)
    delete b[i] ;
  for(unsigned i=0; i<r.size(); ++i)
    delete r[i] ;
}

void Maemo::Timed::Event::Action::setAttribute(const QString &key, const QString &value)
{
  set_attribute(__PRETTY_FUNCTION__, p->aio()->attr, key, value) ;
}
void Maemo::Timed::Event::Action::runCommand()
{
  p->aio()->flags |= ActionFlags::Run_Command ;
}
void Maemo::Timed::Event::Action::runCommand(const QString &cmd)
{
  runCommand() ;
  setAttribute("COMMAND", cmd) ;
}
void Maemo::Timed::Event::Action::runCommand(const QString &cmd, const QString &user)
{
  runCommand(cmd) ;
  setAttribute("USER", user) ;
}
void Maemo::Timed::Event::Action::dbusMethodCall()
{
  p->aio()->flags |= ActionFlags::DBus_Method ;
}
void Maemo::Timed::Event::Action::dbusMethodCall(const QString &srv, const QString &mtd, const QString &obj, const QString &ifc)
{
  dbusMethodCall() ;
  setAttribute("DBUS_SERVICE", srv) ;
  setAttribute("DBUS_METHOD", mtd) ;
  setAttribute("DBUS_PATH", obj) ;
  if(!ifc.isEmpty())
    setAttribute("DBUS_INTERFACE", ifc) ;
}
void Maemo::Timed::Event::Action::dbusSignal()
{
  p->aio()->flags |= ActionFlags::DBus_Signal ;
}
void Maemo::Timed::Event::Action::dbusSignal(QString obj, QString sig, QString ifc)
{
  dbusSignal() ;
  setAttribute("DBUS_PATH", obj) ;
  setAttribute("DBUS_SIGNAL", sig) ;
  setAttribute("DBUS_INTERFACE", ifc) ;
}
void Maemo::Timed::Event::Action::setSendCookieFlag()
{
  p->aio()->flags |= ActionFlags::Send_Cookie ;
}
void Maemo::Timed::Event::Action::setSendAttributesFlag()
{
  p->aio()->flags |= ActionFlags::Send_Action_Attributes ;
}
void Maemo::Timed::Event::Action::setSendEventAttributesFlag()
{
  p->aio()->flags |= ActionFlags::Send_Event_Attributes ;
}
void Maemo::Timed::Event::Action::setUseSystemBusFlag()
{
  p->aio()->flags |= ActionFlags::Use_System_Bus ;
}
void Maemo::Timed::Event::Action::whenQueued()
{
  p->aio()->flags |= ActionFlags::State_Queued ;
}
void Maemo::Timed::Event::Action::whenDue()
{
  p->aio()->flags |= ActionFlags::State_Due ;
}
void Maemo::Timed::Event::Action::whenMissed()
{
  p->aio()->flags |= ActionFlags::State_Missed ;
}
void Maemo::Timed::Event::Action::whenTriggered()
{
  p->aio()->flags |= ActionFlags::State_Triggered ;
}
void Maemo::Timed::Event::Action::whenSnoozed()
{
  p->aio()->flags |= ActionFlags::State_Snoozed ;
}
void Maemo::Timed::Event::Action::whenServed()
{
  p->aio()->flags |= ActionFlags::State_Served ;
}
void Maemo::Timed::Event::Action::whenAborted()
{
  p->aio()->flags |= ActionFlags::State_Aborted ;
}
void Maemo::Timed::Event::Action::whenFailed()
{
  p->aio()->flags |= ActionFlags::State_Failed ;
}
void Maemo::Timed::Event::Action::whenFinalized()
{
  p->aio()->flags |= ActionFlags::State_Finalized ;
}
void Maemo::Timed::Event::Action::whenTranquil()
{
  p->aio()->flags |= ActionFlags::State_Tranquil ;
}
void Maemo::Timed::Event::Action::whenButton(const Event::Button &x)
{
  if(p->eio != x.p->eio)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  int xn = x.p->button_no ;
  p->aio()->flags |= ActionFlags::State_App_Button_1 << xn ;
}
void Maemo::Timed::Event::Action::whenSysButton(int x)
{
  if(x<0 || x>Maemo::Timed::Number_of_Sys_Buttons)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  p->aio()->flags |= ActionFlags::State_Sys_Button_0 << x ;
}

void Maemo::Timed::Event::Action::credentialDrop(const QString &token)
{
  add_cred_modifier(p->aio()->cred_modifiers, token, false) ;
}

void Maemo::Timed::Event::Action::credentialAccrue(const QString &token)
{
  add_cred_modifier(p->aio()->cred_modifiers, token, true) ;
}

void Maemo::Timed::Event::Button::setSnooze(int value)
{
  if(value<10)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  p->bio()->snooze = value ;
}

void Maemo::Timed::Event::Button::setSnoozeDefault()
{
  p->bio()->snooze = +1 ;
}

void Maemo::Timed::Event::Button::setAttribute(const QString &key, const QString &value)
{
  set_attribute(__PRETTY_FUNCTION__, p->bio()->attr, key, value) ;
}
#include <QDebug>
namespace Maemo
{
  namespace Timed
  {
#define XXX qDebug()<<__FILE__<<__LINE__<<"mons"<<R->mons<<"mday"<<R->mday ;
    bool Event::Recurrence::isEmpty() const
    {
      const recurrence_io_t *R = p->rio() ;
      XXX ;
      const uint32_t full_week = 0177 ;
      XXX ;
      if((R->wday&full_week)==0)
        return true ;
      XXX ;
      const uint64_t any_mins = (1ull<<60)-1 ;
      XXX ;
      if((R->mins&any_mins)==0)
        return true ;
      XXX ;
      const uint32_t any_hour = (1u<<24)-1 ;
      XXX ;
      if((R->hour&any_hour)==0)
        return true ;
      XXX ;
      const uint32_t m31 = /* d-o -ay -m- m-j*/ 05325 ;
      XXX ;
      const uint32_t m30 = /* dno say jma m-j*/ 07775 ;
      XXX ;
      const uint32_t d30 = 0x7FFFFFFF ; // 0, 1..30
      XXX ;
      const uint32_t d29 = 0x3FFFFFFF ; // 0, 1..29
      XXX ;
      if(R->mday==0 || R->mons==0)
        return true ;
      XXX ;
      if(R->mons&m31) /* at least one long month, any day then */
        return false ;
      XXX ;
      if((R->mons&m30) && (R->mday&d30)) /* 1..30 in a non-february */
        return false ;
      XXX ;
      if(R->mday & d29) /* 1..29, any month */
        return false ;
      XXX ;
      return true ;
    }
    void Event::Recurrence::addMonth(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 1, 12) ;
      p->rio()->mons |= 1u << (x-1) ;
    }
    void Event::Recurrence::everyMonth()
    {
      p->rio()->mons = 07777 ;
    }
    void Event::Recurrence::addDayOfMonth(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 1, 31) ;
      p->rio()->mday |= 1u << x ;
    }
    void Event::Recurrence::addLastDayOfMonth()
    {
      p->rio()->mday |= 1 << 0 ;
    }
    void Event::Recurrence::everyDayOfMonth()
    {
      p->rio()->mday = 0xFFFFFFFF ;
    }
    void Event::Recurrence::addDayOfWeek(int x)
    {
      log_debug("IN  x=%d wday=0%o", x, p->rio()->wday) ;
      check_interval(__PRETTY_FUNCTION__, x, 0, 7) ;
      p->rio()->wday |= 1u << (x==7 ? 0 : x) ;
      log_debug("OUT x=%d wday=0%o", x, p->rio()->wday) ;
    }
    void Event::Recurrence::everyDayOfWeek()
    {
      const uint32_t full_week = 0177 ;
      p->rio()->wday = full_week ;
    }
    void Event::Recurrence::addHour(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 0, 23) ;
      p->rio()->hour |= 1u << x ;
    }
    void Event::Recurrence::addMinute(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 0, 59) ;
      p->rio()->mins |= 1ull << x ;
    }
  }
}

QVariant Maemo::Timed::Event::dbus_output(const char *) const
{
  return QVariant::fromValue(p->eio) ;
}

Maemo::Timed::Event::List::List()
{
  p = new event_list_pimple_t ;
}

Maemo::Timed::Event::List::~List()
{
  delete p ;
}

Maemo::Timed::Event & Maemo::Timed::Event::List::append()
{
  Event *e = new Event ;
  p->events.push_back(e) ;
  return *e ;
}

QVariant Maemo::Timed::Event::List::dbus_output() const
{
  event_list_io_t eeio ;
  unsigned n = p->events.size() ;
  eeio.ee.resize(n) ;
  for(unsigned i=0; i<n; ++i)
    eeio.ee[i] = p->events[i]->p->eio ;
  return QVariant::fromValue(eeio) ;
}

Maemo::Timed::event_list_pimple_t::~event_list_pimple_t()
{
  for(unsigned i=0; i<events.size(); ++i)
    delete events[i] ;
}

int Maemo::Timed::Event::check(QString *err, bool exc) const
{
  try
  {
  }
  catch(const char *message)
  {
  }
}
