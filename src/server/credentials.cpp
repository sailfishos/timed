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
#include "f.h"

#include <stdlib.h>
#include <sys/creds.h>

#include <QDBusReply>

#include <qm/log>

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

bool credentials_t::apply() const
{
#if F_CREDS_AEGIS_LIBCREDS
  creds_t aegis_creds_want = Aegis::credentials_to_creds_t(*this) ;

  bool res = creds_set(aegis_creds_want) == 0 ;

  if (!res)
    log_error("aegis cred_set() failed") ;

  creds_free(aegis_creds_want) ;

  return res ;
#else // F_CREDS_AEGIS_LIBCREDS
#error credentials_t::apply() is only implemented for F_CREDS_AEGIS_LIBCREDS
#endif
}

bool credentials_t::apply_and_compare()
{
  if (! apply()) // can't apply: nothing to check
    return false ;

  credentials_t current = credentials_t::from_current_process() ;

  ostringstream os ;

  bool equal = true ;

#define COMMA (os << (equal ? "" : ", ") )

  if (current.uid != uid)
    COMMA << "current uid='" << current.uid << "' (requested uid='" << uid <<"')", equal = false ;

  if (current.gid != gid)
    COMMA << "current gid='" << current.gid << "' (requested gid='" << gid <<"')", equal = false ;

#if F_TOKENS_AS_CREDENTIALS

  int all_accrued = true ;
#define COMMA_A (all_accrued ? COMMA << "tokens not present: {" : os << ", ")

  for (set<string>::const_iterator it=tokens.begin(); it!=tokens.end(); ++it)
    if (current.tokens.count(*it)==0)
      COMMA_A << "'" << *it << "'", all_accrued = false ;
  if (!all_accrued)
    os << "}" ;

  equal = equal && all_accrued ;

  int all_dropped = true ;
#define COMMA_D (all_dropped ? COMMA << "tokens not dropped: {" : os << ", ")

  for (set<string>::const_iterator it=current.tokens.begin(); it!=current.tokens.end(); ++it)
    if (tokens.count(*it)==0)
      COMMA_D << "'" << *it << "'", all_accrued = false ;
  if (!all_dropped)
    os << "}" ;

  equal = equal && all_dropped ;

#undef COMMA_A
#undef COMMA_D

#endif // F_TOKENS_AS_CREDENTIALS

#undef COMMA

  if(!equal)
    log_warning("applied and wanted credentials differ: %s", os.str().c_str()) ;

  return equal ;
}

credentials_t credentials_t::from_current_process()
{
#if F_CREDS_AEGIS_LIBCREDS
  creds_t aegis_creds = creds_gettask(0) ;
  credentials_t creds = Aegis::credentials_from_creds_t(aegis_creds) ;

  creds_free(aegis_creds) ;

  return creds ;
#else // not F_CREDS_AEGIS_LIBCREDS
#error credentials_t::from_current_process() is only implemented for F_CREDS_AEGIS_LIBCREDS
#endif
}

// TODO: F_CREDS_UID
//       implement the same function without aegis, asking UID of the caller and
//       setting this UID and the caller's default GID as only available credentials
//
// TODO: F_CREDS_NOBODY
//       implement the same function setting nobody/nogroup as credentials
//
// TODO: F_CREDS_AEGIS_LIBCREDS --- make this function #ifdef'ed

credentials_t credentials_t::from_dbus_connection(const QDBusMessage &message)
{
#if F_CREDS_AEGIS_LIBCREDS
  return Aegis::credentials_from_dbus_connection(message) ;
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

void credentials_t::load(const iodata::record *r)
{
  uid = r->get("uid")->str() ;
  gid = r->get("gid")->str() ;
#if F_TOKENS_AS_CREDENTIALS
  const iodata::array *tok = r->get("tokens")->arr() ;
  for(unsigned i=0; i<tok->size(); ++i)
    tokens.insert(tok->get(i)->str()) ;
#endif
}
