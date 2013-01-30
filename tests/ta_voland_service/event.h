/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef EVENT_H_
# define EVENT_H_

# include "stab.h"
# include "arr.h"
# include "button.h"

# include <dbus/dbus.h>

# ifdef __cplusplus
extern "C" {
# elif 0
} /* fool JED indentation ... */
# endif

/* ------------------------------------------------------------------------- *
 * event_t
 * ------------------------------------------------------------------------- */

typedef struct event_t event_t;

/** Timed event data */
struct event_t
{
  /** Unique alarm identifier */
  dbus_uint32_t  cookie;

  /** Something timed sends us */
  dbus_uint32_t  tick;

  /** Array of event attributes */
  stab_t        *stab;

  /** Array of buttons */
  arr_t         *buttons; // get(i) -> button_t *
};

void     event_ctor(event_t *self);
void     event_dtor(event_t *self);

event_t *event_create(void);
void     event_delete(event_t *self);
void     event_delete_cb(void *self);

void     event_dump(event_t *self);

# ifdef __cplusplus
};
# endif

#endif /* EVENT_H_ */
