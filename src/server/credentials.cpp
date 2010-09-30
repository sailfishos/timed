/***************************************************************************
**                                                                        **
**   Copyright (C) 2010 Nokia Corporation.                                **
**                                                                        **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
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
#include "creds.h"

#define CSTR(s) (s).toLocal8Bit().constData()
#define UTF8(s) (s).toUtf8().constData()

#define SEPARATOR " "

#if F_DBUS_INFO_AS_CREDENTIALS

#if 0
/* ------------------------------------------------------------------------- *
 * credentials_get_name_owner
 * ------------------------------------------------------------------------- */

pid_t
credentials_get_name_owner(const QDBusConnection &bus, const QString &name)
{
  pid_t   result    = -1; // assume failure

  QString service   = "org.freedesktop.DBus";
  QString path      = "/org/freedesktop/DBus";
  QString interface = "org.freedesktop.DBus";
  QString method    = "GetConnectionUnixProcessID";
  // "GetConnectionUnixUser" gives us the client UID as UInt32
  // It seems, we can't get GID just by asking dbus daemon.

  QDBusMessage req  = QDBusMessage::createMethodCall(service,
                                                     path,
                                                     interface,
                                                     method);
  req << name;

  QDBusMessage rsp = bus.call(req);

  // FIXME: there must be more elegant ways to handle error
  // replies and type errors...

  if( rsp.type() != QDBusMessage::ReplyMessage )
  {
    log_error("%s: did not get a valid reply", CSTR(method));
  }
  else
  {
    QList<QVariant> args = rsp.arguments();

    if( args.isEmpty() )
    {
      log_error("%s: reply has no return values", CSTR(method));
    }
    else
    {
      bool ok = false;
      int pid = rsp.arguments().first().toInt(&ok);
      // int or unsigned? toInt or toUInt ?

      if( !ok )
      {
        log_error("%s: return values is not an integer", CSTR(method));
      }
      else
      {
        result = pid;
      }
    }
  }

  return result;
}
#else

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
#endif // if 0 else

#endif // F_DBUS_INFO_AS_CREDENTIALS

bool credentials_t::apply() const
{
#if F_CREDS_AEGIS_LIBCREDS
  creds_t aegis_creds_want = aegis_credentials_to_creds_t(*this) ;

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
  credentials_t creds = aegis_credentials_from_creds_t(aegis_creds) ;

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
  return aegis_credentials_from_dbus_connection(message) ;
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

#if DEAD_CODE

/* ------------------------------------------------------------------------- *
 * xrealloc
 * ------------------------------------------------------------------------- */

static
bool
xrealloc(void *pptr, size_t size)
{
  // *pptr is not changed if false is returned

  bool  res  = true;
  void *prev = *(void **)pptr;
  void *curr = 0;

  // if malloc(0) does (and it can) return non-NULL
  // values, realloc(NULL, 0) will too -> explicitly
  // handle new size of 0 as free ...

  if( size == 0 )
  {
    free(prev);
  }
  else if( (curr = realloc(prev, size)) == 0 )
  {
    res = false, curr = prev;
  }

  *(void **)pptr = curr;
  return res;
}

/* ------------------------------------------------------------------------- *
 * xappend
 * ------------------------------------------------------------------------- */

static
bool
xappend(char **pstr, const char *add)
{
  // *pstr is not changed if false is returned

  char  *res = 0;
  char  *cur = *pstr;

  size_t a = cur ? strlen(cur) : 0;
  size_t b = add ? strlen(add) : 0;

  if( (res = (char *)malloc(a + b + 1)) == 0 )
  {
    goto cleanup;
  }

  memcpy(res+0, cur, a);
  memcpy(res+a, add, b);
  res[a+b] = 0;

  *pstr = res;

cleanup:

  return (res != 0);
}

/* ------------------------------------------------------------------------- *
 * credentials_to_string
 * ------------------------------------------------------------------------- */

char *credentials_to_string(creds_t creds)
{
  bool          success = false;
  char         *result  = 0;

  creds_type_t  cr_type = CREDS_BAD;
  creds_value_t cr_val  = CREDS_BAD;
  char         *cr_str  = 0;
  size_t        cr_len  = 32;

  int i,rc;

  if( (cr_str = (char *)malloc(cr_len)) == 0 )
  {
    goto cleanup;
  }

  for( i = 0; (cr_type = creds_list(creds, i,  &cr_val)) != CREDS_BAD ; ++i )
  {
    if( (rc = creds_creds2str(cr_type, cr_val, cr_str, cr_len)) < 0 )
    {
      log_error("%s: failed", "creds_creds2str");
      goto cleanup;
    }

    if( (size_t)rc >= cr_len )
    {
      cr_len = (size_t)rc + 1;

      if( !xrealloc(&cr_str, cr_len) )
      {
        goto cleanup;
      }

      if( creds_creds2str(cr_type, cr_val, cr_str, cr_len) != rc )
      {
        log_error("%s: failed", "creds_creds2str");
        goto cleanup;
      }
    }

    if( result != 0 && !xappend(&result, SEPARATOR) )
    {
      goto cleanup;
    }
    if( !xappend(&result, cr_str) )
    {
      goto cleanup;
    }
  }

  success = true;

cleanup:

  if( !success )
  {
    // all or nothing
    free(result), result = 0;
  }

  free(cr_str);
  return result;
}

/* ------------------------------------------------------------------------- *
 * credentials_from_string
 * ------------------------------------------------------------------------- */

static
creds_t
credentials_from_string(const char *input)
{
  bool     error = true;   // assume failure
  creds_t  creds = 0;      // value to return

  char    *work  = 0;      // non const copy of the input string
  char    *now;            // parsing pointers
  char    *zen;

  if( !input || !(work = strdup(input)) )
  {
    goto cleanup;
  }

  creds = creds_init(); // no checking: NULL is valid credential too

  for( now = work; now; now = zen )
  {
    creds_type_t  c_type = CREDS_BAD;
    creds_value_t c_val  = CREDS_BAD;

    if( (zen = strstr(now, SEPARATOR)) != 0 )
    {
      *zen++ = 0;
    }

    if( (c_type = creds_str2creds(now, &c_val)) == CREDS_BAD )
    {
      log_error("%s: %s -> %s", "creds_str2creds", "now", "bad type");
      goto cleanup;
    }
    if( c_val == CREDS_BAD )
    {
      log_error("%s: %s -> %s", "creds_str2creds", "now", "bad value");
      goto cleanup;
    }

    if( creds_add(&creds, c_type, c_val) == -1 )
    {
      log_error("%s: failed", "creds_add");
      goto cleanup;
    }
  }

  error = false;

cleanup:

  // all or nothing
  if( error ) creds_free(creds), creds = 0;

  free(work);

  return creds;
}

/* ------------------------------------------------------------------------- *
 * credentials_get_from_pid
 * ------------------------------------------------------------------------- */

static
QString
credentials_get_from_pid(pid_t pid)
{
  QString  result; // assume null string

  creds_t  creds = 0;
  char    *text  = 0;

  if( (creds = creds_gettask(pid)) == 0 )
  {
    log_error("%s: failed", "creds_gettask");
    goto cleanup;
  }

  if( (text = credentials_to_string(creds)) == 0 )
  {
    log_error("could not convert credentials to text");
    goto cleanup;
  }

  result = QString::fromUtf8(text);

cleanup:

  free(text);
  creds_free(creds);

  return result;
}

/* ------------------------------------------------------------------------- *
 * credentials_get_from_dbus
 * ------------------------------------------------------------------------- */

QString
credentials_get_from_dbus(QDBusConnection &bus,
                          const QDBusMessage &msg)
{
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

  QString  result; // assume null string

  pid_t    owner  = -1;
  QString  sender = msg.service(); /* returns "sender" on inbound messages
                                    * and "service" on outbound messages
                                    * which saves one QString object and
                                    * confuses at least me ... */

  if( (owner = credentials_get_name_owner(bus, sender)) == -1 )
  {
    log_error("could not get owner of dbus name");
    goto cleanup;
  }

  result = credentials_get_from_pid(owner);

cleanup:

  return result;
}

/* ------------------------------------------------------------------------- *
 * credentials_set
 * ------------------------------------------------------------------------- */

bool
credentials_set(QString credentials)
{
  // assume failure
  bool          success = false;

  creds_t       cr_want = 0;
  creds_t       cr_have = 0;

  creds_type_t  cr_type = CREDS_BAD;
  creds_value_t cr_val  = CREDS_BAD;

  int i;
  char t[64];

  if( credentials.isEmpty() ) // null string is also empty
  {
    log_error("not setting empty/null credentials");
    goto cleanup;
  }

  if( (cr_want = credentials_from_string(UTF8(credentials))) == 0 )
  {
    log_error("failed to convert string to credentials");
    goto cleanup;
  }

  if( creds_set(cr_want) < 0 )
  {
    log_error("%s: failed", "creds_set");
    goto cleanup;
  }

  if( (cr_have = creds_gettask(0)) == 0 )
  {
    log_error("%s: failed", "creds_gettask");
    goto cleanup;
  }

  // assume success at this point
  success = true;

  // check if we actually have all the credentials requested
  for( i = 0; (cr_type = creds_list(cr_want, i, &cr_val)) != CREDS_BAD ; ++i )
  {
    if( creds_have_p(cr_have, cr_type, cr_val) )
    {
      // remove credential -> the set will be empty if
      // we got only credentials we asked for
      creds_sub(cr_have, cr_type, cr_val);
    }
    else
    {
      // missing a required credential
      success = false;

      // TODO: is creds_creds2str() guaranteed to return valid C-string?
      *t=0, creds_creds2str(cr_type, cr_val, t, sizeof t);
      log_error("failed to acquire credential: %s", t);
    }
  }

  // iterate credentials we have, but did not ask for
  for( i = 0; (cr_type = creds_list(cr_have, i, &cr_val)) != CREDS_BAD ; ++i )
  {
    success = false;

    // TODO: is creds_creds2str() guaranteed to return valid C-string?
    *t=0, creds_creds2str(cr_type, cr_val, t, sizeof t);
    log_error("failed to drop credential: %s", t);
  }

cleanup:

  creds_free(cr_want);
  creds_free(cr_have);

  return success;
}

#endif // DEAD_CODE
