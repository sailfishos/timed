/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include <stdlib.h>

#include "voland.h"
#include "ta_voland.h"
#include "xtimed.h"
#include "mainloop.h"

/** Entry point for test automation alarmui program
 *
 * @param ac number of command line arguments
 * @param av array of command line argument strings
 *
 * @returns 0 on success, or non-zero value on error
 */
int
main(int ac, char **av)
{
  int res = EXIT_FAILURE;

  if( !xtimed_init() )
  {
    goto cleanup;
  }

  if( !voland_init() )
  {
    goto cleanup;
  }

  if( !ta_voland_init() )
  {
    goto cleanup;
  }

  res = mainloop_run();

  cleanup:

  ta_voland_quit();
  voland_quit();
  xtimed_quit();

  return res;
}
