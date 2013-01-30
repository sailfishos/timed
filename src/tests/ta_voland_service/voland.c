/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include <sys/types.h>

#include <glib.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <unistd.h>

#include "voland.h"
#include "xtimed.h"
#include "xdbus.h"

#include "stab.h"
#include "button.h"
#include "event.h"

static void voland_stack_rethink(void);
static void voland_remove_from_stack(unsigned cookie);

/** Is the com.nokia.ta_voland interface in control */
static bool voland_ta_mode = false;

/** Alarm stack, the topmost defines the alarm dialog content */
static arr_t *event_stack = 0; // get(i) -> event_t *

/** Unique alarm identifier of the dialog we currently have */
static unsigned dialog_cookie = 0;

/** Timer id for the "simulate button push in N seconds" */
static guint autoreply_timer_id = 0;

/* ========================================================================= *
 *
 * TIMER BASED DIALOG "BUTTON PRESSING"
 *
 * ========================================================================= */

/** Simulate button press after timer is triggered
 *
 * @param user_data unique alarm identifier (cast to pointer)
 *
 * @return FALSE (to stop the timer from repeating)
 */
static
gboolean
voland_autoreply_timer_cb(gpointer user_data)
{
  int      button = 0;
  unsigned cookie = GPOINTER_TO_INT(user_data);

  if( autoreply_timer_id )
  {
    autoreply_timer_id = 0;

    printf("SIMULATE button=%d, cookie=%u\n", button, cookie);
    xtimed_dialog_response(cookie, button);
    voland_remove_from_stack(cookie);
    voland_stack_rethink();
  }

  return FALSE;
}

/** Reprogram the simulated button press timeout if needed
 */
static
void
voland_autoreply_rethink(void)
{
  if( autoreply_timer_id )
  {
    printf("cancel timeout\n");
    g_source_remove(autoreply_timer_id), autoreply_timer_id = 0;
  }

  if( !voland_ta_mode && dialog_cookie )
  {
    printf("program timeout\n");
    autoreply_timer_id = g_timeout_add_seconds(15, voland_autoreply_timer_cb,
                                     GINT_TO_POINTER(dialog_cookie));
  }
}

/* ========================================================================= *
 *
 * DIALOG STACK MANAGEMENT
 *
 * ========================================================================= */

/** Check if current alarm dialog represents the top of alarm stack
 */
static
void
voland_stack_rethink(void)
{
  event_t  *eve = 0;
  unsigned  now = 0;

  if( event_stack )
  {
    if( (eve = arr_tail(event_stack)) )
    {
      now = eve->cookie;
    }
  }

  if( dialog_cookie != now )
  {
    if( !dialog_cookie )
    {
      printf("create dialog %u\n", now);
    }
    else if( !now )
    {
      printf("delete dialog %u\n", dialog_cookie);
    }
    else
    {
      printf("change dialog %u -> %u\n", dialog_cookie, now);
    }
    dialog_cookie = now;
    voland_autoreply_rethink();
  }
}

/** Remove all alarms from the stack
 */
static
void
voland_clear_stack(void)
{
  arr_clear(event_stack);
}

/** Push alarm event to the top of the stack
 *
 * @param eve the event that should show up in the dialog
 */
static
void
voland_push_to_stack(event_t *eve)
{
  printf("@%s(%u)\n", __FUNCTION__, eve->cookie);

  for( size_t i = 0; i < arr_count(event_stack); )
  {
    event_t *tmp = arr_get(event_stack, i);

    if( tmp->cookie == eve->cookie )
    {
      printf("\t%u remove\n", tmp->cookie);
      arr_remove(event_stack, i);
    }
    else
    {
      printf("\t%u keep\n", tmp->cookie);
      ++i;
    }
  }

  printf("\t%u push\n", eve->cookie);
  arr_push(event_stack, eve);
}

/** Remove alarm event from stack
 *
 * @param cookie unique alarm identifier
 */
static
void
voland_remove_from_stack(unsigned cookie)
{
  printf("@%s(%u)\n", __FUNCTION__, cookie);

  for( size_t i = 0; i < arr_count(event_stack); )
  {
    event_t *eve = arr_get(event_stack, i);

    if( eve->cookie == cookie )
    {
      printf("\t%u remove\n", eve->cookie);
      arr_remove(event_stack, i);
    }
    else
    {
      printf("\t%u keep\n", eve->cookie);
      ++i;
    }
  }
}

/** Initialize alarm stack
 */
static
void
voland_stack_init(void)
{
  event_stack = arr_create(event_delete_cb);
}

/* De-initialize alarm stack
 *
 * Note: this must also close the dialog if it exists
 * and remove timers etc
 */
static
void
voland_stack_quit(void)
{
  voland_clear_stack();
  voland_stack_rethink();

  arr_delete(event_stack), event_stack = 0;
}

/* ========================================================================= *
 *
 * TIMED RUN STATE TRACKING
 *
 * ========================================================================= */

/** Timed run state has changed callback
 *
 * Clear alarm stack, remove dialog etc regardless of whether timed
 * has been started of stopped.
 *
 * If timed was started, it will refeed us the alarm stack that
 * should be active.
 *
 * @param running true if timed claimed dbus name, false if the name lost owner
 */
static
void
voland_timed_runstate_cb(bool running)
{
  printf("%s(%s)\n", __FUNCTION__, running ? "true" : "false");
  voland_clear_stack();
  voland_stack_rethink();
}

/* ========================================================================= *
 *
 * HELPERS FOR DBUS MESSAGE PARSING
 *
 * ========================================================================= */

/** Parse timed attribute from dbus message
 *
 * @param rx parse state
 *
 * @return attr_t object, or NULL in case of errors
 */
attr_t *
rx_attr(xdbusrx_t *rx)
{
  attr_t *res = 0;
  const char *key = 0;
  const char *val = 0;

  if( xdbusrx_enter_dict(rx) )
  {
    xdbusrx_read_string(rx, &key);
    xdbusrx_read_string(rx, &val);

    if( xdbusrx_leave_container(rx) )
    {
      res = attr_create(key, val);
    }
  }

  return res;
}

/** Augment stab_t object with array of timed attributes from dbus message
 *
 * @param rx parse state
 *
 * @return true on success, false on failure
 */
bool
rx_stab(xdbusrx_t *rx, stab_t *stab)
{
  bool res = false;
  int  cnt = 0;

  if( xdbusrx_enter_array_ex(rx, &cnt) )
  {
    for( int i = 0; i < cnt; ++i )
    {
      attr_t *attr = rx_attr(rx);
      if( !attr ) goto cleanup;
      stab_add(stab, attr);
    }
    res = xdbusrx_leave_container(rx);
  }
cleanup:
  return res;
}

/** Augment button_t object with data from dbus message
 *
 * @param rx parse state
 *
 * @return true on success, false on failure
 */
bool
rx_button(xdbusrx_t *rx, button_t *btn)
{
  bool res = false;
  if( xdbusrx_enter_struct(rx) )
  {
    rx_stab(rx, btn->stab);
    res = xdbusrx_leave_container(rx);
  }
  return res;
}

/** Augment event_t object with button data from dbus message
 *
 * @param rx parse state
 *
 * @return true on success, false on failure
 */
bool
rx_event_buttons(xdbusrx_t *rx, event_t *eve)
{
  bool res = false;
  int  cnt = 0;
  if( xdbusrx_enter_array_ex(rx, &cnt) )
  {
    for( int i = 0; i < cnt; ++i )
    {
      button_t *btn = button_create();
      arr_push(eve->buttons, btn);

      if( !rx_button(rx, btn) )
      {
        break;
      }
    }

    res = xdbusrx_leave_container(rx);
  }
  return res;
}

/** Augment event_t object with data from dbus message
 *
 * @param rx parse state
 *
 * @return true on success, false on failure
 */
bool
rx_event(xdbusrx_t *rx, event_t *eve)
{
  bool res = false;

  if( xdbusrx_enter_variant(rx) )
  {
    if( xdbusrx_enter_struct(rx) )
    {
      xdbusrx_read_uint32(rx, &eve->cookie);
      xdbusrx_read_uint32(rx, &eve->tick);

      rx_stab(rx, eve->stab);
      rx_event_buttons(rx, eve);

      xdbusrx_leave_container(rx);
    }
    res = xdbusrx_leave_container(rx);
  }

  return res;
}

/* ========================================================================= *
 *
 * DBUS MESSAGE HANDLING
 *
 * ========================================================================= */

/** Handler for com.nokia.voland.open method call
 *
 * @param msg method call request message
 *
 * @return reply message to send back to the caller
 */
static DBusMessage *voland_open_cb(DBusMessage *msg)
{
  DBusMessage *rsp = dbus_message_new_method_return(msg);
  dbus_bool_t  ack = FALSE;

  xdbusrx_t *rx = xdbusrx_create(msg);

  if( xdbusrx_enter_array(rx) )
  {
    while( xdbusrx_todo_p(rx) )
    {
      event_t *eve = event_create();

      if( rx_event(rx, eve) )
      {
        voland_push_to_stack(eve), eve = 0;
      }
      event_delete(eve);
    }
    ack = xdbusrx_leave_container(rx);
  }

  printf("%s() -> %d\n", __FUNCTION__, ack);
  voland_stack_rethink();

  xdbusrx_delete(rx);

  dbus_message_append_args(rsp,
                           DBUS_TYPE_BOOLEAN, &ack,
                           DBUS_TYPE_INVALID);

  return rsp;
}

/** Handler for com.nokia.voland.close method call
 *
 * @param msg method call request message
 *
 * @return reply message to send back to the caller
 */
static DBusMessage *voland_close_cb(DBusMessage *msg)
{
  DBusMessage *rsp = dbus_message_new_method_return(msg);
  dbus_bool_t  ack = FALSE;

  DBusError    err = DBUS_ERROR_INIT;

  dbus_uint32_t cookie = 0;

  if( !dbus_message_get_args(msg, &err,
                            DBUS_TYPE_UINT32, &cookie,
                            DBUS_TYPE_INVALID) )
  {
    fprintf(stderr, "%s: %s: %s\n", "dbus_message_get_args", err.name, err.message);
    goto cleanup;
  }

  voland_remove_from_stack(cookie);
  voland_stack_rethink();

  ack = TRUE;

  cleanup:

  printf("%s(%u) -> %d\n", __FUNCTION__, cookie, ack);

  dbus_message_append_args(rsp,
                           DBUS_TYPE_BOOLEAN, &ack,
                           DBUS_TYPE_INVALID);

  dbus_error_free(&err);

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
voland_filter(DBusConnection *conn,
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
    if( !strcmp(interface, VOLAND_DBUS_INTERFACE) )
    {
      printf("----------------------------------------------------------------\n");
      printf("t:%s, i:%s, m:%s o:%s\n", type_name, interface, member, object);

      if( !strcmp(member, VOLAND_DBUS_METHOD_OPEN) )
      {
        rsp = voland_open_cb(msg);
      }
      else if( !strcmp(member, VOLAND_DBUS_METHOD_CLOSE) )
      {
        rsp = voland_close_cb(msg);
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
 * TEST AUTOMATION HOOKS
 *
 * ========================================================================= */

/** Return pid of notification service
 *
 * @return pid of notification service
 */
int
voland_pid(void)
{
  int pid = getpid();

  printf("%s() -> %d\n", __FUNCTION__, pid);

  return pid;
}

/** Simulate user activity
 *
 * @param cookie unique alarm identifier
 * @param button buttonpress to simulate
 *
 * @return TRUE if activity could be simulated correctly, or FALSE on errors
 */
bool
voland_answer(unsigned cookie, int button)
{
  bool ack = false;

  if( cookie == dialog_cookie )
  {
    ack = xtimed_dialog_response(cookie, button);

    voland_remove_from_stack(cookie);
    voland_stack_rethink();
  }

  printf("%s() -> %d\n", __FUNCTION__, ack);

  return ack;
}

/** Return cookie corresponding to the current alarm dialog
 *
 * @return topmost cookie in the stack, or 0 if stack is empty
 */
unsigned
voland_top(void)
{
  printf("%s() -> %u\n", __FUNCTION__, dialog_cookie);
  return dialog_cookie;
}

/** Toggle test automation mode
 *
 * @param active whether the test automation interface should be in control
 */
void
voland_set_ta_mode(bool active)
{
  printf("%s(%d)\n", __FUNCTION__, active);
  voland_ta_mode = active;
}

/* ========================================================================= *
 *
 * MODULE INIT/QUIT
 *
 * ========================================================================= */

/** D-Bus connection for serving com.nokia.voland on */
static DBusConnection *voland_bus = 0;

/** Initialize com.nokia.voland service
 *
 * @return true if service was succesfully started, or false in case of errors
 */
bool
voland_init(void)
{
  printf("%s()\n", __FUNCTION__);
  bool        res  = false;
  DBusError   err  = DBUS_ERROR_INIT;
#if 0
  DBusBusType type = DBUS_BUS_SESSION;
#else
  DBusBusType type = DBUS_BUS_SYSTEM;
#endif

  voland_stack_init();

  if( !(voland_bus = dbus_bus_get(type, &err)) )
  {
    fprintf(stderr, "%s: %s: %s\n", "dbus_bus_get", err.name, err.message);
    goto cleanup;
  }

  if( !dbus_connection_add_filter(voland_bus, voland_filter, 0, 0) )
  {
    goto cleanup;
  }

  int rc = dbus_bus_request_name(voland_bus, VOLAND_DBUS_SERVICE,
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

  dbus_connection_setup_with_g_main(voland_bus, NULL);

  xtimed_set_runstate_cb(voland_timed_runstate_cb);

  res = true;

cleanup:
  dbus_error_free(&err);
  return res;
}

/** Stop com.nokia.voland service
 */
void
voland_quit(void)
{
  printf("%s()\n", __FUNCTION__);
  if( voland_bus )
  {
    dbus_connection_remove_filter(voland_bus, voland_filter, 0);

    dbus_connection_unref(voland_bus), voland_bus = 0;
  }

  voland_stack_quit();
}
