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
#include "adaptor.h"
#include "timed.h"
#include "event.h"

#include "../common/log.h"

#include "f.h"

#include <QMetaType>
int main(int ac, char **av)
{

  try
  {
    Timed *server = new Timed(ac,av) ;
    int result = server->exec() ;
    string halt = server->is_halted() ;
    if (!halt.empty())
    {
      const char *cud = "clear-device", *rfs = "restore-original-settings" ;
      if(halt==cud || halt==rfs)
      {
        log_info("halt: '%s' requested", halt.c_str()) ;
        const char *rm_all_files = "rm -rf /var/cache/timed/*" ;
        const char *rm_settings = "rm -rf /var/cache/timed/settings*" ;
        const char *cmd = halt==cud ? rm_all_files : rm_settings ;
        int res = system(cmd) ;
        if (res != 0)
          log_critical("'%s' failed with res=%d: %m", cmd, res) ;
        else
          log_info("cache files erased successfully by '%s'", cmd) ;
      }
      else
        log_warning("unknown parameter in halt() request: '%s'", halt.c_str()) ;
      log_info("Go to sleep, good night!") ;
      for(;;)
        sleep(99999) ;
    }
    delete server ;
    UnixSignal::uninitialize() ;
    return result ;
  }
  catch(const iodata::validator::exception &e)
  {
    log_critical("%s", e.info().c_str()) ;
  }
  catch(const iodata::exception &e)
  {
    log_critical("iodata::exception %s", e.info().c_str()) ;
  }
  catch(const std::exception &e)
  {
    log_critical("oops, unknown std::exception: %s", e.what()) ;
  }
  catch(...)
  {
    log_critical("oops, unknown exception of unknown type ...") ;
  }
  return 1 ;
}
