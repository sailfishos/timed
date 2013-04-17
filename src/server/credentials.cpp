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

#include <stdlib.h>
#if F_CREDS_AEGIS_LIBCREDS
#include <sys/creds.h>
#endif // F_CREDS_AEGIS_LIBCREDS

#include <QDBusReply>

#include "../common/log.h"

#if F_CREDS_UID
#include <pwd.h>
#include <grp.h>
#include "../lib/interface.h"
#endif // F_CREDS_UID

#include "credentials.h"

#include "aegis.h"

#define CSTR(s) (s).toLocal8Bit().constData()
#define UTF8(s) (s).toUtf8().constData()

#if F_DBUS_INFO_AS_CREDENTIALS

// returning:
//   ~0, if error
//   pid of the caller, if aegis
//   uid of the caller, if simple uid creds
//   not compiling otherwise

  /* FIXME: this makes a synchronous roundtrip to dbus daemon
   * and back during which time the timed process will be blocked.
   *
   * Note: We can't really handle this asynchronously without
   * handling the whole add_event asynchronously and this would
   * require modifications to the timed event state machine and
   * delaying sending add_event replies from QDBusAbstractAdaptor.
   * At the moment I do not know how to handle either of those ...
   * ;-(
   */
uint32_t get_name_owner_from_dbus_sync(const QDBusConnection &bus, const QString &name)
{
  // TODO make 'dbus_daemon' interface via QDBusAbstractInterface
  QString service   =  "org.freedesktop.DBus" ;
  QString path      = "/org/freedesktop/DBus" ;
  QString interface =  "org.freedesktop.DBus" ;
#if F_CREDS_AEGIS_LIBCREDS
  QString method    = "GetConnectionUnixProcessID" ;
#elif F_CREDS_UID
  QString method    = "GetConnectionUnixUser" ;
  // It seems, we can't get GID just by asking dbus daemon.
#endif

  QDBusMessage req  = QDBusMessage::createMethodCall(service, path, interface, method);
  req << name;

  QDBusReply<uint> reply = bus.call(req);

  if (reply.isValid())
    return reply.value() ;
  else
  {
    log_error("%s: did not get a valid reply", CSTR(method));
    return ~0 ;
  }
}

#endif // F_DBUS_INFO_AS_CREDENTIALS

uid_t nameToUid(string name)
{
  passwd *info = getpwnam(name.c_str()) ;
  if (info)
    return info->pw_uid ;
  // couldn't get uid for username
  return -1 ;
}

string uidToName(uid_t u)
{
  passwd *info = getpwuid(u) ;
  if (info)
    return info->pw_name ;
  // couldn't get name for uid
  return "nobody" ;
}

gid_t nameToGid(string name)
{
  group *info = getgrnam(name.c_str()) ;
  if (info)
    return info->gr_gid ;
  // couldn't get gid for groupname
  return -1 ;
}

string gidToName(gid_t g)
{
  group *info = getgrgid(g) ;
  if (info)
    return info->gr_name ;
  // couldn't get name for gid
  return "nobody" ;
}

bool credentials_t::apply() const
{
#if F_CREDS_AEGIS_LIBCREDS
  creds_t aegis_creds_want = Aegis::credentials_to_creds_t(*this) ;

  bool res = creds_set(aegis_creds_want) == 0 ;

  if (!res)
    log_error("aegis cred_set() failed") ;

  creds_free(aegis_creds_want) ;

  return res ;
#elif F_CREDS_UID
  if (setgid(nameToGid(gid)) != 0 || setuid(nameToUid(uid)) != 0)
  {
    log_error("uid cred_set() failed") ;
    return false ;
  }
  return true ;
#else
#error unimplemented credentials type
#endif
}

bool credentials_t::apply_and_compare()
{
  if (! apply()) // can't apply: nothing to check
    return false ;

  credentials_t current = credentials_t::from_current_process() ;

  ostringstream os ;

  bool id_matches = true ;

#define COMMA (os << (id_matches ? "" : ", ") )

  if (current.uid != uid)
    COMMA << "current uid='" << current.uid << "' (requested uid='" << uid <<"')", id_matches = false ;

  if (current.gid != gid)
    COMMA << "current gid='" << current.gid << "' (requested gid='" << gid <<"')", id_matches = false ;

#if F_TOKENS_AS_CREDENTIALS

  int all_accrued = true ;
#define COMMA_A (all_accrued ? COMMA << "tokens not present: {" : os << ", ")

  for (set<string>::const_iterator it=tokens.begin(); it!=tokens.end(); ++it)
    if (current.tokens.count(*it)==0)
      COMMA_A << "'" << *it << "'", all_accrued = false ;
  if (!all_accrued)
    os << "}" ;

  int all_dropped = true ;
#define COMMA_D (all_dropped ? COMMA << "tokens not dropped: {" : os << ", ")

  for (set<string>::const_iterator it=current.tokens.begin(); it!=current.tokens.end(); ++it)
    if (tokens.count(*it)==0)
      COMMA_D << "'" << *it << "'", all_accrued = false ;
  if (!all_dropped)
    os << "}" ;

  bool equal = id_matches and all_accrued and all_dropped ;
  bool ret = id_matches and all_dropped ;

#undef COMMA_A
#undef COMMA_D

#else
  bool equal = id_matches ;
  bool ret =  id_matches ;
#endif // F_TOKENS_AS_CREDENTIALS

#undef COMMA

  if(!equal)
    log_warning("applied and wanted credentials differ: %s", os.str().c_str()) ;

  return ret ;
}

credentials_t credentials_t::from_given_process(pid_t pid)
{
#if F_CREDS_AEGIS_LIBCREDS
  creds_t aegis_creds = creds_gettask(pid) ;
  credentials_t creds = Aegis::credentials_from_creds_t(aegis_creds) ;

  creds_free(aegis_creds) ;

  return creds ;
#elif F_CREDS_UID
// TODO: currently nobody:nobody is reported for all processes
  return credentials_t() ;
#else
#error unimplemented credentials type
#endif
}

credentials_t credentials_t::from_current_process()
{
#if F_CREDS_AEGIS_LIBCREDS
  return credentials_t::from_given_process(0) ;
#elif F_CREDS_UID
  return credentials_t(uidToName(getuid()), gidToName(getgid())) ;
#else
#error unimplemented credentials type
#endif
}

// TODO: F_CREDS_UID
//       implement the same function without aegis, asking UID of the caller and
//       setting this UID and the caller's default GID as only available credentials
//
// TODO: F_CREDS_NOBODY
//       implement the same function setting nobody/nobody as credentials
//
// TODO: F_CREDS_AEGIS_LIBCREDS --- make this function #ifdef'ed

credentials_t::credentials_t(const QDBusMessage &message)
: uid("nobody"), gid("nobody")
{
#if F_CREDS_AEGIS_LIBCREDS
  *this = Aegis::credentials_from_dbus_connection(message) ;
#elif F_CREDS_UID
  QString sender = message.service() ;
  uint32_t user_id = get_name_owner_from_dbus_sync(Maemo::Timed::bus(), sender) ;

  if (user_id == ~0u)
    log_warning("can't get user (uid) of the caller, already terminated?") ;
  else
  {
    passwd *info = getpwuid(user_id) ;
    if (info)
    {
      uid = info->pw_name ;
      gid = gidToName(info->pw_gid) ;
    }
  }
#else
#error credentials_t;:from_dbus_connection is only implemented for aegis
#endif
}

iodata::record *credentials_t::save() const
{
  iodata::record *r = new iodata::record ;
  r->add("uid", uid) ;
  r->add("gid", gid) ;
#if F_TOKENS_AS_CREDENTIALS
  iodata::array *tok = new iodata::array ;
  for(set<string>::const_iterator it=tokens.begin(); it!=tokens.end(); ++it)
    tok->add(new iodata::bytes(*it)) ;
  r->add("tokens", tok) ;
#endif

  return r ;
}

credentials_t::credentials_t(const iodata::record *r)
{
  uid = r->get("uid")->str() ;
  gid = r->get("gid")->str() ;
#if F_TOKENS_AS_CREDENTIALS
  const iodata::array *tok = r->get("tokens")->arr() ;
  for(unsigned i=0; i<tok->size(); ++i)
    tokens.insert(tok->get(i)->str()) ;
#endif
}

string credentials_t::str() const
{
  ostringstream os ;
  os << "{uid='" << uid << "', gid='" << gid << "'" ;
#if F_TOKENS_AS_CREDENTIALS
  bool first = true ;
  for (set<string>::const_iterator it=tokens.begin(); it!=tokens.end(); ++it)
    os << (first ? first=false, ", tokens=[" : ", ") << *it ;
  os << (first ? "no tokens" : "]") ;
#endif
  os << "}" ;
  return os.str() ;
}
