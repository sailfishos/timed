/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include "event.h"

#include <stdlib.h>
#include <stdio.h>

/** event_t constructor
 *
 * @param self pointer to uninitialized event_t object
 */
void
event_ctor(event_t *self)
{
  self->cookie  = 0;
  self->tick    = 0;
  self->stab    = stab_create();
  self->buttons = arr_create(button_delete_cb);
}

/** event_t destructor
 *
 * @param self pointer to initialized event_t object
 */
void
event_dtor(event_t *self)
{
  stab_delete(self->stab);
  arr_delete(self->buttons);
}

/** Create an event_t object
 *
 * Release object via event_delete()
 *
 * @return event_t object
 */
event_t *
event_create(void)
{
  event_t *self = calloc(1, sizeof *self);
  event_ctor(self);
  return self;
}

/** Delete an event_t object
 *
 * @param self event_t object, or NULL for no-op
 */
void
event_delete(event_t *self)
{
  if( self != 0 )
  {
    event_dtor(self);
    free(self);
  }
}

/** Typeless delete an event_t object callback
 *
 * @param self event_t object, or NULL for no-op
 */
void
event_delete_cb(void *self)
{
  event_delete(self);
}

/** Dump event to stdout for debugging purposes
 *
 * @param self event_t object
 */

void
event_dump(event_t *self)
{
  printf("eve.cookie = %u\n", self->cookie);
  printf("eve.tick   = %u\n", self->tick);

  for( size_t i = 0; i < stab_count(self->stab); ++i )
  {
    attr_t *attr = stab_get(self->stab, i);
    printf("eve.att[%zd]: %s=%s\n", i, attr->key, attr->val);
  }

  for( size_t k = 0; k < arr_count(self->buttons); ++k )
  {
    button_t *btn = arr_get(self->buttons, k);
    printf("eve.btn[%zd]\n", k);
    for( size_t i=0; i < stab_count(btn->stab); ++i )
    {
      attr_t *attr = stab_get(btn->stab, i);
      printf("eve.btn[%zd].att[%zd]: %s=%s\n", k, i, attr->key, attr->val);
    }
  }
}
