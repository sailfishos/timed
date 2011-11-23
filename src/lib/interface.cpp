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
#include "interface.h"
#include "event-io.h"
#include "event-declarations.h"
#include "exception.h"

Maemo::Timed::EventDBusReply::EventDBusReply(const QDBusMessage &reply)
  : eio_reply(NULL)
  , event_p(NULL)
{
  eio_reply = new QDBusReply<event_io_t>(reply) ;
  if(eio_reply->isValid())
    event_p =  new Event(eio_reply->value()) ;
}

Maemo::Timed::EventDBusReply::EventDBusReply(const EventDBusReply &reply)
  : eio_reply(NULL)
  , event_p(NULL)
{
  eio_reply = new QDBusReply<event_io_t>(*reply.eio_reply) ;
  if(eio_reply->isValid())
    event_p =  new Event(eio_reply->value()) ;
}

Maemo::Timed::EventDBusReply::~EventDBusReply()
{
  delete event_p ;
  delete eio_reply ;
}

bool Maemo::Timed::EventDBusReply::isValid () const
{
  return eio_reply->isValid() ;
}

const QDBusError & Maemo::Timed::EventDBusReply::error()
{
  return eio_reply->error() ;
}

Maemo::Timed::Event & Maemo::Timed::EventDBusReply::value()
{
  return (Maemo::Timed::Event &)(*this) ;
}

const Maemo::Timed::Event & Maemo::Timed::EventDBusReply::value() const
{
  return (Maemo::Timed::Event &)(*this) ;
}

Maemo::Timed::EventDBusReply::operator Maemo::Timed::Event & ()
{
  if(!isValid())
    throw Exception(__PRETTY_FUNCTION__, "invalid EventDBusReply") ;
  return *event_p ;
}




Maemo::Timed::EventDBusPendingReply::EventDBusPendingReply(const QDBusPendingCall &call)
  : eio_reply(NULL)
  , event_p(NULL)
{
  eio_reply = new QDBusPendingReply<event_io_t>(call) ;
  if(eio_reply->isValid())
    event_p =  new Event(eio_reply->value()) ;
}

Maemo::Timed::EventDBusPendingReply::~EventDBusPendingReply()
{
  delete event_p ;
  delete eio_reply ;
}

bool Maemo::Timed::EventDBusPendingReply::isValid () const
{
  return eio_reply->isValid() ;
}

bool Maemo::Timed::EventDBusPendingReply::isError () const
{
  return eio_reply->isError() ;
}

bool Maemo::Timed::EventDBusPendingReply::isFinished () const
{
  return eio_reply->isFinished() ;
}

QDBusError Maemo::Timed::EventDBusPendingReply::error() const
{
  return eio_reply->error() ;
}

Maemo::Timed::Event & Maemo::Timed::EventDBusPendingReply::value()
{
  return (Maemo::Timed::Event &)(*this) ;
}

const Maemo::Timed::Event & Maemo::Timed::EventDBusPendingReply::value() const
{
  return (Maemo::Timed::Event &)(*this) ;
}

void Maemo::Timed::EventDBusPendingReply::waitForFinished()
{
  if(!isFinished())
  {
    eio_reply->waitForFinished() ;
    if(eio_reply->isValid())
      event_p =  new Event(eio_reply->value()) ;
  }
}

Maemo::Timed::EventDBusPendingReply::operator Maemo::Timed::Event & ()
{
  waitForFinished() ;
  if(!isValid())
    throw Exception(__PRETTY_FUNCTION__, "invalid EventDBusReply") ;
  return *event_p ;
}



bool Maemo::Timed::Interface::settings_changed_connect(QObject *object, const char *slot)
{
  static const char *s = Maemo::Timed::service() ;
  static const char *o = Maemo::Timed::objpath() ;
  static const char *i = Maemo::Timed::interface() ;
  static const char *settings_changed_signal = SIGNAL(settings_changed(const Maemo::Timed::WallClock::Info &, bool)) ;
  if(QObject::connect(this, settings_changed_signal, object, slot))
  {
    QObject::disconnect(this, settings_changed_signal, object, slot) ;
    return Maemo::Timed::bus().connect(s,o,i,"settings_changed",object,slot) ;
  }
  else
    return false ;
}

bool Maemo::Timed::Interface::settings_changed_disconnect(QObject *object, const char *slot)
{
  static const char *s = Maemo::Timed::service() ;
  static const char *o = Maemo::Timed::objpath() ;
  static const char *i = Maemo::Timed::interface() ;
  return Maemo::Timed::bus().disconnect(s,o,i,"settings_changed",object,slot) ;
}

Maemo::Timed::EventDBusReply Maemo::Timed::Interface::get_event_sync (uint32_t cookie)
{
  return call("get_event", cookie) ;
}

QDBusPendingCall Maemo::Timed::Interface::get_event_async (uint32_t cookie)
{
  return asyncCall("get_event", cookie) ;
}

Maemo::Timed::Interface::Interface(QObject *parent)
  : QDBusAbstractInterface(Maemo::Timed::service(), Maemo::Timed::objpath(), Maemo::Timed::interface(), Maemo::Timed::bus(), parent)
{
}
