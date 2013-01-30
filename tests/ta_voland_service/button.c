/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include "button.h"
#include <stdlib.h>

/** button_t constructor
 *
 * @param self pointer to uninitialized button_t object
 */
void
button_ctor(button_t *self)
{
  self->stab = stab_create();
}

/** button_t destructor
 *
 * @param self pointer to initialized button_t object
 */
void
button_dtor(button_t *self)
{
  stab_delete(self->stab);
}

/** Create a button_t object
 *
 * Use button_delete() to release the object.
 *
 * @return button_t object
 */
button_t *
button_create(void)
{
  button_t *self = calloc(1, sizeof *self);
  button_ctor(self);
  return self;
}

/** Delete a button_t object
 *
 * @param self button_t object, or NULL for no-op
 */
void
button_delete(button_t *self)
{
  if( self != 0 )
  {
    button_dtor(self);
    free(self);
  }
}

/** Typelses delete a button_t object callback
 *
 * @param self button_t object, or NULL for no-op
 */
void
button_delete_cb(void *self)
{
  button_delete(self);
}
