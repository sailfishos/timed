#ifndef MAEMO_TIMED_TZDATA_H
#define MAEMO_TIMED_TZDATA_H

#include <string>
#include <set>

class olson ;

namespace tzdata
{
  enum zone_type { All_Zones, Main_Zones, Real_Zones } ;

  std::string iso_3166_1_alpha2_by_mcc(std::string mcc) ;
  void by_country(const std::string &alpha2, enum zone_type type, std::set<olson*> &out) ;
  olson* country_default(const std::string &alpha2) ;
  olson* device_default() ;
  int filter(const std::set<olson*> &in, time_t moment, int offset, int dst, std::set<olson*> &out) ;
  bool is_single_zone_country(const std::string &alpha2) ;

  std::string set_str(const std::set<olson*> &) ;
}

#endif//MAEMO_TIMED_TZDATA_H
