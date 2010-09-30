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
#ifndef PINGUIN_H
#define PINGUIN_H

#include <qm/log>

#include <timed-voland/interface>

#include "singleshot.h"

#if 0
namespace Alarm
{
#endif
  class pinguin : public QObject
  {
    Q_OBJECT ;
    unsigned max_num ;
    unsigned counter ;
    bool needed ;
    simple_timer *timer ;
  private Q_SLOTS:
    void timeout()
    {
      // log_debug("counter=%d, timer: %s", counter, timer->status().toStdString().c_str()) ;
      if(!needed) // could rarely happen
        return ;
      counter ++ ;
      log_info("pinging voland activation service, try %d out of %d", counter, max_num) ;
      ping() ;
      if(counter<max_num)
        timer->start() ;
      // log_debug("timer: %s", timer->status().toStdString().c_str()) ;
    }
    void voland_needed()
    {
      // log_debug("timer: %s", timer->status().toStdString().c_str()) ;
      needed = true ;
      if(!timer->isActive())
        timeout() ;
    }
    void voland_registered()
    {
      // log_debug("timer: %s", timer->status().toStdString().c_str()) ;
      timer->stop() ;
      needed = false ;
      counter = 0 ;
    }
  public:
    pinguin(unsigned p, unsigned n, QObject *parent=NULL)
      : QObject(parent), max_num(n), counter(0)
    {
      timer = new simple_timer(p) ;
      needed = false ;
      QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout())) ;
    }
#if 0
    void ping()
    {
      // log_debug("timer: %s", timer->status().toStdString().c_str()) ;
      log_debug() ;
      Maemo::Timed::Voland::ActivationInterface ifc ;
      log_debug() ;
      if(!ifc.isValid())
        log_critical("Activation interface is not valid: %s", ifc.lastError().message().toStdString().c_str()) ;
      else
      {
        log_debug("Calling pid() asyncronosly") ;
        ifc.pid_async() ;
        log_debug() ;
      }
    }
#else
    void ping()
    {
      log_debug() ;
      const char *serv = Maemo::Timed::Voland::/*activation_*/service() ;
      const char *path = Maemo::Timed::Voland::/*activation_*/objpath() ;
      const char *ifac = Maemo::Timed::Voland::/*activation_*/interface() ;
      const char *meth = "pid" ;
      QDBusMessage mess = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
      QDBusConnection conn = Maemo::Timed::Voland::bus() ;
      if(conn.send(mess))
        log_info("the 'pid' request asyncronosly") ;
      else
        log_error("Can't send the 'pid' request: %s", conn.lastError().message().toStdString().c_str()) ;
    }
#endif
  } ;
#if 0
}
#endif

#endif
