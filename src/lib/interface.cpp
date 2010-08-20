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
#include "interface.h"

#if 0
struct Maemo::Timed::signal_receiver : public QObject
{
  Q_OBJECT ;
public:
  signal_receiver()
  {
    const char *s = Maemo::Timed::service() ;
    const char *o = Maemo::Timed::objpath() ;
    const char *i = Maemo::Timed::interface() ;
    Maemo::Timed::bus().connect(s,o,i,"settings_changed_1",SIGNAL(settings_1(bool))) ;
  }
public Q_SIGNALS:
  void settings_1(bool) ;
} ;
#endif

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

Maemo::Timed::Interface::Interface(QObject *parent)
#if 0
  : QDBusInterface("com.nokia.time", "/com/nokia/time", "com.nokia.time", QDBusConnection::sessionBus(), parent)
#else
  : QDBusInterface(Maemo::Timed::service(), Maemo::Timed::objpath(), Maemo::Timed::interface(), Maemo::Timed::bus(), parent)
#endif
{
#if 0
  receiver = new signal_receiver ;
#endif
#if 0
  // bus().connect(service(), objpath(), interface(), "settings_changed", this, ) ;
  // bus().connect(service(), objpath(), interface(), "settings_changed_1", this, SIGNAL(settings_changed_1(bool))) ;
  int res = QDBusConnection::sessionBus().connect(
#if 0
      service(), objpath(), interface(),
#else
      "com.nokia.time", "/com/nokia/time", "com.nokia.time",
#endif
      "settings_changed_1", this, SLOT(xxx(bool))) ;
  qDebug() << __PRETTY_FUNCTION__ ;
  qDebug() << ( isValid() ? "VALID" : "NOT VALID" ) << "res" << res ;
  qDebug() << service() << objpath() << interface() ;
#endif
}
