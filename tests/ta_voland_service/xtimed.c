/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include <glib.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "xtimed.h"
#include "xdbus.h"
#include "voland.h"

/** Is timed process running i.e. does TIMED_DBUS_SERVICE have an owner */
static bool xtimed_running = false;

/** D-Bus connection for communicating with TIMED_DBUS_SERVICE */
static DBusConnection *xtimed_bus = 0;

/** NameOwnerChanged tracking rule for TIMED_DBUS_SERVICE */
static const char rule_NameOwnerChanged_timed[] =
"type='signal'"
",sender='"DBUS_SERVICE_DBUS"'"
",interface='"DBUS_INTERFACE_DBUS"'"
",member='NameOwnerChanged'"
",path='"DBUS_PATH_DBUS"'"
",arg0='"TIMED_DBUS_SERVICE"'"
;

/** Callback for reporting changes in timed run state */
static void (*xtimed_runstate_cb)(bool running) = 0;

/** Setup callback for reporting changes in timed run state
 *
 * If non null cb is set, the current timed runstate will be immediately
 * reported via it.
 *
 * @param cb callback function to be called when timed runstate changes,
 *           or NULL to stop reporting
 */
void
xtimed_set_runstate_cb(void (*cb)(bool))
{
  if( (xtimed_runstate_cb = cb) )
  {
    xtimed_runstate_cb(xtimed_running);
  }
}

/** Set timed run state and report via callback if one is set
 *
 * @param running whether TIMED_DBUS_SERVICE has an owner or not
 */
static
void
xtimed_set_runstate(bool running)
{
  if( xtimed_running != running )
  {
    xtimed_running = running;
    printf("%s(%s)\n", __FUNCTION__, running ? "true" : "false");

    if( xtimed_runstate_cb )
    {
      xtimed_runstate_cb(xtimed_running);
    }
  }
}

/** Send dialog responce to timed
 *
 * NOTE: This call blocks until reply from timed is received
 *
 * @param cookie unique alarm identifier
 * @param button action performed by the user
 *
 * @return true if method call could be sent succesfully
 */
bool
xtimed_dialog_response(unsigned cookie, int button)
{
  bool res = false;

  dbus_uint32_t c = cookie;
  dbus_int32_t  b = button;
  dbus_bool_t   ack = false;

  DBusMessage *req = 0;
  DBusMessage *rsp = 0;
  DBusError    err = DBUS_ERROR_INIT;

  req = dbus_message_new_method_call(TIMED_DBUS_SERVICE,
                                     TIMED_DBUS_PATH,
                                     TIMED_DBUS_INTERFACE,
                                     TIMED_DBUS_METHOD_DIALOG_RESPONSE);
  if( !req ) goto cleanup;

  if( !dbus_message_append_args(req,
                                DBUS_TYPE_UINT32, &c,
                                DBUS_TYPE_INT32, &b,
                                DBUS_TYPE_INVALID) )
  {
    goto cleanup;
  }

  // FIXME: must be done via pending call mechanism or deadlocks will happen

  rsp = xdbus_call_method(xtimed_bus, req);

  if( !rsp ) goto cleanup;

  if( !dbus_message_get_args(rsp, &err,
                             DBUS_TYPE_BOOLEAN, &ack,
                             DBUS_TYPE_INVALID) )
  {
    printf("%s: %s: %s\n", "dialog_response", err.name, err.message);
    goto cleanup;
  }

  res = ack;

cleanup:

  dbus_error_free(&err);

  if( rsp ) dbus_message_unref(rsp);
  if( req ) dbus_message_unref(req);

  return res;
}

/** D-Bus message filter callback
 *
 * Currently used only for handling TIMED_DBUS_SERVICE NameOwnerChanged
 * signals from dbus daemon.
 *
 * @param conn      D-Bus connection
 * @param msg       Message to be filtered
 * @param user_data not used
 *
 * @return DBUS_HANDLER_RESULT_NOT_YET_HANDLED
 */

static
DBusHandlerResult
xtimed_filter(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
  DBusHandlerResult   result    = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  const char         *interface = dbus_message_get_interface(msg);
  const char         *member    = dbus_message_get_member(msg);
  const char         *object    = dbus_message_get_path(msg);
  int                 type      = dbus_message_get_type(msg);
  const char         *type_name = dbus_message_type_to_string(type);
  DBusError          err        = DBUS_ERROR_INIT;

  if( !interface || !member || !object )
  {
    goto cleanup;
  }

  if( type == DBUS_MESSAGE_TYPE_SIGNAL )
  {
    if( !strcmp(interface, DBUS_INTERFACE_DBUS) )
    {
      if( !strcmp(member, "NameOwnerChanged") )
      {
        printf("----------------------------------------------------------------\n");
        printf("t:%s, i:%s, m:%s o:%s\n", type_name, interface, member, object);

        const char *name = 0;
        const char *prev = 0;
        const char *curr = 0;

        if( !dbus_message_get_args(msg, &err,
                                   DBUS_TYPE_STRING, &name,
                                   DBUS_TYPE_STRING, &prev,
                                   DBUS_TYPE_STRING, &curr,
                                   DBUS_TYPE_INVALID) )
        {
          fprintf(stderr, "%s: %s: %s\n", "dbus_message_get_args", err.name, err.message);
          goto cleanup;
        }

        if( !strcmp(name, TIMED_DBUS_SERVICE) )
        {
          xtimed_set_runstate(*curr);
        }
      }
    }
  }

cleanup:

  dbus_error_free(&err);

  return result;
}

/** Initialize module functionality
 *
 * @return true on success, false on errors
 */

bool
xtimed_init(void)
{
  printf("%s()\n", __FUNCTION__);

  bool       res = false;
  DBusError  err = DBUS_ERROR_INIT;

  if( !(xtimed_bus = dbus_bus_get(DBUS_BUS_SYSTEM, &err)) )
  {
    fprintf(stderr, "%s: %s: %s\n", "dbus_bus_get", err.name, err.message);
    goto cleanup;
  }

  if( !dbus_connection_add_filter(xtimed_bus, xtimed_filter, 0, 0) )
  {
    goto cleanup;
  }

  dbus_bus_add_match(xtimed_bus, rule_NameOwnerChanged_timed, 0);

  if( dbus_bus_name_has_owner(xtimed_bus, TIMED_DBUS_SERVICE, &err) )
  {
    xtimed_set_runstate(true);
  }
  else if( dbus_error_is_set(&err) )
  {
    fprintf(stderr, "%s: %s: %s\n", "dbus_bus_name_has_owner", err.name, err.message);
    dbus_error_free(&err);
  }

  dbus_connection_setup_with_g_main(xtimed_bus, NULL);

  res = true;

cleanup:
  dbus_error_free(&err);
  return res;
}

/** Release any resources used by the module
 */
void
xtimed_quit(void)
{
  printf("%s()\n", __FUNCTION__);
  if( xtimed_bus )
  {
    dbus_connection_remove_filter(xtimed_bus, xtimed_filter, 0);

    dbus_connection_unref(xtimed_bus), xtimed_bus = 0;
  }
}
