/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#include "arr.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** Generic array container */
struct arr_t
{
  /** Array of element pointers */
  void **a_elem;

  /** Number of elements in the array */
  size_t a_count;

  /** Number of slots reserved for array elements */
  size_t a_alloc;

  /** Callback for deleteting array elements */
  void (*a_free)(void*);
};

/** Private arr_t constructor
 *
 * @param self pointer to uninitialized arr_t object
 * @param free_cb callback function for deleting items to be added to the array, or NULL
 * if arr_t object does not own the elements
 */
static
void
arr_ctor(arr_t *self, void (*free_cb)(void *))
{
  self->a_elem  = 0;
  self->a_count = 0;
  self->a_alloc = 0;
  self->a_free  = free_cb;
}

/** Private arr_t destructor
 *
 * Deletes all elements still in the array.
 *
 * @param self pointer to initialized arr_t object
 */
static
void
arr_dtor(arr_t *self)
{
  arr_clear(self);
  free(self->a_elem);
}

/** Create an arr_t object
 *
 * Use arr_delete() to release returned objects.
 *
 * @param free_cb callback function for deleting items to be added to the array, or NULL
 * if arr_t object does not own the elements
 */
arr_t *
arr_create(void (*free_cb)(void *))
{
  arr_t *self = calloc(1, sizeof *self);
  arr_ctor(self, free_cb);
  return self;
}

/** Delete arr_t object and all the elements still held in it
 *
 * @param self arr_t object, or NULL for no-op
 */
void
arr_delete(arr_t *self)
{
  if( self != 0 )
  {
    arr_dtor(self);
    free(self);
  }
}

/** Typeless Delete arr_t object callback
 *
 * @param self arr_t object, or NULL for no-op
 */
void
arr_delete_cb(void *self)
{
  arr_delete(self);
}

/** Get the number of elements in arr_t object
 *
 * @param self arr_t object
 *
 * @return number of elements in arr_t object
 */
size_t
arr_count(const arr_t *self)
{
  return self->a_count;
}

/** Delete all elements in arr_t object
 *
 * @param self arr_t object
 */
void
arr_clear(arr_t *self)
{
  if( self->a_free )
  {
    for( size_t i = 0; i < self->a_count; ++i )
    {
      self->a_free(self->a_elem[i]);
    }
  }
  self->a_count = 0;
}

/** Get element from arr_t object by index
 *
 * @param self arr_t object
 * @param i element index
 */
void *
arr_get(arr_t *self, size_t i)
{
  return (i < self->a_count) ? self->a_elem[i] : 0;
}

/** Remove element from arr_t object by index
 *
 * @param self arr_t object
 * @param i element index
 */
void
arr_remove(arr_t *self, size_t i)
{
  if( i < self->a_count )
  {
    if( self->a_free )
    {
      self->a_free(self->a_elem[i]);
    }

    for( --self->a_count; i < self->a_count; ++i )
    {
      self->a_elem[i] = self->a_elem[i+1];
    }
  }
}

/** Add element to arr_t object
 *
 * Note: the elem is owned by arr_t object after adding. Unless
 * the arr_t object was initialized with NULL free_cb.
 *
 * @param self arr_t object
 * @param elem pointer to something
 *
 * @return index of element in the arr_t object
 */
size_t
arr_push(arr_t *self, void *elem)
{
  if( self->a_count == self->a_alloc )
  {
    self->a_alloc += 16;
    self->a_elem = realloc(self->a_elem, self->a_alloc * sizeof *self->a_elem);
  }

  size_t res = self->a_count++;
  self->a_elem[res] = elem;
  return res;
}

/** Remove the last element from arr_t object
 *
 * Note: caller owns the element returned and must release it
 *       appropriate way when it is no longer needed.
 *
 * @param self arr_t object
 *
 * return pointer to something, or NULL if arr_t was empty
 */
void *
arr_pop(arr_t *self)
{
  void *res = 0;
  if( self->a_count > 0 )
  {
    res = self->a_elem[--self->a_count];
  }
  return res;
}

/** Get the last element from arr_t object
 *
 * Note: arr_t still owns the element and it must not be
 *       released by the caller.
 *
 * @param self arr_t object
 *
 * return pointer to something, or NULL if arr_t is empty
 */
void *
arr_tail(arr_t *self)
{
  void *res = 0;
  if( self->a_count > 0 )
  {
    res = self->a_elem[self->a_count - 1];
  }
  return res;
}
