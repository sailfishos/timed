#include <iodata/validator>
#include <iodata/storage>

#include <string>
#include <sstream>
#include <set>
#include <map>
using namespace std ;

#include "olson.h"
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
// 4. Default country (alpha-2) based on default time zome customization
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
  bool first ;
  for (set<olson*>::const_iterator it=x.begin(); it!=x.end(); ++it)
    os << (first ? first=false, "{" : ", " ) << (*it)->name() ;
  os << (first ? " }" : "}") ;
  return os.str() ;
}

#if 0
static iodata::validator *tzdata_validator = iodata::validator::from_file("/usr/share/timed/typeinfo/tzdata.type") ;

static struct tz_distinct_t
{
  // olson * guess_timezone(int mcc, tz_suggestions_t &list) ;
  tz_distinct_t(const iodata::record *) ;
  map<int, vector<olson*> > mcc_to_tzlist ;
}
*tz_distinct=NULL ;

static struct tz_single_t
{
  // olson * guess_timezone(int mcc) ;
  tz_single_t(const iodata::record *) ;
  map<int, string> mcc_to_tz ;
}
tz_single_t *tz_single=NULL ;

static map<string,string> mcc_to_xy ;



static iodata::record *open_file(const char *path, const char *type)
{
  log_notice("opening file '%s', reading record of type '%s'", path, type) ;
  iodata::storage file ;
  file.set_validator(tzdata_validator, type) ;
  file.set_primary_path(path) ;
  return file.load() ;
}


static void read_tzdata_files()
{
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

  if (iodata::record *rec = open_database("/usr/share/tzdata-timed/country-by-mcc.data", "mcc_to_xy_t"))
  {
    const iodata::array *a = rec->get("mcc_to_xy")->arr() ;
    for(unsigned i=0; i<a->size(); ++i)
    {
      int mcc_d = a->get(i)->get("mcc")->value() ;
      string mcc = str_printf("%d", mcc_d) ;
      string xy = a->get(i)->get("country")->str() ;
      mcc_to_xy[mcc] = xy ;
      xy_to_mcc[xy] = mcc ;
    }
    delete rec ;
  }
  else
  {
    log_warning("mcc to country code database not available") ;
    return "" ;
  }
}

#endif
