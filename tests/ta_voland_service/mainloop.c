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

/** Mainloop handle, NULL while we're not running within glib mainloop */
static GMainLoop *mainloop_hnd = 0;

/** Value to return upon exit from mainloop */
static int mainloop_result = EXIT_SUCCESS;

/** Make mainloop exit */
void
mainloop_stop(int exit_code)
{
  if( mainloop_result < exit_code )
  {
    mainloop_result = exit_code;
  }

  if( !mainloop_hnd )
  {
    /* Not running within mainloop -> terminate immediately */
    exit(EXIT_FAILURE);
  }
  g_main_loop_quit(mainloop_hnd);
}

/** Run mainloop
 *
 * @return 0 on normal exit, or >0 for abnormal exit
 */
int
mainloop_run(void)
{
  if( !(mainloop_hnd = g_main_loop_new(NULL, FALSE)) )
  {
    goto cleanup;
  }

  g_main_loop_run(mainloop_hnd);

cleanup:

  if( mainloop_hnd != 0 )
  {
    g_main_loop_unref(mainloop_hnd);
    mainloop_hnd = 0;
  }

  return mainloop_result;
}
