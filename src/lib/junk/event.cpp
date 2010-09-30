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
#include <QDBusMetaType>

#include <timed/event.h>
#include <timed/action.h>
#include <timed/button.h>
#include <timed/exception.h>
#include <timed/recurrence.h>
#include <timed/interface.h>
#include <timed/qmacro.h>

#include "flags.h"

int Maemo::Timed::Event::which_button(const Maemo::Timed::EventButton &B) const
{
  int res = -1 ;
  for(unsigned i=0; res<0 && i<b.size(); ++i)
    if(b[i]==&B)
      res = i ;
  return res ;
}

void Maemo::Timed::Event::set_attribute(const char *pretty, QMap<QString,QString> &map, const QString &key, const QString &value)
{
  if(key.isEmpty())
    throw Exception(pretty, "empty attrubute key") ;
  if(value.isEmpty())
    throw Exception(pretty, "empty attribute value") ;
  if(map.count(key))
    throw Exception(pretty, "attribute key already defined") ;
  map.insert(key, value) ;
}

Maemo::Timed::Event::Event()
{
  ticker = 0 ;
  y = mo = d = h = mi = 0 ;
  flags = 0 ;
}

void Maemo::Timed::Event::setTime(unsigned year, unsigned month, unsigned day, unsigned hour, unsigned minute)
{
  y = year ;
  mo = month ;
  d = day ;
  h = hour ;
  mi = minute ;
}

void Maemo::Timed::Event::setTimezone(const QString &timezone)
{
  tz = timezone ;
}

Maemo::Timed::EventAction & Maemo::Timed::Event::addAction()
{
  Maemo::Timed::EventAction *ea = new Maemo::Timed::EventAction ;
  a.push_back(ea) ;
  return *ea ;
}

Maemo::Timed::EventButton & Maemo::Timed::Event::addButton()
{
  setReminderFlag() ;
  if(b.size() >= Max_Number_of_App_Buttons)
    throw Exception(__PRETTY_FUNCTION__, "too many application buttons") ;
  Maemo::Timed::EventButton *eb = new Maemo::Timed::EventButton(/*this*/) ;
  b.push_back(eb) ;
  return *eb ;
}

Maemo::Timed::EventRecurrence & Maemo::Timed::Event::addRecurrence()
{
  EventRecurrence *er = new EventRecurrence ;
  r.push_back(er) ;
  return *er ;
}

void Maemo::Timed::Event::setAlarmFlag()
{
  flags |= EventFlags::Alarm ;
}

void Maemo::Timed::Event::setTriggerIfMissedFlag()
{
  flags |= EventFlags::Trigger_If_Missed ;
}

void Maemo::Timed::Event::setUserModeFlag()
{
  flags |= EventFlags::User_Mode ;
}

void Maemo::Timed::Event::setAlignedSnoozeFlag()
{
  flags |= EventFlags::Aligned_Snooze ;
}

void Maemo::Timed::Event::setReminderFlag()
{
  flags |= EventFlags::Reminder ;
}

void Maemo::Timed::Event::setBootFlag()
{
  flags |= EventFlags::Boot ;
}

void Maemo::Timed::Event::setAttribute(const QString &key, const QString &value)
{
  set_attribute(__PRETTY_FUNCTION__, attr, key, value) ;
}

Maemo::Timed::Event::~Event()
{
  for(unsigned i=0; i<a.size(); ++i)
    delete a[i] ;
  for(unsigned i=0; i<b.size(); ++i)
    delete b[i] ;
  for(unsigned i=0; i<r.size(); ++i)
    delete r[i] ;
}

#if 0
QDBusArgument & operator<<(QDBusArgument &out, const Maemo::Timed::EventAction &a)
{
}
QDBusArgument & operator<<(QDBusArgument &out, const Maemo::Timed::EventButton &b)
{
}
QDBusArgument & operator<<(QDBusArgument &out, const Maemo::Timed::EventRecurrence &r)
{
}
#endif

#if 0
template<typename T>
static inline QDBusArgument & operator<<(QDBusArgument &out, const std::vector<T*> &v)
{
  out.beginArray(qMetaTypeId<T>()) ;
  for(typename std::vector<T*>::const_iterator it=v.begin(); it!=v.end(); ++it)
    out << **it ;
  out.endArray() ;
  return out ;
}
#endif

QDBusArgument & operator<<(QDBusArgument &out, const Maemo::Timed::Event &x)
{
  return out << event_io_t(x) ;
}

#if 0
QDBusArgument & operator>>(const QDBusArgument &in, Maemo::Timed::Event &)
{
  // should be never called
  return in ;
}
#endif

Maemo::Timed::Event::event_initializer_t event_initializer ;
Maemo::Timed::Event::event_initializer_t::event_initializer_t()
{
  register_metatype(Maemo::Timed::Event) ;
  register_metatype(Maemo::Timed::EventAction) ;
  register_metatype(Maemo::Timed::EventButton) ;
  register_metatype(Maemo::Timed::EventRecurrence) ;
}
