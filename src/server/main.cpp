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

#include "../lib/imagetype.h"
#include "../common/log.h"

#include "f.h"

#include <QMetaType>
int main(int ac, char **av)
{
  const size_t cwd_len = 16 ;
  char cwd[cwd_len], *p = getcwd(cwd, cwd_len) ;
  //bool cwd_is_root = p!=NULL && strcmp(cwd, "/")==0 ;

  // fprintf(stderr, "getcwd returned '%s'", p?cwd:"(null)") ;

//  int syslog_level = qmlog::Warning ;
 // int varlog_level = qmlog::Warning ;

  //bool enable_qmlog = true ;

//  bool debug_flag_timed = access(F_FORCE_DEBUG_PATH, F_OK) == 0 ;
//  bool debug_flag_qmlog = access(QMLOG_ENABLER1, F_OK) == 0 ;
/*
#if F_IMAGE_TYPE
  string image_type = imagetype() ;
  bool debug_flag = debug_flag_timed or debug_flag_qmlog ;
  if (not debug_flag)
  {
    if (image_type=="PR")
    {
      syslog_level = varlog_level = qmlog::None ;
      enable_qmlog = false ; // possibly will change below
    }
    else if(image_type=="RD")
      syslog_level = qmlog::Notice ;
    else if(image_type=="TR")
      syslog_level = qmlog::Info ;
  }
#endif
*/
  //qmlog::syslog()->reduce_max_level(syslog_level) ;

  //const char *log_file = cwd_is_root ? "/var/log/timed.log" : "timed.log" ;
  //qmlog::log_file *varlog = new qmlog::log_file(log_file, varlog_level) ;
  //varlog->enable_fields(qmlog::Monotonic_Milli | qmlog::Time_Milli) ;

  /*
#if F_HOME_LOG
  bool log_file_at_home = access(F_FORCE_HOME_LOG_PATH, F_OK) == 0 ;
  if (log_file_at_home)
  {
    system("touch /home/user/MyDocs/timed.log") ;
    qmlog::log_file *home = new qmlog::log_file("/home/user/MyDocs/timed.log", varlog_level) ;
    home->enable_fields(qmlog::Monotonic_Milli | qmlog::Time_Milli | qmlog::Close_After_Write | qmlog::Cache_If_Cant_Open | qmlog::Dont_Create_File | qmlog::Retry_If_Failed) ;
  }
#endif
*/
//  log_notice("time daemon started; debug_flags: timed=%d, qmlog=%d; syslog-level: %d /var/log-level: %d isatty(2)=%d",
  //        debug_flag_timed, debug_flag_qmlog, qmlog::syslog()->log_level(), varlog->log_level(), isatty_2) ;

#if F_IMAGE_TYPE
  log_notice("image_type='%s'", image_type.c_str()) ;
#endif

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
