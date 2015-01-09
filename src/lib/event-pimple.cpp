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
#include "../common/log.h"

#include "../server/flags.h"
#include "exception.h"
#include "qmacro.h"

#include "event-pimple.h"
#include "../server/timeutil.h"

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

Maemo::Timed::event_pimple_t::event_pimple_t(const event_io_t &e_io)
  : eio(e_io)
{
  for(int ai = 0; ai < eio.actions.count(); ++ai)
  {
    Maemo::Timed::event_action_pimple_t *pa = new Maemo::Timed::event_action_pimple_t ;
    pa->action_no = ai ;
    pa->eio = & eio ;
    a.push_back(pa) ;
  }

  for(int bi = 0; bi < eio.buttons.count(); ++bi)
  {
    Maemo::Timed::event_button_pimple_t *pb = new Maemo::Timed::event_button_pimple_t ;
    pb->button_no = bi ;
    pb->eio = & eio ;
    b.push_back(pb) ;
  }

  for(int ri = 0; ri < eio.recrs.count(); ++ri)
  {
    Maemo::Timed::event_recurrence_pimple_t *pr = new Maemo::Timed::event_recurrence_pimple_t ;
    pr->recurrence_no = ri ;
    pr->eio = & eio ;
    r.push_back(pr) ;
  }
}

Maemo::Timed::Event::Event()
{
  p = new event_pimple_t ;
}

Maemo::Timed::Event::Event(const event_io_t &eio)
{
  p = new event_pimple_t(eio) ;

  if(p->b.size() > Max_Number_of_App_Buttons)
    throw Exception(__PRETTY_FUNCTION__, "too many application buttons") ;
  if(p->b.size() > 0)
    setReminderFlag() ;
}

Maemo::Timed::Event::~Event()
{
  delete p ;
}

void Maemo::Timed::Event::setTicker(time_t ticker)
{
  p->eio.ticker = ticker ;
}

time_t Maemo::Timed::Event::ticker() const
{
  return p->eio.ticker ;
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

unsigned Maemo::Timed::Event::year() const
{
  return p->eio.t_year ;
}

unsigned Maemo::Timed::Event::month() const
{
  return p->eio.t_month ;
}

unsigned Maemo::Timed::Event::day() const
{
  return p->eio.t_day ;
}

unsigned Maemo::Timed::Event::hour() const
{
  return p->eio.t_hour ;
}

unsigned Maemo::Timed::Event::minute() const
{
  return p->eio.t_minute ;
}

void Maemo::Timed::Event::setTimezone(const QString &timezone)
{
  p->eio.t_zone = timezone ;
}

const QString & Maemo::Timed::Event::timezone() const
{
  return p->eio.t_zone ;
}

Maemo::Timed::Event::Action * Maemo::Timed::Event::getAction(event_action_pimple_t *pa) const
{
  if(pa == NULL)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;

  Maemo::Timed::Event::Action *ea = pa->ptr.get() ;
  
  if(ea == NULL)
  {
    ea = new Maemo::Timed::Event::Action ;
    ea->p = pa ;
    pa->ptr = std::auto_ptr<Action> (ea) ;
  }
  return ea ;
}

Maemo::Timed::Event::Action & Maemo::Timed::Event::addAction()
{
  Maemo::Timed::event_action_pimple_t *pa = new Maemo::Timed::event_action_pimple_t ;
  pa->action_no = p->a.size() ;
  pa->eio = & p->eio ;
  p->a.push_back(pa) ;
  p->eio.actions.resize(pa->action_no+1) ;
  return *getAction(pa) ;
}

int Maemo::Timed::Event::actionsCount() const
{
  return p->a.size() ;
}

Maemo::Timed::Event::Action & Maemo::Timed::Event::action(int index)
{
  return *getAction(p->a.at(index)) ;
}

void Maemo::Timed::Event::removeAction(int index)
{
  p->eio.actions.remove(index) ;

  delete p->a[index] ;
  p->a.erase(p->a.begin() + index) ;
  for(unsigned i = index; i < p->a.size(); ++i)
  {
    p->a[i]->action_no = i ;
  }
}

void Maemo::Timed::Event::clearActions()
{
  p->eio.actions.clear() ;
  for(unsigned i=0; i<p->a.size(); ++i)
    delete p->a[i] ;
  p->a.clear() ;
}

Maemo::Timed::Event::Button * Maemo::Timed::Event::getButton(event_button_pimple_t *pb) const
{
  if(pb == NULL)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;

  Maemo::Timed::Event::Button *eb = pb->ptr.get() ;
  
  if(eb == NULL)
  {
    eb = new Maemo::Timed::Event::Button ;
    eb->p = pb ;
    pb->ptr = std::auto_ptr<Button> (eb) ;
  }
  return eb ;
}

Maemo::Timed::Event::Button & Maemo::Timed::Event::addButton()
{
  setReminderFlag() ;
  if(p->b.size() >= Max_Number_of_App_Buttons)
    throw Exception(__PRETTY_FUNCTION__, "too many application buttons") ;
  Maemo::Timed::event_button_pimple_t *pb = new Maemo::Timed::event_button_pimple_t ;
  pb->button_no = p->b.size() ;
  pb->eio = & p->eio ;
  p->b.push_back(pb) ;
  p->eio.buttons.resize(pb->button_no+1) ;
  return *getButton(pb) ;
}
int Maemo::Timed::Event::buttonsCount() const
{
  return p->b.size() ;
}

void Maemo::Timed::Event::removeButton(int index)
{
  vector<event_action_pimple_t*>::iterator a_it;
  for(a_it = p->a.begin(); a_it != p->a.end(); ++a_it)
  {
    Maemo::Timed::Event::Action *ea = getAction(*a_it) ;
    ea->clearWhenButton(*getButton(p->b[index])) ;
  }

  p->eio.buttons.remove(index) ;

  delete p->b[index] ;
  p->b.erase(p->b.begin() + index) ;
  for(unsigned i = index; i < p->b.size(); ++i)
  {
    p->b[i]->button_no = i ;
  }
}

void Maemo::Timed::Event::clearButtons()
{
  vector<event_action_pimple_t*>::iterator a_it;
  vector<event_button_pimple_t*>::iterator b_it;
  for(a_it = p->a.begin(); a_it != p->a.end(); ++a_it)
  {
    Maemo::Timed::Event::Action *ea = getAction(*a_it) ;
    for(b_it = p->b.begin(); b_it != p->b.end(); ++b_it)
    {
      ea->clearWhenButton(*getButton(*b_it)) ;
    }
  }
  p->eio.buttons.clear() ;
  for(unsigned i=0; i<p->b.size(); ++i)
    delete p->b[i] ;
  p->b.clear() ;
  clearReminderFlag() ;
}

Maemo::Timed::Event::Button & Maemo::Timed::Event::button(int index)
{
  return *getButton(p->b.at(index)) ;
}

unsigned Maemo::Timed::Event::getMaximalButtonAmount()
{
  return Maemo::Timed::Max_Number_of_App_Buttons ;
}

unsigned Maemo::Timed::Event::getSysButtonAmount()
{
  return Maemo::Timed::Number_of_Sys_Buttons ;
}

Maemo::Timed::Event::Recurrence * Maemo::Timed::Event::getRecurrence(event_recurrence_pimple_t *pr) const
{
  if(pr == NULL)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;

  Maemo::Timed::Event::Recurrence *er = pr->ptr.get() ;

  if(er == NULL)
  {
    er = new Maemo::Timed::Event::Recurrence ;
    er->p = pr ;
    pr->ptr = std::auto_ptr<Recurrence> (er) ;
  }
  return er ;
}

Maemo::Timed::Event::Recurrence & Maemo::Timed::Event::addRecurrence()
{
  Maemo::Timed::event_recurrence_pimple_t *pr = new Maemo::Timed::event_recurrence_pimple_t ;
  pr->recurrence_no = p->r.size() ;
  pr->eio = & p->eio ;
  p->r.push_back(pr) ;
  p->eio.recrs.resize(pr->recurrence_no+1) ;
  return *getRecurrence(pr) ;
}

int Maemo::Timed::Event::recurrencesCount() const
{
  return p->r.size() ;
}

void Maemo::Timed::Event::removeRecurrence(int index)
{
  p->eio.recrs.remove(index) ;

  delete p->r[index] ;
  p->r.erase(p->r.begin() + index) ;
  for(unsigned i = index; i < p->r.size(); ++i)
  {
    p->r[i]->recurrence_no = i ;
  }
}

void Maemo::Timed::Event::clearRecurrences()
{
  p->eio.recrs.clear() ;

  for(unsigned i = 0; i < p->r.size(); ++i)
  {
    delete p->r[i] ;
  }
  p->r.clear() ;
}

Maemo::Timed::Event::Recurrence & Maemo::Timed::Event::recurrence(int index)
{
  return *getRecurrence(p->r.at(index)) ;
}

void Maemo::Timed::Event::setAlarmFlag()
{
  p->eio.flags |= EventFlags::Alarm ;
}

bool Maemo::Timed::Event::alarmFlag() const
{
  return p->eio.flags & EventFlags::Alarm ;
}

void Maemo::Timed::Event::clearAlarmFlag()
{
  p->eio.flags &= ~EventFlags::Alarm ;
}

void Maemo::Timed::Event::setTriggerIfMissedFlag()
{
  p->eio.flags |= EventFlags::Trigger_If_Missed ;
}

bool Maemo::Timed::Event::triggerIfMissedFlag() const
{
  return p->eio.flags & EventFlags::Trigger_If_Missed ;
}

void Maemo::Timed::Event::clearTriggerIfMissedFlag()
{
  p->eio.flags &= ~EventFlags::Trigger_If_Missed ;
}

void Maemo::Timed::Event::setTriggerWhenAdjustingFlag()
{
  p->eio.flags |= EventFlags::Trigger_When_Adjusting ;
}

bool Maemo::Timed::Event::triggerWhenAdjustingFlag() const
{
  return p->eio.flags & EventFlags::Trigger_When_Adjusting ;
}

void Maemo::Timed::Event::clearTriggerWhenAdjustingFlag()
{
  p->eio.flags &= ~EventFlags::Trigger_When_Adjusting ;
}

void Maemo::Timed::Event::setTriggerWhenSettingsChangedFlag()
{
  p->eio.flags |= EventFlags::Trigger_When_Settings_Changed ;
}

bool Maemo::Timed::Event::triggerWhenSettingsChangedFlag() const
{
  return p->eio.flags & EventFlags::Trigger_When_Settings_Changed ;
}

void Maemo::Timed::Event::clearTriggerWhenSettingsChangedFlag()
{
  p->eio.flags &= ~EventFlags::Trigger_When_Settings_Changed ;
}

void Maemo::Timed::Event::setUserModeFlag()
{
  p->eio.flags |= EventFlags::User_Mode ;
}

bool Maemo::Timed::Event::userModeFlag() const
{
  return p->eio.flags & EventFlags::User_Mode ;
}

void Maemo::Timed::Event::clearUserModeFlag()
{
  p->eio.flags &= ~EventFlags::User_Mode ;
}

void Maemo::Timed::Event::setAlignedSnoozeFlag()
{
  p->eio.flags |= EventFlags::Aligned_Snooze ;
}

bool Maemo::Timed::Event::alignedSnoozeFlag() const
{
  return p->eio.flags & EventFlags::Aligned_Snooze ;
}

void Maemo::Timed::Event::clearAlignedSnoozeFlag()
{
  p->eio.flags &= ~EventFlags::Aligned_Snooze ;
}

void Maemo::Timed::Event::setReminderFlag()
{
  p->eio.flags |= EventFlags::Reminder ;
}

bool Maemo::Timed::Event::reminderFlag() const
{
  return p->eio.flags & EventFlags::Reminder ;
}

void Maemo::Timed::Event::clearReminderFlag()
{
  p->eio.flags &= ~EventFlags::Reminder ;
}

void Maemo::Timed::Event::setBootFlag()
{
  p->eio.flags |= EventFlags::Boot ;
}

bool Maemo::Timed::Event::bootFlag() const
{
  return p->eio.flags & EventFlags::Boot ;
}

void Maemo::Timed::Event::clearBootFlag()
{
  p->eio.flags &= ~EventFlags::Boot ;
}

void Maemo::Timed::Event::setKeepAliveFlag()
{
  p->eio.flags |= EventFlags::Keep_Alive ;
}

bool Maemo::Timed::Event::keepAliveFlag() const
{
  return p->eio.flags & EventFlags::Keep_Alive ;
}

void Maemo::Timed::Event::clearKeepAliveFlag()
{
  p->eio.flags &= ~EventFlags::Keep_Alive ;
}

void Maemo::Timed::Event::setSingleShotFlag()
{
  p->eio.flags |= EventFlags::Single_Shot ;
}

bool Maemo::Timed::Event::singleShotFlag() const
{
  return p->eio.flags & EventFlags::Single_Shot ;
}

void Maemo::Timed::Event::clearSingleShotFlag()
{
  p->eio.flags &= ~EventFlags::Single_Shot ;
}

void Maemo::Timed::Event::setBackupFlag()
{
  p->eio.flags |= EventFlags::Backup ;
}

bool Maemo::Timed::Event::backupFlag() const
{
  return p->eio.flags & EventFlags::Backup ;
}

void Maemo::Timed::Event::clearBackupFlag()
{
  p->eio.flags &= ~EventFlags::Backup ;
}

void Maemo::Timed::Event::setFakeFlag()
{
  setKeepAliveFlag() ;
  log_error("The method %s' is deprecated, please use setKeepAliveFlag() instead!", __PRETTY_FUNCTION__) ;
}

void Maemo::Timed::Event::suppressTimeoutSnooze()
{
  p->eio.flags |= EventFlags::Suppress0 ;
}

bool Maemo::Timed::Event::doSuppressTimeoutSnooze() const
{
  return p->eio.flags & EventFlags::Suppress0 ;
}

void Maemo::Timed::Event::allowTimeoutSnooze()
{
  p->eio.flags &= ~EventFlags::Suppress0 ;
}

void Maemo::Timed::Event::hideSnoozeButton1()
{
  p->eio.flags |= EventFlags::Hide1 ;
}

bool Maemo::Timed::Event::doHideSnoozeButton1() const
{
  return p->eio.flags & EventFlags::Hide1 ;
}

void Maemo::Timed::Event::showSnoozeButton1()
{
  p->eio.flags &= ~EventFlags::Hide1 ;
}

void Maemo::Timed::Event::hideCancelButton2()
{
  p->eio.flags |= EventFlags::Hide2 ;
}

bool Maemo::Timed::Event::doHideCancelButton2() const
{
  return p->eio.flags & EventFlags::Hide2 ;
}

void Maemo::Timed::Event::showCancelButton2()
{
  p->eio.flags &= ~EventFlags::Hide2 ;
}

void Maemo::Timed::Event::setMaximalTimeoutSnoozeCounter(int tsz_max_counter)
{
  p->eio.tsz_max = tsz_max_counter ;
}

int Maemo::Timed::Event::maximalTimeoutSnoozeCounter() const
{
  return p->eio.tsz_max ;
}

void Maemo::Timed::Event::setTimeoutSnooze(int tsz_length)
{
  if(tsz_length<10)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  p->eio.tsz_length = tsz_length ;
}

int Maemo::Timed::Event::timeoutSnoozeLenght() const
{
  return p->eio.tsz_length ;
}

void Maemo::Timed::Event::setAttribute(const QString &key, const QString &value)
{
  set_attribute(__PRETTY_FUNCTION__, p->eio.attr, key, value) ;
}

void Maemo::Timed::Event::removeAttribute(const QString &key)
{
  p->eio.attr.txt.remove(key) ;
}

void Maemo::Timed::Event::clearAttributes()
{
  p->eio.attr.txt.clear() ;
}

const QMap<QString, QString> & Maemo::Timed::Event::attributes() const
{
  return p->eio.attr.txt ;
}

static void add_cred_modifier(QVector<Maemo::Timed::cred_modifier_io_t> &x, const QString &token, bool accrue)
{
  int i = x.size() ;
  x.resize(i+1) ;
  x[i].token = token ;
  x[i].accrue = accrue ;
}

static void remove_cred_modifier(QVector<Maemo::Timed::cred_modifier_io_t> &x, const QString &token, bool accrue)
{
  Maemo::Timed::cred_modifier_io_t lookFor ;
  lookFor.token = token ;
  lookFor.accrue = accrue ;
  for(int i = x.indexOf(lookFor); i != -1; i = x.indexOf(lookFor))
  {
    x.remove(i) ;
  }
}

static QStringList filterCredTokens(QVector<Maemo::Timed::cred_modifier_io_t> &cred_modifiers, bool accrue)
{
  QStringList ret ;
  QVector<Maemo::Timed::cred_modifier_io_t>::const_iterator it ;
  for(it = cred_modifiers.begin() ; it != cred_modifiers.end() ; ++it)
  {
    if(accrue == it->accrue)
    {
      ret << it->token ;
    }
  }
  return ret;
}

void Maemo::Timed::Event::credentialDrop(const QString &token)
{
  add_cred_modifier(p->eio.cred_modifiers, token, false) ;
}

QStringList Maemo::Timed::Event::droppedCredentials() const
{
  return filterCredTokens(p->eio.cred_modifiers, false) ;
}

void Maemo::Timed::Event::removeCredentialDrop(const QString &token)
{
  remove_cred_modifier(p->eio.cred_modifiers, token, false) ;
}

void Maemo::Timed::Event::credentialAccrue(const QString &token)
{
  add_cred_modifier(p->eio.cred_modifiers, token, true) ;
}

QStringList Maemo::Timed::Event::accruedCredentials() const
{
  return filterCredTokens(p->eio.cred_modifiers, true) ;
}

void Maemo::Timed::Event::removeCredentialAccrue(const QString &token)
{
  remove_cred_modifier(p->eio.cred_modifiers, token, true) ;
}

void Maemo::Timed::Event::clearCredentialModifiers()
{
  p->eio.cred_modifiers.clear() ;
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
void Maemo::Timed::Event::Action::removeAttribute(const QString &key)
{
  p->aio()->attr.txt.remove(key) ;
}
void Maemo::Timed::Event::Action::clearAttributes()
{
  p->aio()->attr.txt.clear() ;
}
const QMap<QString, QString> & Maemo::Timed::Event::Action::attributes() const
{
  return p->aio()->attr.txt ;
}
bool Maemo::Timed::Event::Action::runCommandFlag() const
{
  return p->aio()->flags & ActionFlags::Run_Command ;
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
const QString Maemo::Timed::Event::Action::runCommandLine() const
{
  return p->aio()->attr.txt.value("COMMAND") ;
}
void Maemo::Timed::Event::Action::runCommand(const QString &cmd, const QString &user)
{
  runCommand(cmd) ;
  setAttribute("USER", user) ;
}
const QString Maemo::Timed::Event::Action::runCommandUser() const
{
  return p->aio()->attr.txt.value("USER") ;
}
void Maemo::Timed::Event::Action::removeCommand()
{
  p->aio()->flags &= ~ActionFlags::Run_Command ;
  removeAttribute("COMMAND") ;
  removeAttribute("USER") ;
}
void Maemo::Timed::Event::Action::dbusMethodCall()
{
  p->aio()->flags |= ActionFlags::DBus_Method ;
}
bool Maemo::Timed::Event::Action::dbusMethodCallFlag() const
{
  return p->aio()->flags & ActionFlags::DBus_Method ;
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
const QString Maemo::Timed::Event::Action::dbusMethodCallService() const
{
  return dbusMethodCallFlag() ? p->aio()->attr.txt.value("DBUS_SERVICE") : QString() ;
}
const QString Maemo::Timed::Event::Action::dbusMethodCallMethod() const
{
  return dbusMethodCallFlag() ? p->aio()->attr.txt.value("DBUS_METHOD") : QString() ;
}
const QString Maemo::Timed::Event::Action::dbusMethodCallPath() const
{
  return dbusMethodCallFlag() ? p->aio()->attr.txt.value("DBUS_PATH") : QString() ;
}
const QString Maemo::Timed::Event::Action::dbusMethodCallInterface() const
{
  return dbusMethodCallFlag() ? p->aio()->attr.txt.value("DBUS_INTERFACE") : QString() ;
}
void Maemo::Timed::Event::Action::removeDbusMethodCall()
{
  p->aio()->flags &= ~ActionFlags::DBus_Method ;
  removeAttribute("DBUS_SERVICE") ;
  removeAttribute("DBUS_METHOD") ;
  removeAttribute("DBUS_PATH") ;
  removeAttribute("DBUS_INTERFACE") ;
}
void Maemo::Timed::Event::Action::dbusSignal()
{
  p->aio()->flags |= ActionFlags::DBus_Signal ;
}
bool Maemo::Timed::Event::Action::dbusSignalFlag() const
{
  return p->aio()->flags & ActionFlags::DBus_Signal ;
}
void Maemo::Timed::Event::Action::dbusSignal(QString obj, QString sig, QString ifc)
{
  dbusSignal() ;
  setAttribute("DBUS_PATH", obj) ;
  setAttribute("DBUS_SIGNAL", sig) ;
  setAttribute("DBUS_INTERFACE", ifc) ;
}
const QString Maemo::Timed::Event::Action::dbusSignalPath() const
{
  return dbusSignalFlag() ? p->aio()->attr.txt.value("DBUS_PATH") : QString() ;

}
const QString Maemo::Timed::Event::Action::dbusSignalName() const
{
  return dbusSignalFlag() ? p->aio()->attr.txt.value("DBUS_SIGNAL") : QString() ;

}
const QString Maemo::Timed::Event::Action::dbusSignalInterface() const
{
  return dbusSignalFlag() ? p->aio()->attr.txt.value("DBUS_INTERFACE") : QString() ;
}
void Maemo::Timed::Event::Action::removeDbusSignal()
{
  p->aio()->flags &= ~ActionFlags::DBus_Signal ;
  removeAttribute("DBUS_PATH") ;
  removeAttribute("DBUS_SIGNAL") ;
  removeAttribute("DBUS_INTERFACE") ;
}
void Maemo::Timed::Event::Action::setSendCookieFlag()
{
  p->aio()->flags |= ActionFlags::Send_Cookie ;
}
bool Maemo::Timed::Event::Action::sendCookieFlag() const
{
  return p->aio()->flags & ActionFlags::Send_Cookie ;
}
void Maemo::Timed::Event::Action::clearSendCookieFlag()
{
  p->aio()->flags &= ~ActionFlags::Send_Cookie ;
}
void Maemo::Timed::Event::Action::setSendAttributesFlag()
{
  p->aio()->flags |= ActionFlags::Send_Action_Attributes ;
}
bool Maemo::Timed::Event::Action::sendAttributesFlag() const
{
  return p->aio()->flags & ActionFlags::Send_Action_Attributes ;
}
void Maemo::Timed::Event::Action::clearSendAttributesFlag()
{
  p->aio()->flags &= ~ActionFlags::Send_Action_Attributes ;
}
void Maemo::Timed::Event::Action::setSendEventAttributesFlag()
{
  p->aio()->flags |= ActionFlags::Send_Event_Attributes ;
}
bool Maemo::Timed::Event::Action::sendEventAttributesFlag() const
{
  return p->aio()->flags & ActionFlags::Send_Event_Attributes ;
}
void Maemo::Timed::Event::Action::clearSendEventAttributesFlag()
{
  p->aio()->flags &= ~ActionFlags::Send_Event_Attributes ;
}
void Maemo::Timed::Event::Action::setUseSystemBusFlag()
{
  p->aio()->flags |= ActionFlags::Use_System_Bus ;
}
bool Maemo::Timed::Event::Action::useSystemBusFlag() const
{
  return p->aio()->flags & ActionFlags::Use_System_Bus ;
}
void Maemo::Timed::Event::Action::clearUseSystemBusFlag()
{
  p->aio()->flags &= ~ActionFlags::Use_System_Bus ;
}
void Maemo::Timed::Event::Action::whenQueued()
{
  p->aio()->flags |= ActionFlags::State_Queued ;
}
bool Maemo::Timed::Event::Action::whenQueuedFlag() const
{
  return p->aio()->flags & ActionFlags::State_Queued ;
}
void Maemo::Timed::Event::Action::clearWhenQueuedFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Queued ;
}
void Maemo::Timed::Event::Action::whenDue()
{
  p->aio()->flags |= ActionFlags::State_Due ;
}
bool Maemo::Timed::Event::Action::whenDueFlag() const
{
  return p->aio()->flags & ActionFlags::State_Due ;
}
void Maemo::Timed::Event::Action::clearWhenDueFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Due ;
}
void Maemo::Timed::Event::Action::whenMissed()
{
  p->aio()->flags |= ActionFlags::State_Missed ;
}
bool Maemo::Timed::Event::Action::whenMissedFlag() const
{
  return p->aio()->flags & ActionFlags::State_Missed ;
}
void Maemo::Timed::Event::Action::clearWhenMissedFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Missed ;
}
void Maemo::Timed::Event::Action::whenTriggered()
{
  p->aio()->flags |= ActionFlags::State_Triggered ;
}
bool Maemo::Timed::Event::Action::whenTriggeredFlag() const
{
  return p->aio()->flags & ActionFlags::State_Triggered ;
}
void Maemo::Timed::Event::Action::clearWhenTriggeredFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Triggered ;
}
void Maemo::Timed::Event::Action::whenSnoozed()
{
  p->aio()->flags |= ActionFlags::State_Snoozed ;
}
bool Maemo::Timed::Event::Action::whenSnoozedFlag() const
{
  return p->aio()->flags & ActionFlags::State_Snoozed ;
}
void Maemo::Timed::Event::Action::clearWhenSnoozedFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Snoozed ;
}
void Maemo::Timed::Event::Action::whenServed()
{
  p->aio()->flags |= ActionFlags::State_Served ;
}
bool Maemo::Timed::Event::Action::whenServedFlag() const
{
  return p->aio()->flags & ActionFlags::State_Served ;
}
void Maemo::Timed::Event::Action::clearWhenServedFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Served ;
}
void Maemo::Timed::Event::Action::whenAborted()
{
  p->aio()->flags |= ActionFlags::State_Aborted ;
}
bool Maemo::Timed::Event::Action::whenAbortedFlag() const
{
  return p->aio()->flags & ActionFlags::State_Aborted ;
}
void Maemo::Timed::Event::Action::clearWhenAbortedFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Aborted ;
}
void Maemo::Timed::Event::Action::whenFailed()
{
  p->aio()->flags |= ActionFlags::State_Failed ;
}
bool Maemo::Timed::Event::Action::whenFailedFlag() const
{
  return p->aio()->flags & ActionFlags::State_Failed ;
}
void Maemo::Timed::Event::Action::clearWhenFailedFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Failed ;
}
void Maemo::Timed::Event::Action::whenFinalized()
{
  p->aio()->flags |= ActionFlags::State_Finalized ;
}
bool Maemo::Timed::Event::Action::whenFinalizedFlag() const
{
  return p->aio()->flags & ActionFlags::State_Finalized ;
}
void Maemo::Timed::Event::Action::clearWhenFinalizedFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Finalized ;
}
void Maemo::Timed::Event::Action::whenTranquil()
{
  p->aio()->flags |= ActionFlags::State_Tranquil ;
}
bool Maemo::Timed::Event::Action::whenTranquilFlag() const
{
  return p->aio()->flags & ActionFlags::State_Tranquil ;
}
void Maemo::Timed::Event::Action::clearWhenTranquilFlag()
{
  p->aio()->flags &= ~ActionFlags::State_Tranquil ;
}
void Maemo::Timed::Event::Action::whenButton(const Event::Button &x)
{
  if(p->eio != x.p->eio)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  int xn = x.p->button_no ;
  p->aio()->flags |= ActionFlags::State_App_Button_1 << xn ;
}
QVector<int> Maemo::Timed::Event::Action::whenButtons() const
{
  QVector<int> ret;
  for(int button_no = 0 ; button_no < Max_Number_of_App_Buttons ; ++button_no)
  {
    if(p->aio()->flags & (ActionFlags::State_App_Button_1 << button_no))
    {
      ret << button_no ;
    }
  }
  return ret;
}
void Maemo::Timed::Event::Action::clearWhenButton(const Event::Button &x)
{
  if(p->eio != x.p->eio)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  int xn = x.p->button_no ;
  p->aio()->flags &= ~(ActionFlags::State_App_Button_1 << xn) ;
}
void Maemo::Timed::Event::Action::whenSysButton(int x)
{
  if(x<0 || x>Maemo::Timed::Number_of_Sys_Buttons)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  p->aio()->flags |= ActionFlags::State_Sys_Button_0 << x ;
}
QVector<int> Maemo::Timed::Event::Action::whenSysButtons() const
{
  QVector<int> ret ;
  for(int button_no = 0 ; button_no < Number_of_Sys_Buttons + 1 ; ++button_no)
  {
    if(p->aio()->flags & (ActionFlags::State_Sys_Button_0 << button_no))
    {
      ret << button_no ;
    }
  }
  return ret ;
}
void Maemo::Timed::Event::Action::clearWhenSysButton(int x)
{
  if(x<0 || x>Maemo::Timed::Number_of_Sys_Buttons)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  p->aio()->flags &= ~(ActionFlags::State_Sys_Button_0 << x) ;
}
void Maemo::Timed::Event::Action::credentialDrop(const QString &token)
{
  add_cred_modifier(p->aio()->cred_modifiers, token, false) ;
}
QStringList Maemo::Timed::Event::Action::droppedCredentials() const
{
  return filterCredTokens(p->aio()->cred_modifiers, false) ;
}
void Maemo::Timed::Event::Action::removeCredentialDrop(const QString &token)
{
  remove_cred_modifier(p->aio()->cred_modifiers, token, false) ;
}
void Maemo::Timed::Event::Action::credentialAccrue(const QString &token)
{
  add_cred_modifier(p->aio()->cred_modifiers, token, true) ;
}
QStringList Maemo::Timed::Event::Action::accruedCredentials() const
{
  return filterCredTokens(p->aio()->cred_modifiers, true) ;
}
void Maemo::Timed::Event::Action::removeCredentialAccrue(const QString &token)
{
  remove_cred_modifier(p->aio()->cred_modifiers, token, true) ;
}
void Maemo::Timed::Event::Action::clearCredentialModifiers()
{
  p->aio()->cred_modifiers.clear() ;
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

int Maemo::Timed::Event::Button::snooze() const
{
  return p->bio()->snooze ;
}

void Maemo::Timed::Event::Button::setAttribute(const QString &key, const QString &value)
{
  set_attribute(__PRETTY_FUNCTION__, p->bio()->attr, key, value) ;
}

void Maemo::Timed::Event::Button::removeAttribute(const QString &key)
{
  p->bio()->attr.txt.remove(key) ;
}

void Maemo::Timed::Event::Button::clearAttributes()
{
  p->bio()->attr.txt.clear() ;
}

const QMap<QString, QString> & Maemo::Timed::Event::Button::attributes() const
{
  return p->bio()->attr.txt ;
}

#include <QDebug>
namespace Maemo
{
  namespace Timed
  {
    bool Event::Recurrence::isEmpty() const
    {
      const recurrence_io_t *R = p->rio() ;
      const uint32_t full_week = 0177 ;
      if((R->wday&full_week)==0)
        return true ;
      const uint64_t any_mins = (1ull<<60)-1 ;
      if((R->mins&any_mins)==0)
        return true ;
      const uint32_t any_hour = (1u<<24)-1 ;
      if((R->hour&any_hour)==0)
        return true ;
      const uint32_t m31 = /* d-o -ay -m- m-j*/ 05325 ;
      const uint32_t m30 = /* dno say jma m-j*/ 07775 ;
      const uint32_t d30 = 0x7FFFFFFF ; // 0, 1..30
      const uint32_t d29 = 0x3FFFFFFF ; // 0, 1..29
      if(R->mday==0 || R->mons==0)
        return true ;
      if(R->mons&m31) /* at least one long month, any day then */
        return false ;
      if((R->mons&m30) && (R->mday&d30)) /* 1..30 in a non-february */
        return false ;
      if(R->mday & d29) /* 1..29, any month */
        return false ;
      return true ;
    }
    int Event::Recurrence::months() const
    {
      return p->rio()->mons ;
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
    void Event::Recurrence::removeMonth(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 1, 12) ;
      p->rio()->mons &= ~(1u << (x-1)) ;
    }
    void Event::Recurrence::removeEveryMonth()
    {
      p->rio()->mons = 0 ;
    }
    int Event::Recurrence::daysOfMonth() const
    {
      return p->rio()->mday ;
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
    void Event::Recurrence::removeDayOfMonth(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 1, 31) ;
      p->rio()->mday &= ~(1u << x) ;
    }
    void Event::Recurrence::removeLastDayOfMonth()
    {
      p->rio()->mday &= ~(1 << 0) ;
    }
    void Event::Recurrence::removeEveryDayOfMonth()
    {
      p->rio()->mday = 0 ;
    }
    int Event::Recurrence::daysOfWeek() const
    {
      return p->rio()->wday ;
    }
    void Event::Recurrence::addDayOfWeek(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 0, 7) ;
      p->rio()->wday |= 1u << (x==7 ? 0 : x) ;
    }
    void Event::Recurrence::everyDayOfWeek()
    {
      const uint32_t full_week = 0177 ;
      p->rio()->wday = full_week ;
    }
    void Event::Recurrence::removeDayOfWeek(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 0, 7) ;
      p->rio()->wday &= ~(1u << (x==7 ? 0 : x)) ;
    }
    void Event::Recurrence::removeEveryDayOfWeek()
    {
      p->rio()->wday = 0 ;
    }
    int Event::Recurrence::hours() const
    {
      return p->rio()->hour ;
    }
    void Event::Recurrence::addHour(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 0, 23) ;
      p->rio()->hour |= 1u << x ;
    }
    void Event::Recurrence::removeHour(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 0, 23) ;
      p->rio()->hour &= ~(1u << x) ;
    }
    void Event::Recurrence::removeEveryHour()
    {
      p->rio()->hour = 0 ;
    }
    quint64 Event::Recurrence::minutes() const
    {
      return p->rio()->mins ;
    }
    void Event::Recurrence::addMinute(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 0, 59) ;
      p->rio()->mins |= 1ull << x ;
    }
    void Event::Recurrence::removeMinute(int x)
    {
      check_interval(__PRETTY_FUNCTION__, x, 0, 59) ;
      p->rio()->mins &= ~(1ull << x) ;
    }
    void Event::Recurrence::removeEveryMinute()
    {
      p->rio()->mins = 0ull ;
    }
    bool Event::Recurrence::fillingGapsFlag() const
    {
      return p->rio()->flags & RecurrenceFlags::Fill_Gaps ;
    }
    void Event::Recurrence::setFillingGapsFlag()
    {
      p->rio()->flags |= RecurrenceFlags::Fill_Gaps ;
    }
    void Event::Recurrence::clearFillingGapsFlag()
    {
      p->rio()->flags &= ~RecurrenceFlags::Fill_Gaps ;
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

int Maemo::Timed::Event::List::count() const
{
  return p->events.size() ;
}

Maemo::Timed::Event & Maemo::Timed::Event::List::event(int index)
{
  return *(p->events.at(index)) ;
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

Maemo::Timed::Event::List::List(const event_list_io_t &eeio)
{
  p = new event_list_pimple_t ;
  QVector<event_io_t>::const_iterator it ;
  for(it = eeio.ee.begin(); it != eeio.ee.end(); ++it)
  {
    Event *e = new Event(*it) ;
    p->events.push_back(e) ;
  }
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::Event::Triggers &map)
{
  out.beginMap(QVariant::UInt, QVariant::UInt);
  QMapIterator<uint, uint> iter(map);
  while (iter.hasNext()) {
    iter.next();
    out.beginMapEntry();
    out << iter.key() << iter.value();
    out.endMapEntry();
  }
  out.endMap();
  return out;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::Event::Triggers &map)
{
  map.clear();
  in.beginMap();
  while (!in.atEnd()) {
    uint key;
    uint value;
    in.beginMapEntry();
    in >> key >> value;
    in.endMapEntry();
    map.insert(key, value);
  }
  in.endMap();
  return in;
}

register_qtdbus_metatype(Maemo::Timed::Event::Triggers, 11);

int Maemo::Timed::Event::check(QString *, bool ) const { return 0 ; }
// TODO: get rid of it
