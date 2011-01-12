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

Maemo::Timed::Interface::Interface(QObject *parent)
  : QDBusAbstractInterface(Maemo::Timed::service(), Maemo::Timed::objpath(), Maemo::Timed::interface(), Maemo::Timed::bus(), parent)
{
}
