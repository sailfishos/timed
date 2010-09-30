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
#include <cassert>
#include <stdint.h>

#include <timed/recurrence.h>
#include <timed/event.h>

namespace Maemo
{
  namespace Timed
  {
    EventRecurrence::EventRecurrence()
    {
      mins = hour = mday = wday = mons = 0 ;
    }
    bool EventRecurrence::isEmpty() const
    {
      const uint32_t full_week = 0177 ;
      if((wday&full_week)==0)
        return true ;
      const uint64_t any_mins = (1ull<<60)-1 ;
      if((mins&any_mins)==0)
        return true ;
      const uint32_t any_hour = (1u<<24)-1 ;
      if((hour&any_hour)==0)
        return true ;
      const uint32_t m31 = /* d-o -ay -m- m-j*/ 05325 ;
      const uint32_t m30 = /* dno say jma m-j*/ 07775 ;
      const uint32_t d30 = 0x7FFFFFFF ; // 0, 1..30
      const uint32_t d29 = 0x3FFFFFFF ; // 0, 1..29
      if(mday==0 || mons==0)
        return true ;
      if(mons&m31) /* at least one long month, any day then */
        return false ;
      if((mons&m30) && (mday&d30)) /* 1..30 in a non-february */
        return false ;
      if(mday & d29) /* 1..29, any month */
        return false ;
      return true ;
    }
    void EventRecurrence::addMonth(int x)
    {
      Maemo::Timed::Event::check_interval(__PRETTY_FUNCTION__, x, 1, 12) ;
      mons |= 1u << (x-1) ;
    }
    void EventRecurrence::everyMonth()
    {
      mons = 07777 ;
    }
    void EventRecurrence::addDayOfMonth(int x)
    {
      Event::check_interval(__PRETTY_FUNCTION__, x, 0, 31) ;
      mday |= 1u << x ;
    }
    void EventRecurrence::addLastDayOfMonth()
    {
      mday |= 1 << 0 ;
    }
    void EventRecurrence::everyDayOfMonth()
    {
      mday = 0xFFFFFFFF ;
    }
    void EventRecurrence::addDayOfWeek(int x)
    {
      Event::check_interval(__PRETTY_FUNCTION__, x, 0, 7) ;
      wday |= 1u << (x==7 ? 0 : x) ;
    }
    void EventRecurrence::everyDayOfWeek()
    {
      const uint32_t full_week = 0177 ;
      wday = full_week ;
    }
    void EventRecurrence::addHour(int x)
    {
      Event::check_interval(__PRETTY_FUNCTION__, x, 0, 23) ;
      hour |= 1u << x ;
    }
    void EventRecurrence::addMinute(int x)
    {
      Event::check_interval(__PRETTY_FUNCTION__, x, 0, 59) ;
      mins |= 1ull << x ;
    }
    /*
    void EventRecurrence::check_interval(int value, int min, int max)
    {
      assert(min<=value && value<=max) ;
    }
    */
  }
}
