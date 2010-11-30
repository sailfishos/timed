#ifndef MAEMO_TIMED_ALIASES_H
#define MAEMO_TIMED_ALIASES_H

#include <string>

namespace Maemo
{
  namespace Timed
  {
    void free_tz_list() ;
    bool is_tz_name(const std::string &tz) ;
    std::string tz_alias_to_name(const string &tz) ;
  }
}

#endif // MAEMO_TIMED_ALIASES_H
