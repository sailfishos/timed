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
#include "tz.h"
#include "tzdata.h"

iodata::validator *tz_oracle_t::validator()
{
  static iodata::validator *v = NULL ;
  if(v==NULL)
    v = iodata::validator::from_file("/usr/share/timed/typeinfo/tzdata.type") ;
  return v ;
}

iodata::record *tz_oracle_t::open_database(const char *path, const char *type)
{
  iodata::storage file ;
  file.set_validator(validator(), type) ;
  file.set_primary_path(path) ;
  return file.load() ;
}

tz_oracle_t::tz_oracle_t()
{
  history = new history_t ;

  if(iodata::record *single = open_database("/usr/share/tzdata-timed/single.data", "tz_single_t"))
  {
    tz_single = new tz_single_t(single) ;
    delete single ;
  }
  else
  {
    tz_single = NULL ;
    log_warning("mcc-to-single-tz database corrupted or not present") ;
  }

  if(iodata::record *distinct = open_database("/usr/share/tzdata-timed/distinct.data", "tz_distinct_t"))
  {
    tz_distinct = new tz_distinct_t(distinct) ;
    delete distinct ;
  }
  else
  {
    tz_distinct = NULL ;
    log_warning("mcc-to-distinct-tz database corrupted or not present") ;
  }

  // log_assert(false, "please initialize xy_to_tz!") ;

  read_timezones_by_country() ;

  timer = new QTimer ;
  timer->setSingleShot(true) ;
}

tz_oracle_t::~tz_oracle_t()
{
  delete history ;
  delete tz_single ;
  delete tz_distinct ;
  delete timer ;
  delete validator() ;
}

void tz_oracle_t::read_timezones_by_country()
{
  iodata::record *rec = open_database("/usr/share/tzdata-timed/zones-by-country.data", "zones_by_country_t") ;
  log_assert(rec!=NULL) ;
  const iodata::array *a = rec->get("xy_to_tz")->arr() ;
  for(unsigned i=0; i<a->size(); ++i)
  {
    const iodata::record *country = a->get(i)->rec() ;
    string xy = country->get("xy")->str() ;
    vector<string> &zone_list = xy_to_tz[xy] ;
    // we are just ignoring major/minor for now
    static const char *keys[] = { "major", "minor" } ;
    for(int k=0; k<2; ++k)
    {
      string key = keys[k] ;
      const iodata::array *zones = country->get(key)->arr() ;
      for(unsigned j=0; j<zones->size(); ++j)
        zone_list.push_back(zones->get(j)->str()) ;
    }
  }
  delete rec ;
}

#if 0
void tz_oracle_t::mcc_delay_timeout()
{
  log_assert(false, "oops") ;
}
#endif

#if 0
void tz_oracle_t::nitz_data(const cellular_info_t &ci)
{
  log_debug() ;
  if(ci.has_offset())
    handle_offset(ci) ;
  else if(ci.has_mcc() && ci.has_mnc())
    mcc_data(ci.mcc(), ci.mnc()) ;
  else
    log_critical("oops") ;
}
#endif

#if 0
void tz_oracle_t::handle_offset(const cellular_info_t &ci)
{
  // preliminary implementation:
  // 1. take the full list of zones for given mcc
  // 2. filter out not matching
  // 3. if nothing remains: take iso_* zone, set all the zones as "secondary candidates"
  // 4. if a single zone remains: take it as "certain"
  // 5. if multiply zones remain: take the first one, all other are "primary candidates"

  // 1. find the country info based on mcc
  if(! ci.has_mcc())
  {
    log_error("cellular info doesn't contain mcc, ignoring it. ci=%s", ci.to_string().c_str()) ;
    return ;
  }
  if(! ci.has_offset())
  {
    log_error("cellular info doesn't contain offset, ignoring it. ci=%s", ci.to_string().c_str()) ;
    return ;
  }
  string xy = mcc_to_xy(ci.mcc()) ;
  if(xy.empty())
  {
    log_error("can't map cellular info's mcc to country code, ci=%s", ci.to_string().c_str()) ;
    return ;
  }

  map<string, vector<string> >::const_iterator tz_it = xy_to_tz.find(xy) ;
  if(tz_it==xy_to_tz.end())
  {
    log_error("can't find zone list for the country '%s'", xy.c_str()) ;
    return ;
  }

  const vector<string> &tz_list = tz_it->second ;

  // 2.
  vector<olson *> match ;
  time_t at = time(NULL) ;
  if(ci.has_time())
  {
    nanotime_t nitz_time = ci.timestamp() + nanotime_t::systime_at_zero() ;
    at = nitz_time.to_time_t() ;
  }
  int dst_flag = ci.has_dst() ? ci.dst() : -1 ;
  for(vector<string>::const_iterator it=tz_list.begin(); it!=tz_list.end(); ++it)
  {
    olson *zone = olson::by_name(*it) ;
    bool ok = zone->match(at, ci.offset(), dst_flag) ;
    log_debug("time zone '%s' %s match", zone->name().c_str(), ok ? "does" : "does not") ;
    if(ok)
      match.push_back(zone) ;
  }

  // 3. Just take the fist one. TODO: implement the other stuff

  if(match.size()>0)
  {
    tz_suggestions_t s ;
    s.gq = Reliable ;
    emit tz_detected(match[0], s) ;
    return ;
  }
  else
  {
    log_error("can't find matching zone for '%s'", ci.to_string().c_str()) ;
    return ;
  }
}
#endif

string tz_oracle_t::mcc_to_xy(int mcc_value)
{
  static bool loaded = false ;
  static map<int, string> mcc_to_xy ;
  if(!loaded)
  {
    loaded = true ;
    iodata::record *rec = open_database("/usr/share/tzdata-timed/country-by-mcc.data", "mcc_to_xy_t") ;
    if(rec==NULL)
    {
      log_warning("mcc to country code database not available") ;
      return "" ;
    }
    const iodata::array *a = rec->get("mcc_to_xy")->arr() ;
    for(unsigned i=0; i<a->size(); ++i)
    {
      int mcc = a->get(i)->get("mcc")->value() ;
      string xy = a->get(i)->get("country")->str() ;
      mcc_to_xy[mcc] = xy ;
    }
    delete rec ;
  }
  map<int, string>::const_iterator it = mcc_to_xy.find(mcc_value) ;
  return it==mcc_to_xy.end() ? "" : it->second ;
}

void tz_oracle_t::user_input(olson *tz)
{
  // not used yet, no UI for that
  tz_suggestions_t s ;
  s.gq = Reliable ;
  emit tz_detected(tz, s) ;
}

#if 0
void tz_oracle_t::mcc_data(int new_mcc, const string & /* mnc not used */)
{
  if(new_mcc == 0) // offline => do nothing
    return ;

  if(new_mcc == mcc) // the same country as before => do nothing
    return ;

  mcc = new_mcc ;

  if(olson *tz = tz_single->guess_timezone(mcc))
  {
    tz_suggestions_t s ;
    s.gq = Reliable ;
    emit tz_detected(tz, s) ;
    return ;
  }

  tz_suggestions_t s ;

  if(olson *tz = tz_distinct->guess_timezone(mcc, s))
  {
    emit tz_detected(tz, s) ;
    return ;
  }

  s.add(olson::by_name("Europe/Helsinki")) ;
  s.add(olson::by_name("Asia/Kolkata")) ;
  s.gq = Uncertain ;

  emit tz_detected(olson::by_name("Antarctica/DumontDUrville"), s) ;
}
#endif

void tz_oracle_t::output(olson *zone)
{
  output(zone, new suggestion_t, true) ;
}

void tz_oracle_t::output(olson *zone, suggestion_t *s, bool sure)
{
  (void) s ;
//#warning TODO
  log_notice("Time zone magically detected: '%s' (%s" "sure)", zone->name().c_str(), sure?"":"not ") ;
}

void tz_oracle_t::set_by_offset(const cellular_offset_t &data)
{
  if (oper.known_mcc())
  {
    set<olson*> m, r, result ; // main, real
    tzdata::by_country(oper.location(), tzdata::Main_Zones, m) ;
    tzdata::by_country(oper.location(), tzdata::Real_Zones, r) ;
    set<olson*>  rm = r ; // real minus main
    set_change<olson*>(rm, m, false) ; // false: rm-=m
    bool found = false ;
    if (data.dst<0) // usually it means "winter time", i.e. set dst to zero
    {
      found = found or tzdata::filter(m, data.timestamp, data.offset, 0, result) ;
      log_debug("after try 1: found=%d", found) ;
      found = found or tzdata::filter(rm, data.timestamp, data.offset, 0, result) ;
      log_debug("after try 2: found=%d", found) ;
    }
    if (data.dst!=0) // either "summer" (>0) or really not specified (<0)
    {
      found = found or tzdata::filter(m, data.timestamp, data.offset, data.dst, result) ;
      log_debug("after try 3: found=%d", found) ;
      found = found or tzdata::filter(rm, data.timestamp, data.offset, data.dst, result) ;
      log_debug("after try 4: found=%d", found) ;
    }
    if (not data.sender_time) // sender time not really know, try plus/minus day
    {
      const int day = 60*60*24 ;
      found = found or tzdata::filter(r, data.timestamp-day, data.offset, data.dst, result) ;
      log_debug("after try 5: found=%d", found) ;
      found = found or tzdata::filter(r, data.timestamp+day, data.offset, data.dst, result) ;
      log_debug("after try 6: found=%d", found) ;
    }
    log_notice("%d candidates selected: %s", result.size(), tzdata::set_str(result).c_str()) ;
    olson *zone = NULL ;
    if (result.size()==0)
    {
      zone = olson::by_offset(data.offset) ;
      if (zone==NULL)
      {
        log_error("failed to set time zome by offset=%d", data.offset) ;
        return ;
      }
    }
    else
    {
      zone = * result.begin() ;
      for (set<olson*>::const_iterator it=result.begin(); it!=result.end(); ++it)
        if (*it==stat.last_zone)
        {
          zone = *it ;
          break ;
        }
    }
    suggestion_t *s =  new suggestion_t ;
    for (set<olson*>::const_iterator it=rm.begin(); it!=rm.end(); ++it)
      s->add(*it, 20) ;
    for (set<olson*>::const_iterator it=m.begin(); it!=m.end(); ++it)
      s->add(*it, 10) ;
    s->add(tzdata::device_default(), 10) ;
    for (set<olson*>::const_iterator it=result.begin(); it!=result.end(); ++it)
      s->add(*it, 5) ;
    output(zone, s, result.size()==1) ;
  }
  else // some weird operator, or even empty
  {
    log_notice("setting timezone %s for a strange operator %s", data.str().c_str(), oper.str().c_str()) ;
    olson *zone = olson::by_offset(data.offset) ;
    if (zone==NULL)
    {
      log_error("failed to set time zone by offset=%d", data.offset) ;
      return ;
    }
    output(zone) ;
    // TODO: find by offset worldwide ???
  }
}

void tz_oracle_t::set_by_operator()
{
  // bool known = oper.known_mcc() ;

  if (oper.known_mcc())
  {
    set<olson*> all_zones, main_zones, real_zones ;
    tzdata::by_country(oper.location(), tzdata::All_Zones, all_zones) ;
    tzdata::by_country(oper.location(), tzdata::Main_Zones, main_zones) ;
    tzdata::by_country(oper.location(), tzdata::Real_Zones, real_zones) ;
    olson *guess = NULL ;
    if (stat.last_zone and all_zones.count(stat.last_zone)) // zone contained in this country: take it
      guess = stat.last_zone ;
    else
    {
      stat.last_zone = NULL ;
      guess = tzdata::device_default() ;
      if (not all_zones.count(guess)) // we're not in home country
        if (olson *first = tzdata::country_default(oper.location())) // let's be paranoid
          guess = first ;
    }
    log_assert(guess, "oops, guessed NULL pointer") ;
    log_info("zone '%s' magically guessed by operator identity", guess->name().c_str()) ;
    bool sure = real_zones.size()==1 ; // it's saingle zone country
    suggestion_t *s = new suggestion_t ;
    for (set<olson*>::const_iterator it=real_zones.begin(); it!=real_zones.end(); ++it)
      s->add(*it, 20) ;
    for (set<olson*>::const_iterator it=main_zones.begin(); it!=main_zones.end(); ++it)
      s->add(*it, 10) ;
    s->add(tzdata::device_default(), 10) ;
    output(guess, s, sure) ;
  }
  else // some weird operator located nowhere
  {
    if (stat.last_zone==NULL)
    {
      log_notice("can't guess zone for operator %s", oper.str().c_str()) ;
      return ;
    }
    else
    {
      log_notice("guessing last used zone '%s' for operator %s", stat.last_zone->name().c_str(), oper.str().c_str()) ;
      output(stat.last_zone) ;
    }
  }
}

tz_single_t::tz_single_t(const iodata::record *list_r)
{
  log_debug() ;

  if(list_r==NULL)
  {
    log_critical("can't load single zone mcc database") ;
    return ;
  }

  const iodata::array *list = list_r->get("list")->arr() ;
  unsigned N = 0 ;
  for(; N < list->size(); ++N)
  {
    const iodata::record *r = list->get(N)->rec() ;
    int mcc = r->get("mcc")->value() ;
    mcc_to_tz[mcc] = r->get("tz")->str() ;
  }

  log_info("loaded %d single zone mcc records", N) ;

}

olson * tz_single_t::guess_timezone(int mcc)
{
  map<int,string>::const_iterator it = mcc_to_tz.find(mcc) ;
  if(it!=mcc_to_tz.end())
    return olson::by_name(it->second) ;
  else
    return NULL ;
}

tz_distinct_t::tz_distinct_t(const iodata::record *rec)
{
  if(rec==NULL)
  {
    log_critical("can't load multiple zone mcc database") ;
    return ;
  }

  const iodata::array *list = rec->get("list")->arr() ;
  unsigned N = 0 ;
  for(; N < list->size(); ++N)
  {
    const iodata::record *r = list->get(N)->rec() ;
    int mcc = r->get("mcc")->value() ;
    const iodata::array *a = r->get("tz")->arr() ;
    vector<olson*> &tzlist = mcc_to_tzlist[mcc] ;
    for(unsigned i=0; i<a->size(); ++i)
      tzlist.push_back(olson::by_name(a->get(i)->str())) ;
  }

  log_info("loaded %d multiple zone mcc records", N) ;
}

olson * tz_distinct_t::guess_timezone(int mcc, tz_suggestions_t &list)
{
  map<int, vector<olson*> >::const_iterator it = mcc_to_tzlist.find(mcc) ;
  if(it==mcc_to_tzlist.end())
    return NULL ;
  const vector<olson*> &tzlist = it->second ;
  if(tzlist.size()==0) // why?!
    return NULL ;
  list.set(tzlist) ;
  list.gq = tzlist.size() > 1 ? Uncertain : Reliable ;

  return tzlist[0] ;
}

void tz_oracle_t::cellular_offset(const cellular_offset_t &data)
{
  timer->stop() ; // preventing setting tz by operator only
  if (data.oper.empty())
    log_error("opratorless NITZ received: %s", data.str().c_str()) ;
  else if (oper != data.oper)
  {
    log_error("unexpected operator change in NITZ package %s (current operator %s)", data.str().c_str(), oper.str().c_str()) ;
    history->save_status(stat, oper) ;
    oper = data.oper ;
    history->load_status(stat, oper) ;
  }
  set_by_offset(data) ;
}

void tz_oracle_t::cellular_operator(const cellular_operator_t &o)
{
  timer->stop() ;
#if 0
  delete waiting_oper ;
  waiting_oper = NULL ;
#endif

  bool empty = o.empty() ;
  bool same = o==oper ;
  bool same_country = o.known_mcc() and o.location()==oper.location() ;

  log_debug("o=%s, empty=%d, same=%d, same_country=%d", o.str().c_str(), empty, same, same_country) ;
  if (empty) // disconnected: do nothing
  {
    log_debug("empty operator") ;
    have_oper = false ;
    return ;
  }
  else
    have_oper = true ; // TODO: oper = o ;

  if (same) // same as current or same as last: do nothing
    return ;

  // now the operator is changing, first we have to save status
  history->save_status(stat, oper) ;

  oper = o ;

  history->load_status(stat, oper) ;
  log_debug() ;

  if(same_country) // nothing to do?
    return ;

  bool small_country = oper.known_mcc() and tzdata::is_single_zone_country(oper.location()) ;

  if (small_country and stat.regular)
    set_by_operator() ;
  else
    timer->start(nitz_wait_ms) ;
}

void tz_oracle_t::waiting_for_nitz_timeout()
{
  timer->stop() ;
  set_by_operator() ;
}

#if 0
int tz_oracle_t::basic_mcc(int mcc)
{
  return mcc ; // TODO: USA->310 etc etc
}
#endif
