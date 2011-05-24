#ifndef MAEMO_LIBTZ_TIMEZONE_H
#define MAEMO_LIBTZ_TIMEZONE_H

#include <string>
#include <map>
#include <set>
using namespace std ;

namespace libtz { namespace internal { struct timezone_t ; } }
namespace libtz { namespace internal { struct ttinfo_t ; } }

struct libtz::internal::ttinfo_t
{
  time_t start ;
  int32_t offset ;
  const char *name ;
  bool dst ;
} ;

struct libtz::internal::timezone_t
{
  int ref_counter ;
  unsigned size ; // 
  ttinfo_t *table ; // size+1 entries, the last entry has .start=MAX_TIME_T
  string file ;

  void increase_ref_counter() ;

  static map<string,timezone_t*> loaded ;
  static set<string> failed ;
  static timezone_t* load(const string &s) ;
  static timezone_t* load_from_file(const string &s) ;
} ;

#endif//MAEMO_LIBTZ_TIMEZONE_H

