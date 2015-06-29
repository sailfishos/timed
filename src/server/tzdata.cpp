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

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <iodata-qt5/validator>
#include <iodata-qt5/storage>
#else
#include <iodata/validator>
#include <iodata/storage>
#endif

#include <string>
#include <sstream>
#include <set>
#include <map>
using namespace std ;

#include "../common/log.h"

#include "olson.h"
#include "misc.h"

#include "tzdata.h"

#if DEADCODE
struct tz_single_t ;
struct tz_distinct_t ;

struct tz_distinct_t
{
  olson * guess_timezone(int mcc, tz_suggestions_t &list) ;
  tz_distinct_t(const iodata::record *) ;
  map<int, vector<olson*> > mcc_to_tzlist ;
} ;

struct tz_single_t
{
  olson * guess_timezone(int mcc) ;
  tz_single_t(const iodata::record *) ;
  map<int, string> mcc_to_tz ;
} ;


string mcc_to_xy(int mcc) ; // maps mcc to country code (2 chars)
map<string, vector<string> > xy_to_tz ; // time zones by country code
iodata::validator *validator() ;
iodata::record *open_database(const char *path, const char *type) ;
void read_timezones_by_country() ;

#endif

// we need some some data structures...

// 1. Mapping mcc to alpha-2 code: 310=>US
map<string,string> mcc_to_xy ;
// 2. Mapping alpha-2 to zone to importance (0,1,2,3): US=>(New_York=>0, Chicago=>1, Phoenix=>2, Zaporozhye=>3)
map<string,map<string,int> > xy_to_zone_to_level ;
// 3. Mapping alpha-2 to main zone: US=>New_York
map<string,string> xy_to_zone0 ;
// 4. Default country (alpha-2) and zone based on default time zome customization
string home_country ;
olson *home_zone=NULL ;
// 5. Set of alpha-2 of single zone counties (only 0,1,2 count; 3 doesn't)
set<string> small_countries ;

string tzdata::iso_3166_1_alpha2_by_mcc(const string &mcc)
{
  map<string,string>::const_iterator it = mcc_to_xy.find(mcc) ;
  return it==mcc_to_xy.end() ? "" : it->second ;
}

int tzdata::by_country(const string &alpha2, enum tzdata::zone_type type, set<olson*> &out)
{
  map<string,map<string,int> >::const_iterator it = xy_to_zone_to_level.find(alpha2) ;
  if (it==xy_to_zone_to_level.end())
    return 0 ;

  int counter = 0 ;
  int threshold = type==tzdata::Main_Zones ? 1 : type==tzdata::Real_Zones ? 2 : 3 ; // all by default
  const map<string,int> &zone_to_level = it->second ;

  for (map<string,int>::const_iterator jt=zone_to_level.begin(); jt!=zone_to_level.end(); ++jt)
    if (jt->second <= threshold)
      out.insert(olson::by_name(jt->first)), ++counter ;

  return counter ;
}

olson *tzdata::country_default(const string &alpha2)
{
  map<string,string>::const_iterator it = xy_to_zone0.find(alpha2) ;
  return it==xy_to_zone0.end() ? NULL : olson::by_name(it->second) ;
}

olson *tzdata::device_default()
{
  return home_zone ;
}

int tzdata::filter(const set<olson*> &in, time_t moment, int offset, int dst, set<olson*> &out)
{
  int counter = 0 ;
  for (set<olson*>::const_iterator it=in.begin(); it!=in.end(); ++it)
    if ((*it)->match(moment, offset, dst))
      ++counter, out.insert(*it) ;
  return counter ;
}

bool tzdata::is_single_zone_country(const string &alpha2)
{
  return small_countries.find(alpha2) != small_countries.end() ;
}

string tzdata::set_str(const set<olson*> &x)
{
  ostringstream os ;
  bool first = true ;
  for (set<olson*>::const_iterator it=x.begin(); it!=x.end(); ++it)
    os << (first ? first=false, "{" : ", " ) << (*it)->name() ;
  os << (first ? "{ }" : "}") ;
  return os.str() ;
}

// --- initialization ---

#include "tzdata.type.h"

static iodata::record *open_database(const char *path, const char *type)
{
  log_notice("opening file '%s', reading record of type '%s'", path, type) ;
  iodata::storage file ;
  file.set_validator(tzdata_validator(), type) ;
  file.set_primary_path(path) ;
  if (iodata::record *res = file.load())
    return res ;
  log_abort("file '%s' corrupted or not present", path) ;
  // return NULL ; TODO: just print an error in init() and continue
}

static void process_zone(const string &xy, const string &tz, int i_value)
{
  if (i_value==0)
    xy_to_zone0[xy] = tz ; // capital
  if (tz==home_zone->name())
    home_country = xy ;
  xy_to_zone_to_level[xy][tz] = i_value ;
}

void tzdata::init(const string &default_tz)
{
  home_zone = olson::by_name(default_tz) ;

  iodata::record *A = open_database("/usr/share/tzdata-timed/country-by-mcc.data", "mcc_to_xy_t") ;
  iodata::record *B = open_database("/usr/share/tzdata-timed/single.data", "tz_single_t") ;
  iodata::record *C = open_database("/usr/share/tzdata-timed/zones-by-country.data", "zones_by_country_t") ;

  const iodata::array *a = A->get("mcc_to_xy")->arr() ;
  for(unsigned i=0; i<a->size(); ++i)
  {
    int mcc_d = a->get(i)->get("mcc")->value() ;
    string mcc = str_printf("%d", mcc_d) ;
    string xy = a->get(i)->get("country")->str() ;
    mcc_to_xy[mcc] = xy ;
  }

  const iodata::array *b = B->get("list")->arr() ; // TODO: rename list->tz_single (here and in tzdata script)
  for(unsigned i=0; i<b->size(); ++i)
  {
    int mcc_d = b->get(i)->get("mcc")->value() ;
    string mcc = str_printf("%d", mcc_d) ;
    string xy = tzdata::iso_3166_1_alpha2_by_mcc(mcc) ;
    if (xy.empty())
    {
      log_critical("Iso-3166 alpha-2 ID not found for MCC=%d", mcc_d) ;
      continue ;
    }
    small_countries.insert(xy) ;
    string tz = b->get(i)->get("tz")->str() ;
    process_zone(xy, tz, 0) ; // 0 is 'capital'
  }

  const iodata::array *c = C->get("xy_to_tz")->arr() ;
  for(unsigned i=0; i<c->size(); ++i)
  {
    // log_debug("i=%d", i) ;
    string xy = c->get(i)->get("xy")->str() ;
    for (int important=1; important<=2; ++important)
    {
      // log_debug("i=%d important=%d", i, important) ;
      const char *key = important==1 ? "major" : "minor" ;
      const iodata::array *list = c->get(i)->get(key)->arr() ;
      for (unsigned j=0; j<list->size(); ++j)
      {
        // log_debug("i=%d important=%d j=%d", i, important, j) ;
        int i_value = (important==1 and j==0) ? 0 : important ; // the very first is the capital
        process_zone(xy, list->get(j)->str(), i_value) ;
      }
    }
  }

  delete A ;
  delete B ;
  delete C ;
}
