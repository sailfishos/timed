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
#include <map>
#include <string>
using namespace std ;

#include "../common/log.h"

#include "timeutil.h"
#include "misc.h"
#include "olson.h"

map<string, olson*> *olson::zonetab = NULL ;

olson::olson(const string &name) : zone_name(name)
{
}

olson *olson::by_name(const string &zone)
{
  if (zonetab==NULL)
    zonetab = new map<string, olson*> ;
  map<string,olson*>::iterator it = zonetab->find(zone) ;
  if(it==zonetab->end())
    it = zonetab->insert(make_pair(zone, new olson(zone))).first ;
  return it->second ;
}

olson *olson::by_offset(int offset)
{
  int min15 = 15*60, units = offset/min15 ;
  if (offset%min15)
  {
    log_error("can't find a time zone by offset=%d (not divisible by 15min unit)", offset) ;
    return NULL ;
  }
  if (units < -60 || 60 < units)
  {
    log_error("can't find a time zone by offset=%d (exceeds 15h boundary)", offset) ;
    return NULL ;
  }
  int sec = offset ;
  char sign = sec<0 ? (sec=-sec, '-') : '+' ;
  int m = sec/60, hour = m/60, min = m%60 ;
  string name = str_printf("Iso8601/%c%02d%02d", sign, hour, min) ;
  log_info("mapping offset=%d to zone '%s'", offset, name.c_str()) ;
  return by_name(name) ;
}

bool olson::match(time_t at, int offset, int dst_flag)
{
  switch_timezone xx(zone_name) ;
  struct tm tm ;
  if(localtime_r(&at, &tm)==NULL)
  {
    log_warning("localtime_r(%ld) for TZ='%s' failed: %m", at, zone_name.c_str()) ;
    return false ;
  }
  bool result = offset == tm.tm_gmtoff ;
  if(result && dst_flag>=0 && tm.tm_isdst>=0) // check dst flag, both value are available
    result = (dst_flag==0) == (tm.tm_isdst==0) ;
  log_debug("input: at=%ld, offset=%d, dst_flag=%d; zone: name=%s, gmt_off=%ld isdst=%d; return: %s",
     at, offset, dst_flag, zone_name.c_str(), tm.tm_gmtoff, tm.tm_isdst, result?"YES":"NO") ;
  return result ;
}

void olson::destructor()
{
  log_debug("zonetab=%p", zonetab) ;
  if (not zonetab)
    return ;
  for(map<string, olson*>::iterator it=zonetab->begin(); it!=zonetab->end(); ++it)
    delete it->second ;
  delete zonetab ;
}

namespace
{
  struct singleton_t
  {
    ~singleton_t()
    {
      olson::destructor() ;
    }
  } singleton ;
}
