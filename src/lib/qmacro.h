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
#ifndef MAEMO_TIMED_QMACRO_H
#define MAEMO_TIMED_QMACRO_H

#include <QDebug>
#include <QMetaType>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusReply>

namespace Maemo { namespace Timed { inline QString c2q(const char *c) ; } }

inline QString Maemo::Timed::c2q(const char *c)
{
  return QString::fromLatin1(c) ;
}

#define declare_qtdbus_io(type_name) \
  QDBusArgument &operator<<(QDBusArgument &out, const type_name &x) ; \
  const QDBusArgument &operator>>(const QDBusArgument &in, type_name &x) ; \
  Q_DECLARE_METATYPE(type_name)

#define declare_qtdbus_io_friends(type_name) \
  friend QDBusArgument & ::operator<<(QDBusArgument &, const type_name &) ; \
  friend const QDBusArgument & ::operator>>(const QDBusArgument &in, type_name &x) ;

#define qtdbus_print_class_name(type_name) if(not true) qDebug() << "register_qtdbus_metatype: " #type_name ;

#define register_qtdbus_metatype(type_name, I) namespace{static struct _reg_t##I{_reg_t##I(){ \
  qtdbus_print_class_name(type_name) qDBusRegisterMetaType<type_name> () ; } } _reg_t_object##I ; }

#define qtdbus_method(name, param_in, ...) \
    QDBusMessage name##_sync param_in { return call(Maemo::Timed::c2q(#name), ## __VA_ARGS__) ; } \
    QDBusPendingCall name##_async param_in { return asyncCall(Maemo::Timed::c2q(#name), ## __VA_ARGS__) ; }

struct qdbusargument_structrure_wrapper
{
  QDBusArgument &o ;
  qdbusargument_structrure_wrapper(QDBusArgument &out) : o(out) {o.beginStructure();}
 ~qdbusargument_structrure_wrapper() {o.endStructure();}
} ;

struct qdbusargument_structrure_wrapper_const
{
  const QDBusArgument &i ;
  qdbusargument_structrure_wrapper_const(const QDBusArgument &in) : i(in) {i.beginStructure();}
 ~qdbusargument_structrure_wrapper_const() {i.endStructure();}
} ;

template <class T>
class qdbus_reply_wrapper
{
public:
  qdbus_reply_wrapper(const QDBusMessage &reply) ;
  ~qdbus_reply_wrapper() ;
  bool isValid () const ;
  const QDBusError &error() ;
  T &value() ;
  const T &value() const ;
  operator T & () ;

private:
  QDBusReply<typename T::IO> *io_reply ;
  T *p ;
} ;

template <class T>
class qdbus_pending_reply_wrapper
{
public:
  qdbus_pending_reply_wrapper(const QDBusPendingCall &call) ;
  ~qdbus_pending_reply_wrapper() ;
  bool isValid () const ;
  bool isError () const ;
  bool isFinished () const ;
  QDBusError error() const ;
  T &value() ;
  const T &value() const ;
  void waitForFinished() ;
  operator T & ()  ;

private:
  QDBusPendingReply<typename T::IO> *io_reply ;
  T *p ;
} ;

#endif
