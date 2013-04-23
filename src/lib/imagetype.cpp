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
#include <stdlib.h>
#include <string>
using namespace std ;

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
    sysinfo_finish(sc) ;
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
  sysinfo_finish(sc) ;

  return result ;
}

#endif // F_IMAGE_TYPE
