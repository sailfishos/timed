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
#include "adaptor.h"
#include "timed.h"
#include "event.h"

#include "timed/imagetype.h"
#include "timed/log.h"

#include "f.h"

#include <qmlog>

#include <QMetaType>
int main(int ac, char **av)
{
  int syslog_level = qmlog::Full ;
  int varlog_level = qmlog::Full ;

#if F_IMAGE_TYPE
  string image_type = imagetype() ;
  bool debug_flag = access(F_FORCE_DEBUG_PATH, F_OK) == 0 ;
  if (not debug_flag)
  {
    if (image_type=="PR")
      syslog_level = varlog_level = qmlog::None ;
    else if(image_type=="RD")
      syslog_level = qmlog::Notice ;
    else if(image_type=="TR")
      syslog_level = qmlog::Info ;
  }
#endif

  qmlog::syslog()->reduce_max_level(syslog_level) ;

  qmlog::log_file *varlog = new qmlog::log_file("/var/log/timed.log", varlog_level) ;
  varlog->enable_fields(qmlog::Monotonic_Milli | qmlog::Time_Milli) ;

  bool isatty_2 = isatty(2) ;

  if (not isatty_2) // stderr is not a terminal -> no stderr logging
    delete qmlog::stderr() ;

  LIBTIMED_LOGGING_DISPATCHER->set_proxy(qmlog::dispatcher()) ;

  log_notice("time daemon started, debug_flag=%d, syslog-level=%d /var/log-level=%d isatty(2)=%d", debug_flag, qmlog::syslog()->log_level(), varlog->log_level(), isatty_2) ;

#if F_IMAGE_TYPE
  log_notice("image_type='%s'", image_type.c_str()) ;
#endif

  try
  {
    event_t::codec_initializer() ;
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
