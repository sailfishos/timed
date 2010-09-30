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
#include <map>
#include <string>
using namespace std ;

#include <qm/log>

#include "timeutil.h"
#include "olson.h"

map<string, olson*> olson::zonetab ;

olson::olson(const string &name) : zone_name(name)
{
}

olson *olson::by_name(const string &zone)
{
  map<string,olson*>::iterator it = zonetab.find(zone) ;
  if(it==zonetab.end())
    it = zonetab.insert(make_pair(zone, new olson(zone))).first ;
  return it->second ;
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
