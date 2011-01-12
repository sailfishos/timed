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
#ifndef __RECURRENCE_H__
#define __RECURRENCE_H__

#include <stdint.h>

namespace Maemo
{
  namespace Timed
  {
    class EventRecurrence
    {
    protected:
      uint64_t mins ;
      uint32_t hour ;
      uint32_t mday ;
      uint32_t wday ;
      uint32_t mons ;
    public:
      EventRecurrence() ;
      bool isEmpty() const ;
      void addMonth(int x) ;
      void everyMonth() ;
      void addDayOfMonth(int x) ;
      void addLastDayOfMonth() ;
      void everyDayOfMonth() ;
      void addDayOfWeek(int x) ;
      void everyDayOfWeek() ;
      void addHour(int x) ;
      void addMinute(int x) ;
    } ;
  }
}

#endif
