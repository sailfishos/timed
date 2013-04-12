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
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
using namespace std ;

const int year = 2010 ;

int main(int ac, char **av)
{
  if(ac!=2 || av[1][0]!='/')
  {
    cerr << "Usage: " << av[0] << " /full/path/to/zone/file\n" ;
    return 1 ;
  }
  if(access(av[1], F_OK) != 0)
  {
    cerr << av[0] << ": " << av[1] << " can't read file:" << strerror(errno) << endl ;
    return 1 ;
  }

  int res_setenv = setenv("TZ", ":Etc/GMT", true) ;
  tzset() ;

  if(res_setenv<0)
  {
    cerr << av[0] << ": can't set TZ to Etc/GTM: " << strerror(errno) << endl ;
    return 1 ;
  }

  struct tm tm ;
  tm.tm_year = year - 1900 ;
  tm.tm_mon = 0 ; // January
  tm.tm_mday = 1 ;
  tm.tm_hour = 12 ;
  tm.tm_min = 34 ; // 12:34 1st of January GMT
  tm.tm_sec = 0 ;

  time_t begin = mktime(&tm) ;

  if(begin==(time_t)(-1))
  {
    cerr << av[0] << ": mktime() failed: " << strerror(errno) << endl ;
    return 1 ;
  }

  string tz = ":" ;
  tz += av[1] ;

  int res_setenv2 = setenv("TZ", tz.c_str(), true) ;
  tzset() ;

  if(res_setenv2<0)
  {
    cerr << av[0] << ": can't set TZ to '" << tz << "': " << strerror(errno) << endl ;
    return 1 ;
  }

  string signature ;

  for(int i=0; i<=1000; ++i)
  {
    time_t t = begin + i*24*60*60 ; // 1 Jan + i days
    struct tm result ;
    struct tm *res = localtime_r(&t, &result) ;
    if(res==NULL)
    {
      cerr << av[0] << ": localtime failed for time_t t=" << t << "': " << strerror(errno) << endl ;
      return 1 ;
    }
    char uws = result.tm_isdst<0 ? 'u' : result.tm_isdst>0 ? 's' : 'w' ;
    // uws: unknown/winter/summer
    long off = result.tm_gmtoff ;
    bool minus = off<0 ? (off=-off, true) : false ;
    const int min15 = 15*60 ;
    if(off % min15 !=0)
    {
      cerr << av[0] << ": " << tz << ": offset is " << off << " (not divisible by 15 mins)\n" ;
      return 1 ;
    }
    int h = off / (60*60) ;
    int m = off % (60*60) ;
    m /= min15 ;
    assert(0<=m && m<=3) ;
    char oxyz = "oxyz"[m] ; // o:00 x:15 y:30 z:45
    char H = h<10 ? h+'0' : h-10+'A' ;
    if(minus)
      uws = toupper(uws), oxyz=toupper(oxyz) ;
    signature += uws ;
    signature += H ;
    signature += oxyz ;
  }

  cout << signature << endl ;

  return 0 ;
}
