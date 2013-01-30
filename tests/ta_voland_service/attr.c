/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include "attr.h"

#include <string.h>
#include <stdlib.h>

/** attr_t constructor
 *
 * @param self pointer to uninitialized attr_t object
 * @param key  name of the attr_t object
 */
void
attr_ctor(attr_t *self, const char *key)
{
  self->key = strdup(key);
  self->val = 0;
}

/** attr_t destructor
 *
 * @param self pointer to initialized attr_t object
 */
void
attr_dtor(attr_t *self)
{
  free(self->key);
  free(self->val);
}

/** Create an attr_t object
 *
 * Use attr_delete() to release the object.
 *
 * @param key name of the attr_t
 * @param val value of the attr_t
 *
 * @return attr_t object
 */
attr_t *
attr_create(const char *key, const char *val)
{
  attr_t *self = calloc(1, sizeof *self);
  attr_ctor(self, key);
  attr_set(self, val);
  return self;
}

/** Delete an attr_t object
 *
 * @param self attr_t object, or NULL for no-op
 */
void
attr_delete(attr_t *self)
{
  if( self != 0 )
  {
    attr_dtor(self);
    free(self);
  }
}

/** Typeless delete an attr_t object callback
 *
 * @param self attr_t object, or NULL for no-op
 */
void
attr_delete_cb(void *self)
{
  attr_delete(self);
}

/** Set value of an attr_t object
 *
 * @param self attr_t object
 * @param val value of the attr_t, or NULL to unset
 */
void
attr_set(attr_t *self, const char *val)
{
  free(self->val);
  self->val = val ? strdup(val) : 0;
}
