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
#include "reminder.h"

#include "../server/flags.h"
#include "reminder-pimple.h"


Maemo::Timed::Voland::Reminder::Reminder()
{
  p = new reminder_pimple_t ;
}

Maemo::Timed::Voland::Reminder::Reminder(reminder_pimple_t *self)
{
  p = self ;
}

Maemo::Timed::Voland::Reminder::~Reminder()
{
  delete p ;
}

Maemo::Timed::Voland::Reminder & Maemo::Timed::Voland::Reminder::operator=(const Reminder &x)
{
  delete p ;
  p = new reminder_pimple_t ;
  *p = *x.p ;
  return *this ;
}

Maemo::Timed::Voland::Reminder::Reminder(const Reminder &x)
{
  p = new reminder_pimple_t ;
  *p = *x.p ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::Voland::button_io_t &x)
{
  out.beginStructure() ;
  out << x.attr ;
  out.endStructure() ;
  return out ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::Voland::button_io_t &x)
{
  in.beginStructure() ;
  in >> x.attr ;
  in.endStructure() ;
  return in ;
}

QDBusArgument &operator<<(QDBusArgument &out, const Maemo::Timed::Voland::Reminder &x)
{
  out.beginStructure() ;
  out << x.p->cookie << x.p->flags << x.p->attr << x.p->buttons ;
  out.endStructure() ;
  return out ;
}

const QDBusArgument &operator>>(const QDBusArgument &in, Maemo::Timed::Voland::Reminder &x)
{
  in.beginStructure() ;
  in >> x.p->cookie >> x.p->flags >> x.p->attr >> x.p->buttons ;
  in.endStructure() ;
  return in ;
}

unsigned Maemo::Timed::Voland::Reminder::cookie() const
{
  return p->cookie ;
}

QString Maemo::Timed::Voland::Reminder::attr(const QString &key) const
{
  QMap<QString,QString>::const_iterator it = p->attr.find(key) ;
  if(it==p->attr.end())
    return QString() ;
  else
    return it.value() ;
}

const QMap<QString,QString> & Maemo::Timed::Voland::Reminder::attributes() const
{
  return p->attr ;
}

unsigned Maemo::Timed::Voland::Reminder::buttonAmount() const
{
  return p->buttons.size() ;
}

bool Maemo::Timed::Voland::Reminder::suppressTimeoutSnooze() const
{
  return ( p->flags & EventFlags::Suppress0 ) != 0 ;
}

bool Maemo::Timed::Voland::Reminder::hideSnoozeButton1() const
{
  return ( p->flags & EventFlags::Hide1 ) != 0 ;
}

bool Maemo::Timed::Voland::Reminder::hideCancelButton2() const
{
  return ( p->flags & EventFlags::Hide2 ) != 0 ;
}

bool Maemo::Timed::Voland::Reminder::isMissed() const
{
  return ( p->flags & EventFlags::Missed ) != 0 ;
}

QString Maemo::Timed::Voland::Reminder::buttonAttr(int x, const QString &key) const
{
  if(x<1 || x>p->buttons.size())
    return QString() ;

  const QMap<QString,QString> &B = p->buttons[x-1].attr;
  QMap<QString,QString>::const_iterator it = B.find(key) ;
  if(it==B.end())
    return QString() ;
  else
    return it.value() ;
}

register_qtdbus_metatype(Maemo::Timed::Voland::Reminder, 1) ;
register_qtdbus_metatype(Maemo::Timed::Voland::button_io_t, 2) ;
