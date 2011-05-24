#include <string>
using namespace std ;

#include "api.h"

#include "timezone.h"
using libtz::internal::timezone_t ;

libtz::tz_t::tz_t(const string &s)
{
  pimple = timezone_t::load(s) ;

  if (pimple)
    pimple->increase_ref_counter() ;
}

bool libtz::tz_t::is_valid() const
{
  return pimple != NULL ;
}
