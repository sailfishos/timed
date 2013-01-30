/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include "stab.h"
#include "arr.h"

#include <stdlib.h>

/* ------------------------------------------------------------------------- *
 * stab_t
 * ------------------------------------------------------------------------- */

/** Symbol table object */
struct stab_t
{
  /** Array of attributes */
  arr_t *s_attr_arr; // get(i) -> attr_t *
};

/** Private stab_t constructor
 *
 * @param self pointer to uninitialized stab_t object
 */
static
void
stab_ctor(stab_t *self)
{
  self->s_attr_arr = arr_create(attr_delete_cb);
}

/** Private stab_t destructor
 *
 * @param self pointer to initialized stab_t object
 */
static
void
stab_dtor(stab_t *self)
{
  arr_delete(self->s_attr_arr);
}

/** Create a new stab_t object
 *
 * Use stab_delete() to release the object.
 *
 * @return stab_t object
 */
stab_t *
stab_create(void)
{
  stab_t *self = calloc(1, sizeof *self);
  stab_ctor(self);
  return self;
}

/** Delete a stab_t object
 *
 * @param stab_t object, or NULL for no-op
 */
void
stab_delete(stab_t *self)
{
  if( self != 0 )
  {
    stab_dtor(self);
    free(self);
  }
}
/** Typeless delete a stab_t object callback
 *
 * @param stab_t object, or NULL for no-op
 */
void
stab_delete_cb(void *self)
{
  stab_delete(self);
}

/** Get number of elements in stab_t object
 *
 * @param stab_t object
 *
 * @return number of elements in stab_t object
 */
size_t
stab_count(const stab_t *self)
{
  return arr_count(self->s_attr_arr);
}

/** Get stab_t object element by index
 *
 * @param stab_t object
 * @param i index
 *
 * @return attr_t object, or NULL
 */

attr_t *
stab_get(const stab_t *self, size_t i)
{
  return arr_get(self->s_attr_arr, i);
}

/** Add attr_t object to stab_t object
 *
 * @param stab_t object
 * @param attr_t object,
 */
void
stab_add(stab_t *self, attr_t *attr)
{
  arr_push(self->s_attr_arr, attr);
}
