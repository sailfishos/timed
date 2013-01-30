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

#include "ta_voland.h"
#include "voland.h"
#include "xtimed.h"
#include "xdbus.h"
#include "mainloop.h"

#include "stab.h"
#include "button.h"
#include "event.h"

/* ========================================================================= *
 * DELAYED QUIT
 * ========================================================================= */

/** Timer id for the "quit in N seconds" */
static guint  delayed_quit_id = 0;

/** Quit after timer is triggered
 *
 * @param user_data not used
 *
 * @return FALSE (to stop the timer from repeating)
 */
static
gboolean
ta_voland_delayed_exit_cb(gpointer user_data)
{
  if( delayed_quit_id )
  {
    delayed_quit_id = 0;

    printf("TERMINATING\n");
    mainloop_stop(EXIT_SUCCESS);
  }

  return FALSE;
}

/** Initiate delayed exit
 */
static
bool
ta_voland_request_delayed_exit(void)
{
  if( delayed_quit_id )
  {
    printf("timeout already programmed\n");
    return false;
  }

  printf("program timeout\n");
  delayed_quit_id = g_timeout_add_seconds(3, ta_voland_delayed_exit_cb, 0);

  return (delayed_quit_id != 0);
}

/* ========================================================================= *
 *
 * DBUS MESSAGE HANDLING
 *
 * ========================================================================= */

/** Handler for com.nokia.ta_voland.pid method call
 *
 * @param msg method call request message
 *
 * @return reply message to send back to the caller
 */
static DBusMessage *ta_voland_pid_cb(DBusMessage *msg)
{
  DBusMessage  *rsp = dbus_message_new_method_return(msg);
  dbus_int32_t  pid = voland_pid();

  printf("%s() -> %d\n", __FUNCTION__, pid);

  dbus_message_append_args(rsp,
                           DBUS_TYPE_INT32, &pid,
                           DBUS_TYPE_INVALID);

  return rsp;
}

/** Handler for com.nokia.ta_voland.answer method call
 *
 * @param msg method call request message
 *
 * @return reply message to send back to the caller
 */
static DBusMessage *ta_voland_answer_cb(DBusMessage *msg)
{
  DBusMessage *rsp = dbus_message_new_method_return(msg);
  dbus_bool_t  ack = FALSE;
  DBusError    err = DBUS_ERROR_INIT;

  dbus_uint32_t cookie = 0;
  dbus_int32_t  button = 0;

  if( !dbus_message_get_args(msg, &err,
                            DBUS_TYPE_UINT32, &cookie,
                            DBUS_TYPE_INT32,  &button,
                            DBUS_TYPE_INVALID) )
  {
    fprintf(stderr, "%s: %s: %s\n", "dbus_message_get_args", err.name, err.message);
    goto cleanup;
  }

  ack = voland_answer(cookie, button);

  cleanup:

  printf("%s(%u, %d) -> %d\n", __FUNCTION__, cookie, button, ack);

  dbus_message_append_args(rsp,
                           DBUS_TYPE_BOOLEAN, &ack,
                           DBUS_TYPE_INVALID);

  dbus_error_free(&err);

  return rsp;
}

/** Handler for com.nokia.ta_voland.top method call
 *
 * @param msg method call request message
 *
 * @return reply message to send back to the caller
 */
static DBusMessage *ta_voland_top_cb(DBusMessage *msg)
{
  DBusMessage  *rsp = dbus_message_new_method_return(msg);

  dbus_uint32_t cookie = voland_top();

  printf("%s() -> %u\n", __FUNCTION__, cookie);

  dbus_message_append_args(rsp,
                           DBUS_TYPE_UINT32, &cookie,
                           DBUS_TYPE_INVALID);

  return rsp;
}

/** Handler for com.nokia.ta_voland.quit method call
 *
 * @param msg method call request message
 *
 * @return reply message to send back to the caller
 */
static DBusMessage *ta_voland_quit_cb(DBusMessage *msg)
{
  DBusMessage  *rsp = dbus_message_new_method_return(msg);
  dbus_bool_t   ack = ta_voland_request_delayed_exit();

  printf("%s() -> %d\n", __FUNCTION__, ack);

  dbus_message_append_args(rsp,
                           DBUS_TYPE_BOOLEAN, &ack,
                           DBUS_TYPE_INVALID);

  return rsp;
}

/** DBus message filter
 *
 * @param conn dbus connection
 * @param msg  message to be filtered
 * @param user_data not used
 *
 * @return DBUS_HANDLER_RESULT_HANDLED if the msg was method call
 *         succesfully handled by the filter, otherwise
 *         DBUS_HANDLER_RESULT_NOT_YET_HANDLED
 */
static
DBusHandlerResult
ta_voland_filter(DBusConnection *conn,
              DBusMessage *msg,
              void *user_data)
{
  DBusHandlerResult   result    = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  const char         *interface = dbus_message_get_interface(msg);
  const char         *member    = dbus_message_get_member(msg);
  const char         *object    = dbus_message_get_path(msg);
  int                 type      = dbus_message_get_type(msg);
  const char         *type_name = dbus_message_type_to_string(type);

  DBusMessage        *rsp       = 0;
  DBusError          err        = DBUS_ERROR_INIT;

  if( !interface || !member || !object )
  {
    goto cleanup;
  }

  if( type == DBUS_MESSAGE_TYPE_METHOD_CALL )
  {
    if( !strcmp(interface, TA_VOLAND_DBUS_INTERFACE) )
    {
      printf("----------------------------------------------------------------\n");
      printf("t:%s, i:%s, m:%s o:%s\n", type_name, interface, member, object);
      //xdbusrx_emit(msg);

      if( !strcmp(member, TA_VOLAND_DBUS_METHOD_PID) )
      {
        rsp = ta_voland_pid_cb(msg);
      }
      else if( !strcmp(member, TA_VOLAND_DBUS_METHOD_ANSWER) )
      {
        rsp = ta_voland_answer_cb(msg);
      }
      else if( !strcmp(member, TA_VOLAND_DBUS_METHOD_TOP) )
      {
        rsp = ta_voland_top_cb(msg);
      }
      else if( !strcmp(member, TA_VOLAND_DBUS_METHOD_QUIT) )
      {
        rsp = ta_voland_quit_cb(msg);
      }
      else
      {
        printf("unhandled %s\n", member);
      }

      if( rsp == 0 && !dbus_message_get_no_reply(msg) )
      {
        rsp = dbus_message_new_error(msg, DBUS_ERROR_FAILED, member);
      }

      if( rsp != 0 )
      {
        dbus_connection_send(conn, rsp, 0);
      }

      result = DBUS_HANDLER_RESULT_HANDLED;
    }
  }

cleanup:

  if( rsp != 0 )
  {
    dbus_message_unref(rsp);
  }

  dbus_error_free(&err);

  return result;
}

/* ========================================================================= *
 *
 * MODULE INIT/QUIT
 *
 * ========================================================================= */

/** D-Bus connection for serving com.nokia.ta_voland on */
static DBusConnection *ta_voland_bus = 0;

/** Initialize com.nokia.ta_voland service
 *
 * @return true if service was succesfully started, or false in case of errors
 */
bool
ta_voland_init(void)
{
  printf("%s()\n", __FUNCTION__);
  bool        res  = false;
  DBusError   err  = DBUS_ERROR_INIT;
#if 0
  DBusBusType type = DBUS_BUS_SESSION;
#else
  DBusBusType type = DBUS_BUS_SYSTEM;
#endif

  if( !(ta_voland_bus = dbus_bus_get(type, &err)) )
  {
    fprintf(stderr, "%s: %s: %s\n", "dbus_bus_get", err.name, err.message);
    goto cleanup;
  }

  if( !dbus_connection_add_filter(ta_voland_bus, ta_voland_filter, 0, 0) )
  {
    goto cleanup;
  }

  int rc = dbus_bus_request_name(ta_voland_bus, TA_VOLAND_DBUS_SERVICE,
                                 DBUS_NAME_FLAG_DO_NOT_QUEUE, &err);

  if( dbus_error_is_set(&err) )
  {
    fprintf(stderr, "%s: %s: %s\n", "dbus_bus_request_name", err.name, err.message);
    goto cleanup;
  }

  if( rc != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER )
  {
    fprintf(stderr, "not primary owner\n");
    goto cleanup;
  }

  dbus_connection_setup_with_g_main(ta_voland_bus, NULL);

  voland_set_ta_mode(true);

  res = true;

cleanup:
  dbus_error_free(&err);
  return res;
}

/** Stop com.nokia.ta_voland service
 */
void
ta_voland_quit(void)
{
  printf("%s()\n", __FUNCTION__);
  if( ta_voland_bus )
  {
    voland_set_ta_mode(false);

    dbus_connection_remove_filter(ta_voland_bus, ta_voland_filter, 0);

    dbus_connection_unref(ta_voland_bus), ta_voland_bus = 0;
  }
}
