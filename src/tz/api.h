#ifndef MAEMO_LIBTZ_API_H
#define MAEMO_LIBTZ_API_H

#include <string>

namespace libtz
{
  namespace internal { struct timezone_t ; }

  class tz_t
  {
    internal::timezone_t *pimple ;
  public:
    bool is_valid() const ;
    tz_t(const std::string &s) ;

  } ;
}

#endif//MAEMO_LIBTZ_API_H
