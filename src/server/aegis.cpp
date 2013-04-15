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
#include "f.h"

#include <cstring>

#if F_CREDS_AEGIS_LIBCREDS
#include <sys/creds.h> // move it to creds.h or credentials.h
#endif

#include "../common/log.h"
#include <iodata/iodata>

#include "timed/interface"
#include "credentials.h"

#include "aegis.h"

#if F_CREDS_AEGIS_LIBCREDS
credentials_t Aegis::credentials_from_creds_t(creds_t aegis_creds)
{
  credentials_t creds ; // uid/gid is set to nobody/nobody by default

  // using a buffer directly on stack, if not enough -> use heap
  static const size_t buf_stack_len = 1024 ;
  char buf_on_stack[buf_stack_len] ;

  char *buf = buf_on_stack ;
  size_t len = buf_stack_len ;

  creds_type_t aegis_type ;
  creds_value_t aegis_val ;
  for(int i=0; (aegis_type = creds_list(aegis_creds, i,  &aegis_val)) != CREDS_BAD; ++i)
  {
    int res1 = creds_creds2str(aegis_type, aegis_val, buf, len) ;
    bool error1 = res1 < 0 ;
    bool memory1 = len <= (size_t)res1 ;
    bool failure1 = error1 || memory1 ;

    if (failure1 && buf!=buf_on_stack)
      delete [] buf ;

    if (error1)
    {
      log_error("creds_creds2str() failed (%d returned)", res1) ;
      return credentials_t() ; // new empty object, being paranoid
    }

    if (memory1) // have to re-alloc
    {
      len = (size_t)res1 + 1, buf = new char[len] ;
      int res2 = creds_creds2str(aegis_type, aegis_val, buf, len) ;
      bool failure2 = res2 < 0 || (len <= (size_t)res2) ;
      if(failure2)
      {
        log_error("creds_creds2str() failed second time (len=%u, returned value=%d)", len, res2) ;
        delete [] buf ;
        return credentials_t() ;
      }
    }

    // now 'buf' is containing a proper '\0'-terminated c-string.

    bool is_token = true ;
    static const char *id[] = {"UID::", "GID::"} ; // loop over 'uid' and 'gid'
    static const int prefix_len = 5 ;

    for (int j=0; j<2; ++j)
      if (strncmp(buf, id[j], prefix_len)==0) // found!
      {
        is_token = false ;
        const char *shifted = buf + prefix_len ;
        if(*shifted == '\0')
        {
          log_warning("got empty uid/gid credential value in '%s'", buf) ;
          break ;
        }

        (j==0 ? creds.uid : creds.gid) = (string) shifted ;
        break ;
      }

    if(is_token)
      creds.tokens.insert(buf) ;
  }

  if (buf!=buf_on_stack)
    delete [] buf ;

  return creds ;
}

bool Aegis::add_string_to_creds_t(creds_t &aegis_creds, const string &token, bool silent)
{
  creds_value_t aegis_val ;
  creds_type_t aegis_type = creds_str2creds(token.c_str(), &aegis_val) ;
  if (aegis_type == CREDS_BAD || aegis_val == CREDS_BAD)
  {
    if (!silent)
      log_error("failed to recognize aegis token '%s': bad %s", token.c_str(), aegis_type == CREDS_BAD ? "type" : "value") ;
    return false ;
  }

  if (creds_add(&aegis_creds, aegis_type, aegis_val) == -1)
  {
    if (!silent)
      log_error("aegis creds_add() failed for token '%s'", token.c_str()) ;
    return false ;
  }

  return true ;
}

creds_t Aegis::credentials_to_creds_t(const credentials_t &creds)
{
  creds_t aegis_creds = creds_init() ;

  bool ok = true ;
  for(set<string>::const_iterator it=creds.tokens.begin(); it!=creds.tokens.end() && ok; ++it)
    ok = Aegis::add_string_to_creds_t(aegis_creds, *it, false) ;
  ok = ok && Aegis::add_string_to_creds_t(aegis_creds, (string)"UID::" + creds.uid, false) ;
  ok = ok && Aegis::add_string_to_creds_t(aegis_creds, (string)"GID::" + creds.gid, false) ;

  if (!ok)
    creds_free(aegis_creds), aegis_creds = creds_init() ;

  return aegis_creds ;
}
#endif // F_CREDS_AEGIS_LIBCREDS

#if F_CREDS_AEGIS_LIBCREDS
credentials_t Aegis::credentials_from_dbus_connection(const QDBusMessage &message)
{
  // We are doing this in a kinda insecure way. Two steps:
  // 1. Ask dbus daemon, what is the pid of the client.
  // --- race race race --- (please someone file a bug about it) --- race race race ---
  // 2. Ask aegis kernel extension, what are the credentials of given pid.

  QString sender = message.service() ;
  /* "returns "sender" on inbound messages
      and "service" on outbound messages
      which saves one QString object and
      confuses at least me ..." -- so true ! */

  // 1. Ask DBus daemon, what is the PID of the 'sender':

  uint32_t owner_id = get_name_owner_from_dbus_sync(Maemo::Timed::bus(), sender) ;

  if (owner_id == ~0u)
  {
    log_warning("can't get owner (pid) of the caller, already terminated?") ;
    return credentials_t() ;
  }

  pid_t pid = owner_id ;

  // 2. Getting aegis credentials from the kernel, by pid

  creds_t aegis_creds = creds_gettask(pid) ;

  // Don't check result, as NULL is a valid set of aegis credentials

  credentials_t creds = Aegis::credentials_from_creds_t(aegis_creds) ;

  creds_free(aegis_creds) ;

  return creds ;
}

#endif // F_CREDS_AEGIS_LIBCREDS
