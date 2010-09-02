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
#include "tz.h"

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
  mcc = 0 ;

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
#if 0
  tz_default = NULL ;
#endif

  delay = new QTimer ;
  delay->setSingleShot(true) ;
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

void tz_oracle_t::mcc_delay_timeout()
{
  log_assert(false, "oops") ;
}

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
  time_t at = ci.has_time() ? ci.timestamp().to_time_t() : time(NULL) ;
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

string tz_oracle_t::mcc_to_xy(int mcc_value)
{
  static bool loaded = false ;
  static map<int, string> mcc_to_xy ;
  if(!loaded)
  {
    loaded = true ;
    iodata::record *rec = open_database("/usr/share/tzdata-timed/single.data", "mcc_to_xy_t") ;
    if(rec==NULL)
    {
      log_warning("mcc to country code database not available") ;
      return "" ;
    }
    const iodata::array *a = rec->arr() ;
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

tz_single_t::tz_single_t(const iodata::record *list_r)
{
  log_debug() ;
#if 0
  iodata::storage *mcc_tz = new iodata::storage ;
  mcc_tz->set_validator(validator(), "tz_single_t") ;
  mcc_tz->set_primary_path("/usr/share/tzdata-timed/single.data") ;
  iodata::record *list_r = mcc_tz->load() ;
  delete mcc_tz ;
#endif

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

// --------------------------------------------------

#if 0

void tz_state::gmt_offset(time_t now, int new_offset, int new_dst_flag)
{
  if(offset==new_offset && dst_flag==new_dst_flag)
  {
    update_time(now) ;
    return ;
  }

  offset = new_offset ;
  dst_flag = new_dst_flag ;

  compute_candidates(now) ;
}

void tz_state::compute_candidates(time_t now)
{
  candidates.clear() ;
  if(const olson *s=cnifo->sinle_zone())
  {
    candidates.insert(s) ;
    return ;
  }
}

#endif
