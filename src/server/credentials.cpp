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

#include "credentials.h"
#include "log.h"

#include <sys/creds.h>

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <dbus/dbus.h>

#define CS(s) (s).toLocal8Bit().constData()
#define US(s) (s).toUtf8().constData()

#define BOINK() log_warning("!!! %s:%d: BOINK\n", __FILE__,__LINE__)

#define SEPARATOR " "

/* ------------------------------------------------------------------------- *
 * get_owner_pid
 * ------------------------------------------------------------------------- */

static
pid_t
get_owner_pid(QDBusConnection &bus, const char *name)
{
  /* FIXME: this makes a synchronous roundtrip to dbus daemon
   * and back during which time the timed process will be blocked.
   *
   * Note: We can't really handle this asynchronously without
   * handling the whole add_event asynchronously and this would
   * require modifications to the timed event state machine and
   * delaying sending add_event replies from QDBusAbstractAdaptor.
   * At the moment I do not know how to handle either of those ...
   */

  pid_t   result    = -1;
  QString service   = "org.freedesktop.DBus";
  QString path      = "/org/freedesktop/DBus";
  QString interface = "org.freedesktop.DBus";
  QString method    = "GetConnectionUnixProcessID";

  QDBusMessage req  = QDBusMessage::createMethodCall(service,
                                                     path,
                                                     interface,
                                                     method);
  req << QString(name);

  QDBusMessage rsp = bus.call(req);

  if( rsp.type() == QDBusMessage::ReplyMessage )
  {
    QList<QVariant> args = rsp.arguments();
    if( !args.isEmpty() )
    {
      bool ok = false;
      int reply = rsp.arguments().first().toInt(&ok);
      log_warning("@@@ rsp.ok = %d, rsp.reply = %d", ok, reply);
      if( ok ) result = reply;
    }
  }

  log_warning("@@@ owner pid = %d", (int)result);

  return result;
}

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
  return true;
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

static
char *
credentials_to_string(creds_t creds)
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
      BOINK();
      continue;
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
        BOINK();
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
  creds_t  creds = creds_init();
  char    *work  = strdup(input);

  char    *now;
  char    *zen;

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
      BOINK();
      continue;
    }

    if( c_val == CREDS_BAD )
    {
      BOINK();
      continue;
    }

    if( creds_add(&creds, c_type, c_val) == -1 )
    {
      BOINK();
    }
  }

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
    BOINK();
    goto cleanup;
  }

  if( (text = credentials_to_string(creds)) == 0 )
  {
    BOINK();
    goto cleanup;
  }

  result = QString::fromUtf8(text);

cleanup:

  free(text);
  creds_free(creds);

  log_warning("@@@ pid=%d -> creds=%s", pid, CS(result));

  return result;
}

/* ------------------------------------------------------------------------- *
 * credentials_get_from_dbus
 * ------------------------------------------------------------------------- */

QString
credentials_get_from_dbus(QDBusConnection &bus,
                          const QDBusMessage &msg)
{
  QString  result; // assume null string

  pid_t    owner  = -1;
  QString  sender = msg.service(); /* returns "sender" on inbound messages
                                    * and "service" on outbound messages
                                    * which saves one QString object and
                                    * confuses at least me ... */

  log_warning("@@@ service/sender = '%s'", CS(sender));

  if( (owner = get_owner_pid(bus, sender.toUtf8().constData())) == -1 )
  {
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
  bool    success = false;

  creds_t cr_want = 0;
  creds_t cr_have = 0;

  creds_type_t  cr_type = CREDS_BAD;
  creds_value_t cr_val  = CREDS_BAD;

  if( credentials.isEmpty() ) // null string is also empty
  {
    BOINK();
    goto cleanup;
  }

  cr_want = credentials_from_string(credentials.toUtf8().constData());
  if( cr_want == 0 )
  {
    BOINK();
    goto cleanup;
  }

  if( creds_set(cr_want) < 0 )
  {
    BOINK();
    goto cleanup;
  }

  if( (cr_have = creds_gettask(0)) == 0 )
  {
    BOINK();
    goto cleanup;
  }

  // assume success at this point
  success = true;

  int i;

  for( i = 0; (cr_type = creds_list(cr_want, i, &cr_val)) != CREDS_BAD ; ++i )
  {
    if( creds_have_p(cr_have, cr_type, cr_val) )
    {
      creds_sub(cr_have, cr_type, cr_val);
    }
    else
    {
      // missing a required credential
      BOINK();
      success = false;
    }
  }

  if( (cr_type = creds_list(cr_want, 0, &cr_val)) != CREDS_BAD )
  {
    // other than asked for credentials left
      BOINK();
      success = false;
  }

cleanup:

  creds_free(cr_want);
  creds_free(cr_have);

  return success;
}
