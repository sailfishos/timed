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

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <iodata-qt5/iodata>
#include <iodata-qt5/validator>
#include <iodata-qt5/storage>
#else
#include <iodata/iodata>
#include <iodata/validator>
#include <iodata/storage>
#endif

#include "queue.type.h"
#include "settings.type.h"

#include <string>

#include "backup.h"

// The directory name, must match the path mentioned in timedbackup.conf:
static const string backup_dir = "/tmp/.timed-backup" ;

// File names:
static const string backup_queue = backup_dir + "/events.backup.data" ;
static const string backup_settings = backup_dir + "/timed-settings.backup.data" ;

bool execute(const string &command)
{
  const char *cmd = command.c_str() ;

  log_info("executing '%s'", cmd) ;
  int res = system(cmd) ;

  if (res == 0)
  {
    log_info("command   '%s' successfully executed", cmd) ;
    return true ;
  }

  if(res<0)
    log_error("execution '%s' failed: %m", cmd) ;
  else if(WIFSIGNALED(res))
    log_error("command '%s' killed by signal %d", cmd, WTERMSIG(res)) ;
  else if(WIFEXITED(res))
    log_error("command '%s' failed, exit status: %d", cmd, WEXITSTATUS(res)) ;
  else
    log_error("command '%s' failed in some weird way, system() call returned %d", cmd, res) ;

  return false ;
}

void backup_t::dump_data()
{
  log_info("starting to dump backup data to '%s'", backup_dir.c_str()) ;

  bool status = true ;
  status = status && execute((string)"/bin/mkdir " + backup_dir) ;
  status = status && save_queue_as(backup_queue) ;
  status = status && save_settings_as(backup_settings) ;
  status = status && execute((string)"/bin/chmod -R a+rwX " + backup_dir) ;

  if(status)
    log_info("backup data successfully written to '%s'", backup_dir.c_str()) ;
  else
  {
    log_error("data dump aborted due to above error(s), backup not available") ;
    clean_up() ; // backup FW will notify the user, that we failed
  }
}

void backup_t::read_data()
{
  log_info("starting to read restored data from '%s'", backup_dir.c_str()) ;

  bool status = true ;

  // First we just copy the settings file.
  // Even if it contains rubbish, the second copy containing old settings
  // remains in our cache directory and will be used next time after restart.
  status = status && execute((string)"/bin/cp "+backup_settings+" "+timed->get_settings_path()) ;

  // Now we will replace events with 'backup' flag and save the queue
  status = status && read_queue_from(backup_queue) ;

  if(status)
    log_info("backup data successfully restored from '%s'", backup_dir.c_str()) ;
  else
    log_error("failed to restore backup data due to above error(s)") ;
}

void backup_t::clean_up()
{
  execute((string)"/bin/rm -rf " + backup_dir) ;
}

bool backup_t::save_queue_as(const string &path)
{
  iodata::record *r = timed->am->save(true) ;
  iodata::storage *backup = new iodata::storage ;

  backup->set_primary_path(path) ;
  std::string name = "event_queue_t";
  backup->set_validator(events_data_validator(), name) ;
  int res = backup->save(r) ;

  delete backup ;
  delete r ;

  if (res==0)
  {
    log_info("event queue successfully written to '%s'", path.c_str()) ;
    return true ;
  }
  else
  {
    log_error("can't write event queue to '%s', backup failed", path.c_str()) ;
    return false ;
  }
}

bool backup_t::read_queue_from(const string &path)
{
  log_debug() ;
  // First read the file
  iodata::storage *backup = new iodata::storage ;

  backup->set_primary_path(backup_queue) ;
  std::string name = "event_queue_t";
  backup->set_validator(events_data_validator(), name) ;

  iodata::record *r = backup->load() ;

  if (not r) // should never happen!
  {
    log_critical("Failed to read backup data from '%s', weird!", path.c_str()) ;
    return false ;
  }

  if (backup->source()!=0) // means: file is corrupted/non-present, 'r' is just a fake
  {
    log_error("Failed to read backup data from '%s'", path.c_str()) ;
    return false ;
  }

  log_assert(r) ;
  log_assert(backup->source()==0) ;

  // Great, now we have data, but first we have to get rid of old events.

  timed->am->cancel_backup_events() ;
  timed->am->process_transition_queue() ;

  // Now we can put the restored events to the queue
  timed->am->load_events(r->get("events")->arr(), false, false) ;

  // And let the machine process events later
  timed->am->invoke_process_transition_queue() ;

  log_debug("returning 'true'") ;
  return true ;
}

bool backup_t::save_settings_as(const string &path)
{
  iodata::record *r = timed->settings->save() ;
  iodata::storage *backup = new iodata::storage ;

  backup->set_primary_path(path) ;
  backup->set_validator(settings_data_validator(), "settings_t") ;
  int res = backup->save(r) ;

  delete backup ;
  delete r ;

  if (res==0)
  {
    log_info("settings successfully written to '%s'", path.c_str()) ;
    return true ;
  }
  else
  {
    log_error("can't write settings to '%s', backup failed", path.c_str()) ;
    return false ;
  }
}

void backup_t::disable_triggering()
{
  timed->am->state_armed->close() ;
}
