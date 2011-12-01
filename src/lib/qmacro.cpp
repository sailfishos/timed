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

#include "qmacro.h"
#include "exception.h"
#include "event-declarations.h"
#include "event-io.h"

template <class T, class IO>
qdbus_reply_wrapper<T, IO>::qdbus_reply_wrapper(const QDBusMessage &reply)
  : io_reply(NULL)
  , p(NULL)
{
  io_reply = new QDBusReply<IO>(reply) ;
  if(io_reply->isValid())
    p =  new T(io_reply->value()) ;
}

template <class T, class IO>
qdbus_reply_wrapper<T, IO>::qdbus_reply_wrapper(const qdbus_reply_wrapper<T, IO> &reply)
  : io_reply(NULL)
  , p(NULL)
{
  io_reply = new QDBusReply<IO>(*reply.io_reply) ;
  if(io_reply->isValid())
    p =  new T(io_reply->value()) ;
}

template <class T, class IO>
qdbus_reply_wrapper<T, IO>::~qdbus_reply_wrapper()
{
  delete p ;
  delete io_reply ;
}

template <class T, class IO>
bool qdbus_reply_wrapper<T, IO>::isValid () const
{
  return io_reply->isValid() ;
}

template <class T, class IO>
const QDBusError & qdbus_reply_wrapper<T, IO>::error()
{
  return io_reply->error() ;
}

template <class T, class IO>
T & qdbus_reply_wrapper<T, IO>::value()
{
  return (T &)(*this) ;
}

template <class T, class IO>
const T & qdbus_reply_wrapper<T, IO>::value() const
{
  return (T &)(*this) ;
}

template <class T, class IO>
qdbus_reply_wrapper<T, IO>::operator T & ()
{
  if(!isValid())
    throw Maemo::Timed::Exception(__PRETTY_FUNCTION__, "invalid io_reply") ;
  return *p ;
}

template <class T, class IO>
qdbus_pending_reply_wrapper<T, IO>::qdbus_pending_reply_wrapper(const QDBusPendingCall &call)
  : io_reply(NULL)
  , p(NULL)
{
  io_reply = new QDBusPendingReply<IO>(call) ;
  if(io_reply->isValid())
    p =  new T(io_reply->value()) ;
}

template <class T, class IO>
qdbus_pending_reply_wrapper<T, IO>::~qdbus_pending_reply_wrapper()
{
  delete p ;
  delete io_reply ;
}

template <class T, class IO>
bool qdbus_pending_reply_wrapper<T, IO>::isValid () const
{
  return io_reply->isValid() ;
}

template <class T, class IO>
bool qdbus_pending_reply_wrapper<T, IO>::isError () const
{
  return io_reply->isError() ;
}

template <class T, class IO>
bool qdbus_pending_reply_wrapper<T, IO>::isFinished () const
{
  return io_reply->isFinished() ;
}

template <class T, class IO>
QDBusError qdbus_pending_reply_wrapper<T, IO>::error() const
{
  return io_reply->error() ;
}

template <class T, class IO>
T & qdbus_pending_reply_wrapper<T, IO>::value()
{
  return (T &)(*this) ;
}

template <class T, class IO>
const T & qdbus_pending_reply_wrapper<T, IO>::value() const
{
  return (T &)(*this) ;
}

template <class T, class IO>
void qdbus_pending_reply_wrapper<T, IO>::waitForFinished()
{
  if(!isFinished())
  {
    io_reply->waitForFinished() ;
    if(io_reply->isValid())
      p =  new T(io_reply->value()) ;
  }
}

template <class T, class IO>
qdbus_pending_reply_wrapper<T, IO>::operator T & () 
{
  waitForFinished() ;
  if(!isValid())
    throw Maemo::Timed::Exception(__PRETTY_FUNCTION__, "invalid io_reply") ;
  return *p ;
}

template class qdbus_reply_wrapper<Maemo::Timed::Event, Maemo::Timed::event_io_t> ;
template class qdbus_pending_reply_wrapper<Maemo::Timed::Event, Maemo::Timed::event_io_t> ;
template class qdbus_reply_wrapper<Maemo::Timed::Event::List, Maemo::Timed::event_list_io_t> ;
template class qdbus_pending_reply_wrapper<Maemo::Timed::Event::List, Maemo::Timed::event_list_io_t> ;
