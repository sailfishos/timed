#include <stdlib.h>
#include <string>
using namespace std ;

#include "daemon/f.h"

#if F_IMAGE_TYPE

#include <sysinfo.h>
#include "imagetype.h"

string imagetype()
{
  string result ;
  struct system_config *sc ;
  int init = sysinfo_init(&sc) ;
  if (init!=0)
    return result ;

  uint8_t *value ;
  unsigned long length ;
  int get = sysinfo_get_value(sc, "/device/sw-release-ver", &value, &length) ;
  if (get!=0)
  {
    free(sc) ;
    return result ;
  }

  unsigned im_start=0, im_len=0 ;
  for(unsigned i=0, count=0; i<length; ++i)
  {
    if (value[i]!='_')
      continue ;
    ++ count ;
    if (count==3)
      im_start = i+1 ;
    else if (count==4)
    {
      im_len = i - im_start ;
      break ;
    }
  }

  if (im_start and im_len)
    result = string((const char*)(value+im_start), im_len) ;

  free(value) ;
  free(sc) ;

  return result ;
}

#endif // F_IMAGE_TYPE
