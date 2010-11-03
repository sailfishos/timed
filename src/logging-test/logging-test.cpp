/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
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
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

char *name()
{
  static char buf[1024] ;
  *buf = '\0' ;
  FILE *cmdline = fopen("/proc/self/cmdline", "r") ;
  if (cmdline)
  {
    size_t res = fread(buf, 1, 1000, cmdline) ;
    buf[res] = '\0' ;
    fclose(cmdline) ;
  }
  return buf ;
}

int main()
{
  char buf[2000] ;
  time_t t=time(NULL) ;
  struct tm tm ;
  localtime_r(&t, &tm) ;

  sprintf(buf, "logging-test[%d]: %04d-%02d-%02d %02d:%02d:%02d cmdline='%s'", getpid(), tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, name()) ;
  fprintf(stderr, "%s\n", buf) ;
  openlog("logging-test", LOG_NOWAIT|LOG_DAEMON, LOG_DAEMON) ;
  syslog(LOG_CRIT, "%s", buf) ;
  closelog() ;
  FILE *fp = fopen("/var/log/logging-test", "a") ;
  if (fp!=NULL)
  {
    fprintf(fp, "%s\n", buf) ;
    fflush(NULL) ;
    fclose(fp) ;
  }
  return 0 ;
}
