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
#include <sys/types.h>
#include <sys/stat.h>

#include "ticker.h"

int main(int ac, char **av)
{
  if (ac>1 && (string)av[1]=="start")
  {
    pid_t pid = fork() ;
    if (pid<0)
    {
      fprintf(stderr, "fork() failed: %m\n") ;
      return 1 ;
    }
    if (pid>0)
    {
      fprintf(stderr, "started as pid=%d\n", pid) ;
      return 0 ;
    }
    umask(0) ;
    setsid() ; // don't check for failure
    chdir("/") ;
    freopen( "/dev/null", "r", stdin) ;
    freopen( "/dev/null", "w", stdout) ;
    freopen( "/dev/null", "w", stderr) ;
  }
  return ticker(ac,av).exec() ;
}
