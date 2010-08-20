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
#ifndef __ACTION_H__
#define __ACTION_H__

#include <QString>

namespace Maemo
{
  namespace Timed
  {
    class Event ;
    class EventButton ;

    class EventAction
    {
    protected:
      class Event *owner ;
      QMap<QString,QString> attr ;
      uint32_t flags ;
    public:
      void setAttribute(const QString &key, const QString &value) ;
      void runCommand() ;
      void runCommand(const QString &cmd) ;
      void runCommand(const QString &cmd, const QString &user) ;
      void dbusMethodCall() ;
      void dbusMethodCall(const QString &srv, const QString &mtd, const QString &obj, const QString &ifc) ;
      void dbusSignal() ;
      void dbusSignal(QString op, QString ifc, QString sg) ;
      void setSendCookieFlag() ;
      void setSendAttributesFlag() ;
      void setSendEventAttributesFlag() ;
      void setUseSystemBusFlag() ;
      void whenQueued() ;
      void whenDue() ;
      void whenMissed() ;
      void whenTriggered() ;
      void whenSnoozed() ;
      void whenServed() ;
      void whenAborted() ;
      void whenFailed() ;
      void whenFinalized() ;
      void whenButton(const EventButton &x) ;
      void whenSysButton(int x) ;
    } ;
  }
}

#endif
