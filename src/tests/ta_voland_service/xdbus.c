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
#include <dbus/dbus-glib-lowlevel.h>

#include "xdbus.h"

/* ------------------------------------------------------------------------- *
 * misc
 * ------------------------------------------------------------------------- */

/** Helper for getting dbus data type as string
 *
 * @param type dbus data type (DBUS_TYPE_BOOLEAN etc)
 *
 * @return type name with out common prefix (BOOLEAN etc)
 */
static const char *xdbus_get_type_name(int type)
{
  const char *res = "UNKNOWN";
  switch( type )
  {
  case DBUS_TYPE_INVALID:     res = "INVALID";     break;
  case DBUS_TYPE_BYTE:        res = "BYTE";        break;
  case DBUS_TYPE_BOOLEAN:     res = "BOOLEAN";     break;
  case DBUS_TYPE_INT16:       res = "INT16";       break;
  case DBUS_TYPE_UINT16:      res = "UINT16";      break;
  case DBUS_TYPE_INT32:       res = "INT32";       break;
  case DBUS_TYPE_UINT32:      res = "UINT32";      break;
  case DBUS_TYPE_INT64:       res = "INT64";       break;
  case DBUS_TYPE_UINT64:      res = "UINT64";      break;
  case DBUS_TYPE_DOUBLE:      res = "DOUBLE";      break;
  case DBUS_TYPE_STRING:      res = "STRING";      break;
  case DBUS_TYPE_OBJECT_PATH: res = "OBJECT_PATH"; break;
  case DBUS_TYPE_SIGNATURE:   res = "SIGNATURE";   break;
  case DBUS_TYPE_UNIX_FD:     res = "UNIX_FD";     break;
  case DBUS_TYPE_ARRAY:       res = "ARRAY";       break;
  case DBUS_TYPE_VARIANT:     res = "VARIANT";     break;
  case DBUS_TYPE_STRUCT:      res = "STRUCT";      break;
  case DBUS_TYPE_DICT_ENTRY:  res = "DICT_ENTRY";  break;
  default: break;
  }
  return res;
}

/** Clone an array of strings
 *
 * Use xdbus_free_string_array() to release the returned array.
 *
 * @param v NULL terminated array of strings, or NULL
 * @return duplicate of the array, or NULL if NULL array was passed
 */
char **
xdbus_dup_string_array(const char **v)
{
  char **r = 0;

  if( v != 0 )
  {
    size_t n = 0;
    while( v[n] ) ++n;

    r = calloc(n+1, sizeof *r);

    for( size_t i = 0; i < n; ++i )
    {
      r[i] = strdup(v[i]);
    }
    r[n] = 0;
  }
  return r;
}

/** Release an array of strings
 *
 * @param v NULL terminated array of strings, or NULL for no-op
 */
void
xdbus_free_string_array(char **v)
{
  if( v != 0 )
  {
    for( size_t i = 0; v[i]; ++i )
    {
      free(v[i]);
    }
    free(v);
  }
}

/* ------------------------------------------------------------------------- *
 * xdbusrx_t
 * ------------------------------------------------------------------------- */

/** Read iterator for parsing D-Bus messages */
struct xdbusrx_t
{
  /** Parse error. Once set, all further operations will fail.*/
  DBusError err;

  /** Dbus iterator stack */
  DBusMessageIter iter[DBUS_MAXIMUM_TYPE_RECURSION_DEPTH];

  /** Stack pointer */
  int sp;

  /* Initialized succesfully to readable state flag */
  int rd;
};

/** Get current parse context
 *
 * @param self xdbusrx_t object pointer
 *
 * @return currently active DBusMessageIter pointer
 */
static
DBusMessageIter *
xdbusrx_now(xdbusrx_t *self)
{
  return &self->iter[self->sp];
}

/** Have we already bumped into parsing error
 *
 * @param self xdbusrx_t object pointer
 *
 * @return true if error is set, false otherwise
 */
gboolean
xdbusrx_error_p(const xdbusrx_t *self)
{
  return dbus_error_is_set(&self->err);
}

/** Is the read iterator in readable state
 *
 * @param self xdbusrx_t object pointer
 *
 * @return true if iterator is readable, false otherwise
 */
gboolean
xdbusrx_read_p(xdbusrx_t *self)
{
  if( xdbusrx_error_p(self) )
  {
    return FALSE;
  }
  if( dbus_error_is_set(&self->err) )
  {
    return FALSE;
  }
  return TRUE;
}

/** Are there more paramaters to read from the iterator
 *
 * @param self xdbusrx_t object pointer
 *
 * @return true if current container is not at DBUS_TYPE_INVALID
 */

gboolean
xdbusrx_todo_p(xdbusrx_t *self)
{
  if( !xdbusrx_read_p(self) )
  {
    return FALSE;
  }

  int type = dbus_message_iter_get_arg_type(xdbusrx_now(self));
  return type != DBUS_TYPE_INVALID;
}

/** Clear error state of read iterator to allow further processing
 *
 * @param self xdbusrx_t object pointer
 */
void
xdbusrx_clear_error(xdbusrx_t *self)
{
  if( dbus_error_is_set(&self->err) )
  {
    dbus_error_free(&self->err);
  }
}

/** Construct a xdbusrx_t object
 *
 * @param self xdbusrx_t object pointer
 */
static
void
xdbusrx_ctor(xdbusrx_t *self)
{
  memset(self, 0, sizeof self);
  self->sp = 0;
  self->rd = 0;
  dbus_error_init(&self->err);
}

/** Destroy a xdbusrx_t object
 *
 * NOTE: does implicit error logging if error state has not been
 *       cleared prior to destroying the object
 *
 * @param self xdbusrx_t object pointer
 */
static
void
xdbusrx_dtor(xdbusrx_t *self)
{
  if( dbus_error_is_set(&self->err) )
  {
    fprintf(stderr, "%s: %s\n",
            self->err.name,
            self->err.message);
    dbus_error_free(&self->err);
  }
}

/** Create a new read iterator for dbus message
 *
 * NOTE: while this function will always return non-null parse
 *       object, it might be in error state if initialization fails.
 *
 * The returned object must be released via xdbusrx_delete().
 *
 * @param msg pointer to dbus message object
 *
 * @return xdbusrx_t object pointer
 */
xdbusrx_t *
xdbusrx_create(DBusMessage *msg)
{
  xdbusrx_t *self = calloc(1, sizeof *self);
  xdbusrx_ctor(self);

  if( !dbus_message_iter_init(msg, xdbusrx_now(self)) )
  {
    fprintf(stderr, "failed to initialize dbus read iterator\n");
  }
  else
  {
    self->rd = 1;
  }
  return self;
}

/** Delete a read iterator object created via xdbusrx_create()
 *
 * @param self xdbusrx_t object pointer, or NULL for no-op
 */
void
xdbusrx_delete(xdbusrx_t *self)
{
  if( self != 0 )
  {
    xdbusrx_dtor(self);
    free(self);
  }
}

/** Typeless delete function for callback usage
 *
 * @param self xdbusrx_t object pointer, or NULL for no-op
 */
void
xdbusrx_delete_cb(void *self)
{
  xdbusrx_delete(self);
}

/** Signature of current dbus element
 *
 * @param self xdbusrx_t object pointer
 *
 * @return signature string for current parse context
 */
const char *
xdbusrx_signature(xdbusrx_t *self)
{
  if( !xdbusrx_read_p(self) )
  {
    return NULL;
  }

  return dbus_message_iter_get_signature(xdbusrx_now(self));
}

/** Type of the next parameter to be parsed
 *
 * @param self xdbusrx_t object pointer
 *
 * @return dbus type identifier, or DBUS_TYPE_INVALID in case
 *         of errors / end of container reached
 */
static
int
xdbusrx_at(xdbusrx_t *self)
{
  if( !xdbusrx_read_p(self) )
  {
    return DBUS_TYPE_INVALID;
  }

  return dbus_message_iter_get_arg_type(xdbusrx_now(self));
}

/** Helper for testing that iterator points to expected data type
 *
 * @param self xdbusrx_t object pointer
 * @param want_type D-Bus data type
 *
 * @return TRUE if iterator points to give data type, FALSE otherwise
 */
static
gboolean
xdbusrx_require_type(xdbusrx_t *self,
                     int want_type)
{
  if( !xdbusrx_read_p(self) )
  {
    return FALSE;
  }

  int have_type = dbus_message_iter_get_arg_type(xdbusrx_now(self));

  if( want_type != have_type )
  {
    dbus_set_error(&self->err, DBUS_ERROR_INVALID_ARGS,
                   "expected %s, got %s\n",
                   xdbus_get_type_name(want_type),
                   xdbus_get_type_name(have_type));
    return FALSE;
  }

  return TRUE;
}

/** Internally used helper for parsing basic types
 *
 * @param self xdbusrx_t object pointer
 * @param type dbus type identifier to parse
 * @param addr where to store the parameter
 *
 * @return TRUE if required type was parsed, or FALSE in case of errors
 */
static
gboolean
xdbusrx_read_basic_private_(xdbusrx_t *self,
                            int type, void *addr)
{
  if( !xdbusrx_require_type(self, type) )
  {
    return FALSE;
  }

  dbus_message_iter_get_basic(xdbusrx_now(self), addr);
  dbus_message_iter_next(xdbusrx_now(self));

  return TRUE;
}

/** Get dbus_int16_t parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_int16(xdbusrx_t *self, dbus_int16_t *data)
{
  int type = DBUS_TYPE_INT16;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get dbus_int32_t parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_int32(xdbusrx_t *self, dbus_int32_t *data)
{
  int type = DBUS_TYPE_INT32;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get dbus_int64_t parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_int64(xdbusrx_t *self, dbus_int64_t *data)
{
  int type = DBUS_TYPE_INT64;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get dbus_uint16_t parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_uint16(xdbusrx_t *self, dbus_uint16_t *data)
{
  int type = DBUS_TYPE_UINT16;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get dbus_uint32_t parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_uint32(xdbusrx_t *self, dbus_uint32_t *data)
{
  int type = DBUS_TYPE_UINT32;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get dbus_uint64_t parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_uint64(xdbusrx_t *self, dbus_uint64_t *data)
{
  int type = DBUS_TYPE_UINT64;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get octet parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_byte(xdbusrx_t *self, unsigned char *data)
{
  int type = DBUS_TYPE_BYTE;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get dbus_bool_t parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_bool(xdbusrx_t *self, dbus_bool_t *data)
{
  int type = DBUS_TYPE_BOOLEAN;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get double parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_double(xdbusrx_t *self, double *data)
{
  int type = DBUS_TYPE_DOUBLE;
  return xdbusrx_read_basic_private_(self, type, data);
}

/** Get int parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_int(xdbusrx_t *self, gint *value)
{
  dbus_int32_t data = 0;
  int          type = DBUS_TYPE_INT32;

  if( !xdbusrx_read_basic_private_(self, type, &data) )
  {
    return FALSE;
  }

  return *value = data, TRUE;
}

/** Get gboolean parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_boolean(xdbusrx_t *self, gboolean *value)
{
  dbus_int32_t data = 0;
  int          type = DBUS_TYPE_BOOLEAN;

  if( !xdbusrx_read_basic_private_(self, type, &data) )
  {
    return FALSE;
  }

  return *value = data, TRUE;
}

/** Get const string parameter from read iterator
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the value
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_read_string(xdbusrx_t *self, const char **value)
{
  char *data = 0;
  int   type = DBUS_TYPE_STRING;

  if( !xdbusrx_read_basic_private_(self, type, &data) )
  {
    return FALSE;
  }

  return *value = data, TRUE;
}

/** Return parse context to parent container
 *
 * @param self xdbusrx_t object pointer
 *
 * @return TRUE on success, or FALSE in case of errors
 */

gboolean
xdbusrx_leave_container(xdbusrx_t *self)
{
  if( !xdbusrx_read_p(self) )
  {
    return FALSE;
  }

  if( self->sp < 1 )
  {
    dbus_set_error(&self->err, DBUS_ERROR_FAILED,
                   "read stack underflow");
    return FALSE;
  }

  self->sp -= 1;
  return TRUE;
}

/** Internal helper for entering child container
 *
 * @param self xdbusrx_t object pointer
 * @param type dbus type identifier of container
 *
 * @return TRUE on success, or FALSE in case of errors
 */
static
gboolean
xdbusrx_enter_private(xdbusrx_t *self, int type)
{
  if( !xdbusrx_read_p(self) )
  {
    return FALSE;
  }

  if( !xdbusrx_require_type(self, type) )
  {
    return FALSE;
  }

  if( self->sp + 1 >= DBUS_MAXIMUM_TYPE_RECURSION_DEPTH )
  {
    dbus_set_error(&self->err, DBUS_ERROR_FAILED,
                   "read stack overflow");
    return FALSE;
  }

  DBusMessageIter *iter, *sub;

  iter = &self->iter[self->sp++];
  sub  = &self->iter[self->sp];
  dbus_message_iter_recurse(iter, sub);
  dbus_message_iter_next(iter);

  return TRUE;
}

/** Enter child container of variant type
 *
 * @param self xdbusrx_t object pointer
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_enter_variant(xdbusrx_t *self)
{
  return xdbusrx_enter_private(self, DBUS_TYPE_VARIANT);
}

/** Enter child container of struct type
 *
 * @param self xdbusrx_t object pointer
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_enter_struct(xdbusrx_t *self)
{
  return xdbusrx_enter_private(self, DBUS_TYPE_STRUCT);
}

/** Enter child container of dict type
 *
 * @param self xdbusrx_t object pointer
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_enter_dict(xdbusrx_t *self)
{
  return xdbusrx_enter_private(self, DBUS_TYPE_DICT_ENTRY);
}

/** Enter child container of array type
 *
 * @param self xdbusrx_t object pointer
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_enter_array(xdbusrx_t *self)
{
  return xdbusrx_enter_private(self, DBUS_TYPE_ARRAY);
}

/** Enter child container of array type and get size of array
 *
 * @param self xdbusrx_t object pointer
 * @param pcount where to store the number of elements in the array
 *
 * @return TRUE on success, or FALSE in case of errors
 */
gboolean
xdbusrx_enter_array_ex(xdbusrx_t *self, int *pcount)
{
  if( !xdbusrx_read_p(self) )
  {
    return FALSE;
  }

  if( !xdbusrx_require_type(self, DBUS_TYPE_ARRAY) )
  {
    return FALSE;
  }

  if( self->sp + 1 >= DBUS_MAXIMUM_TYPE_RECURSION_DEPTH )
  {
    dbus_set_error(&self->err, DBUS_ERROR_FAILED,
                   "read stack overflow");
    return FALSE;
  }

  DBusMessageIter *iter, *sub;

  iter = &self->iter[self->sp++];
  sub  = &self->iter[self->sp];
  dbus_message_iter_recurse(iter, sub);

  if( pcount )
  {
    int count = 0;

    while( dbus_message_iter_get_arg_type(sub) != DBUS_TYPE_INVALID )
    {
      ++count;
      dbus_message_iter_next(sub);
    }
    *pcount = count;
    dbus_message_iter_recurse(iter, sub);
  }

  dbus_message_iter_next(iter);

  return TRUE;

}

/** Internally used helper for parsing arrays of basic types
 *
 * @param self xdbusrx_t object pointer
 * @param type dbus type identifier to parse
 * @param data where to store the array pointer
 * @param count where to store the number of elements in the array
 *
 * @return TRUE if required type was parsed, or FALSE in case of errors
 */
static
gboolean
xdbusrx_read_basic_array_private_(xdbusrx_t *self,
                                  int type,
                                  void *data,
                                  int *count)
{
  if( !xdbusrx_require_type(self, DBUS_TYPE_ARRAY) )
  {
    return FALSE;
  }

  int have = dbus_message_iter_get_element_type(xdbusrx_now(self));

  if( have != type )
  {
    dbus_set_error(&self->err, DBUS_ERROR_INVALID_ARGS,
                   "expected array of %s, got array of %s\n",
                   xdbus_get_type_name(type),
                   xdbus_get_type_name(have));
    return FALSE;
  }

  if( !xdbusrx_enter_array(self) )
  {
    return FALSE;
  }

  dbus_message_iter_get_fixed_array(xdbusrx_now(self),
                                    data, count);

  xdbusrx_leave_container(self);
  return TRUE;
}

/** Get array of int type values
 *
 * NOTE: The resulting array is borrowed from underlying dbus message
 *       object and must not be freed.
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the array pointer
 * @param count where to store the number of elements in the array
 *
 * @return TRUE if required type was parsed, or FALSE in case of errors
 */
gboolean
xdbusrx_read_int_array(xdbusrx_t *self,
                       int **data,
                       int *count)
{
  return xdbusrx_read_basic_array_private_(self, DBUS_TYPE_INT32, data, count);
}

/** Get array of string type values
 *
 * NOTE: The resulting array pointer itself must be released with free().
 *       The elements within array are borrowed from underlying dbus
 *       message and must not be freed.
 *
 * @param self xdbusrx_t object pointer
 * @param data where to store the array pointer
 * @param count where to store the number of elements in the array
 *
 * @return TRUE if required type was parsed, or FALSE in case of errors
 */
gboolean
xdbusrx_read_string_array(xdbusrx_t *self,
                          const char ***addr,
                          int *count)
{
  gboolean res = FALSE;

  int n = 0;
  int m = 256;
  const char **v = 0;

  if( !xdbusrx_enter_array(self) )
  {
    goto bailout;
  }

  v = malloc(m * sizeof *v);

  while( xdbusrx_todo_p(self) )
  {
    if( n == m )
    {
      v = realloc(v, (m *= 2) * sizeof *v);
    }
    if( !xdbusrx_read_string(self, &v[n]) )
    {
      goto cleanup;
    }
    n += 1;
  }

  v = realloc(v, (n+1) * sizeof *v);
  v[n] = 0;

  *count = n, *addr = v, v = 0;

  res = TRUE;

cleanup:
  xdbusrx_leave_container(self);

bailout:
  free(v);

  return res;
}

/* ------------------------------------------------------------------------- *
 * xdbustx_t
 * ------------------------------------------------------------------------- */

/** Write iterator for constructing D-Bus messages */
struct xdbustx_t
{
  /** Write error. Once set, all further operations will fail.*/
  DBusError err;

  /** Dbus iterator stack */
  DBusMessageIter iter[DBUS_MAXIMUM_TYPE_RECURSION_DEPTH];

  /** Stack pointer */
  int sp;

  /* Initialized succesfully to writable state flag */
  int wr;
};

/** Clear error state of write iterator to allow further processing
 *
 * NOTE: Usually this will make no sense, once message construction
 *       fails it needs to be started from scratch or abandoned
 *       altogether.
 *
 * @param self xdbusrx_t object pointer
 */
void
xdbustx_clear_error(xdbustx_t *self)
{
  if( dbus_error_is_set(&self->err) )
  {
    dbus_error_free(&self->err);
  }
}

/** Construct a xdbustx_t object
 *
 * @param self xdbustx_t object pointer
 */

static
void
xdbustx_ctor(xdbustx_t *self)
{
  memset(self, 0, sizeof self);
  self->sp = 0;
  self->wr = 0;
  dbus_error_init(&self->err);
}

/** Destroy a xdbustx_t object
 *
 * NOTE: does implicit error logging if error state has not been
 *       cleared prior to destroying the object
 *
 * Also abandons any containers left open due to errors.
 *
 * @param self xdbustx_t object pointer
 */
static
void
xdbustx_dtor(xdbustx_t *self)
{
  if( dbus_error_is_set(&self->err) )
  {
    fprintf(stderr, "%s: %s\n",
            self->err.name,
            self->err.message);
    dbus_error_free(&self->err);
  }

  if( self->wr && self->sp > 0 )
  {
    while( self->sp-- > 0 )
    {
      dbus_message_iter_abandon_container(&self->iter[self->sp + 0],
                                          &self->iter[self->sp + 1]);
    }
  }
}

/** Get currently active DBusMessageIter from write iterator
 *
 * @param self xdbustx_t object pointer
 *
 * @return iterator for container that is currently processed
 */
static
DBusMessageIter *
xdbustx_now(xdbustx_t *self)
{
  return &self->iter[self->sp];
}

/** Have we already bumped into append error
 *
 * @param self xdbustx_t object pointer
 *
 * @return TRUE if error state has been set, FALSE othewise
 */
gboolean
xdbustx_error_p(const xdbustx_t *self)
{
  return dbus_error_is_set(&self->err);
}

/** Is the write iterator in writable state
 *
 * @param self xdbustx_t object pointer
 *
 * @return TRUE if error state has been set, FALSE othewise
 */
gboolean
xdbustx_write_p(xdbustx_t *self)
{
  if( xdbustx_error_p(self) )
  {
    return FALSE;
  }
  if( dbus_error_is_set(&self->err) )
  {
    return FALSE;
  }
  return TRUE;
}

/** Create a new append iterator for dbus message
 *
 * NOTE: while this function will always return non-null parse
 *       object, it might be in error state if initialization fails.
 *
 * The returned object must be released via xdbustx_delete().
 *
 * @param msg pointer to dbus message object
 *
 * @return xdbustx_t object pointer
 */
xdbustx_t *
xdbustx_create(DBusMessage *msg)
{
  xdbustx_t *self = calloc(1, sizeof *self);
  xdbustx_ctor(self);

  dbus_message_iter_init_append(msg, xdbustx_now(self));
  // Nice, we must expect read iterators that can fail, but
  // append iterators that do not?

  self->wr = 1;

  return self;
}

/** Delete a write iterator object created via xdbustx_create()
 *
 * @param self xdbustx_t object pointer, or NULL for no-op
 */
void
xdbustx_delete(xdbustx_t *self)
{
  if( self != 0 )
  {
    xdbustx_dtor(self);
    free(self);
  }
}

/** Typeless delete function for callback usage
 *
 * @param self xdbustx_t object pointer, or NULL for no-op
 */
void
xdbustx_delete_cb(void *self)
{
  xdbustx_delete(self);
}

/** Internal helper for adding basic type data to message
 *
 * @param self xdbustx_t object pointer
 * @param type dbus type identifier
 * @param data pointer to value to add
 *
 * @return TRUE on success, or FALSE in case of errors
 */
static
gboolean
xdbustx_write_basic_private(xdbustx_t *self,
                            int type, const void *data)
{
  if( !xdbustx_write_p(self) )
  {
    return FALSE;
  }

  if( !dbus_message_iter_append_basic(xdbustx_now(self), type, data) )
  {
    fprintf(stderr, "failed to add %s data\n", xdbus_get_type_name(type));
    return FALSE;
  }

  return TRUE;
}

/** Append gint value to D-Bus message
 *
 * @param self xdbustx_t object pointer
 * @param value the value to add
 *
 * @return TRUE on success, FALSE on failure
 */
gboolean
xdbustx_write_int(xdbustx_t *self, gint value)
{
  dbus_int32_t data = value;
  int          type = DBUS_TYPE_INT32;

  return xdbustx_write_basic_private(self, type, &data);
}

/** Append gboolean value to D-Bus message
 *
 * @param self xdbustx_t object pointer
 * @param value the value to add
 *
 * @return TRUE on success, FALSE on failure
 */
gboolean
xdbustx_write_boolean(xdbustx_t *self, gboolean value)
{
  dbus_bool_t data = value;
  int         type = DBUS_TYPE_BOOLEAN;

  return xdbustx_write_basic_private(self, type, &data);
}

/** Append object path value to D-Bus message
 *
 * @param self xdbustx_t object pointer
 * @param value the value to add
 *
 * @return TRUE on success, FALSE on failure
 */
gboolean
xdbustx_write_path(xdbustx_t *self, const gchar *value)
{
  const char *data = value;
  int         type = DBUS_TYPE_OBJECT_PATH;

  return xdbustx_write_basic_private(self, type, &data);
}

/** Append string value to D-Bus message
 *
 * @param self xdbustx_t object pointer
 * @param value the value to add
 *
 * @return TRUE on success, FALSE on failure
 */
gboolean
xdbustx_write_string(xdbustx_t *self, const gchar *value)
{
  const char *data = value;
  int         type = DBUS_TYPE_STRING;

  return xdbustx_write_basic_private(self, type, &data);
}

/** Append variant container to D-Bus message
 *
 * Note: xdbustx_pop_container() must be called once the
 *       variant content has been added.
 *
 * @param self xdbustx_t object pointer
 * @param signature signature string of the data that will be added
 *                  to the variant container
 *
 * @return TRUE on success, FALSE on failure
 */

gboolean
xdbustx_push_variant(xdbustx_t *self,
                     const char *signature)
{
  if( !xdbustx_write_p(self) )
  {
    return FALSE;
  }

  if( self->sp + 1 >= DBUS_MAXIMUM_TYPE_RECURSION_DEPTH )
  {
    dbus_set_error(&self->err, DBUS_ERROR_FAILED,
                   "write stack overflow");
    return FALSE;
  }

  DBusMessageIter *iter, *sub;

  iter = &self->iter[self->sp + 0];
  sub  = &self->iter[self->sp + 1];

  if( !dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                        signature, sub) )
  {
    dbus_set_error(&self->err, DBUS_ERROR_FAILED,
                   "failed to initialize variant write iterator");
    return FALSE;
  }

  self->sp += 1;
  return TRUE;
}

/** Finalize container object in D-Bus message
 *
 * @param self xdbustx_t object pointer
 *
 * @return TRUE on success, FALSE on failure
 */
gboolean
xdbustx_pop_container(xdbustx_t *self)
{
  if( !xdbustx_write_p(self) )
  {
    return FALSE;
  }

  if( self->sp < 1 )
  {
    dbus_set_error(&self->err, DBUS_ERROR_FAILED,
                   "write stack underflow");
    return FALSE;
  }

  DBusMessageIter *iter, *sub;

  sub  = &self->iter[self->sp--];
  iter = &self->iter[self->sp];

  if( !dbus_message_iter_close_container(iter, sub) )
  {
    dbus_set_error(&self->err, DBUS_ERROR_FAILED,
                   "failed to close container");
    return FALSE;
  }

  return TRUE;
}

/* ------------------------------------------------------------------------- *
 * misc
 * ------------------------------------------------------------------------- */

/** Helper for making blocking D-Bus method calls
 *
 * @param con D-Bus connection to communicate over
 * @param req D-Bus method call message to send
 *
 * @return D-Bus method reply message, or NULL on failure
 */
DBusMessage *
xdbus_call_method(DBusConnection *con, DBusMessage *req)
{
  DBusMessage *rsp = 0;
  DBusError    err = DBUS_ERROR_INIT;

  rsp = dbus_connection_send_with_reply_and_block(con, req, -1, &err);

  if( !rsp )
  {
    fprintf(stderr, "%s.%s: %s: %s\n",
            dbus_message_get_interface(req),
            dbus_message_get_member(req),
            err.name, err.message);
    goto EXIT;
  }

EXIT:
  dbus_error_free(&err);

  return rsp;
}

/** Get list of D-Bus names currently registered on a connection
 *
 * Use xdbus_free_string_array() to release the returned array.
 *
 * @param con D-Bus connection
 *
 * @return array of name strings, or NULL in case of errors
 */
char **busListNames(DBusConnection *con)
{
  char       **res = 0;
  DBusMessage *req = 0;
  DBusMessage *rsp = 0;
  xdbusrx_t    *rx  = 0;

  const char **v = 0;
  int          n = 0;

  req = dbus_message_new_method_call(DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS,
                                     DBUS_INTERFACE_DBUS,
                                     "ListNames");

  if( !req ) goto cleanup;

  if( !(rsp = xdbus_call_method(con, req)) )
  {
    goto cleanup;
  }

  rx = xdbusrx_create(rsp);

  if( !xdbusrx_read_string_array(rx, &v, &n) )
  {
    goto cleanup;
  }

  res = xdbus_dup_string_array(v);

cleanup:

  free(v);

  xdbusrx_delete(rx);

  if( rsp ) dbus_message_unref(rsp);
  if( req ) dbus_message_unref(req);

  return res;
}

/** Get the (private) name of the owner of a (well known) dbus name
 *
 * @param con D-Bus connection
 * @param name name to query
 *
 * @return name of the owner, or NULL in case of errors
 */
char *busGetNameOwner(DBusConnection *con, const char *name)
{
  char        *res = 0;
  DBusMessage *req = 0;
  DBusMessage *rsp = 0;
  xdbustx_t  *tx  = 0;
  xdbusrx_t  *rx  = 0;

  const char *data = 0;

  req = dbus_message_new_method_call(DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS,
                                     DBUS_INTERFACE_DBUS,
                                     "GetNameOwner");

  if( !req ) goto cleanup;

  tx = xdbustx_create(req);

  if( !xdbustx_write_string(tx, name) )
  {
    goto cleanup;
  }

  if( !(rsp = xdbus_call_method(con, req)) )
  {
    goto cleanup;
  }

  rx = xdbusrx_create(rsp);

  if( !xdbusrx_read_string(rx, &data) )
  {
    goto cleanup;
  }

  res = strdup(data);

cleanup:

  xdbusrx_delete(rx);
  xdbustx_delete(tx);

  if( rsp ) dbus_message_unref(rsp);
  if( req ) dbus_message_unref(req);

  return res;
}

/** Construct a GetNameOwner method call message
 *
 * @param name dbus name to query
 *
 * @return dbus method call message, or NULL in case of errors
 */
DBusMessage *busGetNameOwner_req(const char *name)
{
  DBusMessage *req = 0;

  req = dbus_message_new_method_call(DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS,
                                     DBUS_INTERFACE_DBUS,
                                     "GetNameOwner");

  if( req &&  !dbus_message_append_args(req,
                                        DBUS_TYPE_STRING, &name,
                                        DBUS_TYPE_INVALID) )
  {
    dbus_message_unref(req), req = 0;
  }

  return req;
}

/** Placeholder for any basic dbus data type */
typedef union
{
  dbus_int16_t i16;
  dbus_int32_t i32;
  dbus_int64_t i64;

  dbus_uint16_t u16;
  dbus_uint32_t u32;
  dbus_uint64_t u64;

  dbus_bool_t   b;
  unsigned char o;
  const char   *s;
  double        d;

} dbus_any_t;

/** Internal helper for dumping dbus message content to stdout
 *
 * @param self xdbusrx_t object pointer
 * @param lev  container level (for indentation)
 *
 * @return TRUE on success, FALSE otherwise
 */
static bool xdbus_dump_message_sub(xdbusrx_t *self, int lev)
{
  bool ok = true;

  dbus_any_t v = { .u64 = 0 };

  int type = xdbusrx_at(self);

  if( type == DBUS_TYPE_INVALID )
  {
    return false;
  }

  printf("%*s", lev*4, "");
  switch( type )
  {
  case DBUS_TYPE_INVALID:
    ok = false;
    break;

  case DBUS_TYPE_BYTE:
    ok = xdbusrx_read_byte(self, &v.o);
    printf("byte:0x%02x", v.o);
    break;

  case DBUS_TYPE_BOOLEAN:
    ok = xdbusrx_read_bool(self, &v.b);
    printf("bool:%s", v.b ? "true" : "false");
    break;

  case DBUS_TYPE_INT16:
    ok = xdbusrx_read_int16(self, &v.i16);
    printf("int16:%d", v.i16);
    break;

  case DBUS_TYPE_INT32:
    ok = xdbusrx_read_int32(self, &v.i32);
    printf("int32:%d", v.i32);
    break;

  case DBUS_TYPE_INT64:
    ok = xdbusrx_read_int64(self, &v.i64);
    printf("int32:%lld", (long long)v.i64);
    break;

  case DBUS_TYPE_UINT16:
    ok = xdbusrx_read_uint16(self, &v.u16);
    printf("uint16:%u", v.u16);
    break;

  case DBUS_TYPE_UINT32:
    ok = xdbusrx_read_uint32(self, &v.u32);
    printf("uint32:%u", v.u32);
    break;

  case DBUS_TYPE_UINT64:
    ok = xdbusrx_read_uint64(self, &v.u64);
    printf("uint32:%llu", (unsigned long long)v.u64);
    break;

  case DBUS_TYPE_DOUBLE:
    ok = xdbusrx_read_double(self, &v.d);
    printf("double:%g", v.d);
    break;

  case DBUS_TYPE_STRING:
    ok = xdbusrx_read_string(self, &v.s);
    printf("string:'%s'", v.s);
    break;

  case DBUS_TYPE_ARRAY:
    if( (ok = xdbusrx_enter_array(self)) )
    {
      printf("array('%s'):[\n", xdbusrx_signature(self));
      while( xdbus_dump_message_sub(self, lev+1) ) {}
      printf("%*s", lev*4, "");
      printf("]");
      xdbusrx_leave_container(self);
    }
    break;

  case DBUS_TYPE_VARIANT:
    if( (ok = xdbusrx_enter_variant(self)) )
    {
      printf("variant('%s'):\n", xdbusrx_signature(self));
      while( xdbus_dump_message_sub(self, lev+1) ) {}
      xdbusrx_leave_container(self);
    }
    break;

  case DBUS_TYPE_STRUCT:
    if( (ok = xdbusrx_enter_struct(self)) )
    {
      printf("struct:(\n");
      while( xdbus_dump_message_sub(self, lev+1) ) {}
      printf("%*s", lev*4, "");
      printf(")");
      xdbusrx_leave_container(self);
    }
    break;

  case DBUS_TYPE_DICT_ENTRY:
    if( (ok = xdbusrx_enter_dict(self)) )
    {
      printf("dict:{\n");
      while( xdbus_dump_message_sub(self, lev+1) ) {}
      printf("%*s", lev*4, "");
      printf("}");
      xdbusrx_leave_container(self);
    }
    break;

  case DBUS_TYPE_OBJECT_PATH:
  case DBUS_TYPE_SIGNATURE:
  case DBUS_TYPE_UNIX_FD:

  default:
    abort();
    break;
  }
  printf("\n");

  return ok;
}

/** Dump dbus message content to stdout (for debugging purposes)
 *
 * @param msg dbus message
 */
void xdbus_dump_message(DBusMessage *msg)
{
  xdbusrx_t *rx = xdbusrx_create(msg);

  while( xdbus_dump_message_sub(rx, 0) ) {}
  printf("\n");

  xdbusrx_delete(rx);
}
