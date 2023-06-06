/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef XDBUS_H_
#define XDBUS_H_

#include <dbus/dbus.h>

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

/* ------------------------------------------------------------------------- *
 * xdbusrx_t
 * ------------------------------------------------------------------------- */

typedef struct xdbusrx_t xdbusrx_t;

gboolean xdbusrx_error_p(const xdbusrx_t *self);
gboolean xdbusrx_read_p(xdbusrx_t *self);
gboolean xdbusrx_todo_p(xdbusrx_t *self);

void xdbusrx_clear_error(xdbusrx_t *self);

xdbusrx_t *xdbusrx_create(DBusMessage *msg);
void xdbusrx_delete(xdbusrx_t *self);
void xdbusrx_delete_cb(void *self);

const char *xdbusrx_signature(xdbusrx_t *self);

gboolean xdbusrx_read_int16(xdbusrx_t *self, dbus_int16_t *data);
gboolean xdbusrx_read_int32(xdbusrx_t *self, dbus_int32_t *data);
gboolean xdbusrx_read_int64(xdbusrx_t *self, dbus_int64_t *data);
gboolean xdbusrx_read_uint16(xdbusrx_t *self, dbus_uint16_t *data);
gboolean xdbusrx_read_uint32(xdbusrx_t *self, dbus_uint32_t *data);
gboolean xdbusrx_read_uint64(xdbusrx_t *self, dbus_uint64_t *data);
gboolean xdbusrx_read_byte(xdbusrx_t *self, unsigned char *data);
gboolean xdbusrx_read_bool(xdbusrx_t *self, dbus_bool_t *data);
gboolean xdbusrx_read_double(xdbusrx_t *self, double *data);
gboolean xdbusrx_read_string(xdbusrx_t *self, const char **value);

gboolean xdbusrx_read_int(xdbusrx_t *self, gint *value);
gboolean xdbusrx_read_boolean(xdbusrx_t *self, gboolean *value);

gboolean xdbusrx_read_int_array(xdbusrx_t *self, int **data, int *count);
gboolean xdbusrx_read_string_array(xdbusrx_t *self, const char ***addr, int *count);

gboolean xdbusrx_enter_variant(xdbusrx_t *self);
gboolean xdbusrx_enter_struct(xdbusrx_t *self);
gboolean xdbusrx_enter_dict(xdbusrx_t *self);
gboolean xdbusrx_enter_array(xdbusrx_t *self);
gboolean xdbusrx_enter_array_ex(xdbusrx_t *self, int *pcount);
gboolean xdbusrx_leave_container(xdbusrx_t *self);

/* ------------------------------------------------------------------------- *
 * xdbustx_t
 * ------------------------------------------------------------------------- */

typedef struct xdbustx_t xdbustx_t;

xdbustx_t *xdbustx_create(DBusMessage *msg);
void xdbustx_delete(xdbustx_t *self);
void xdbustx_delete_cb(void *self);

void xdbustx_clear_error(xdbustx_t *self);

gboolean xdbustx_error_p(const xdbustx_t *self);
gboolean xdbustx_write_p(xdbustx_t *self);

gboolean xdbustx_write_int(xdbustx_t *self, gint value);
gboolean xdbustx_write_boolean(xdbustx_t *self, gboolean value);
gboolean xdbustx_write_path(xdbustx_t *self, const gchar *value);
gboolean xdbustx_write_string(xdbustx_t *self, const gchar *value);

gboolean xdbustx_push_variant(xdbustx_t *self, const char *signature);
gboolean xdbustx_pop_container(xdbustx_t *self);

/* ------------------------------------------------------------------------- *
 * misc
 * ------------------------------------------------------------------------- */

DBusMessage *xdbus_call_method(DBusConnection *con, DBusMessage *req);

void xdbus_free_string_array(char **v);
char **xdbus_dup_string_array(const char **v);

char **busListNames(DBusConnection *con);

char *busGetNameOwner(DBusConnection *con, const char *name);
DBusMessage *busGetNameOwner_req(const char *name);

void xdbus_dump_message(DBusMessage *msg);

#ifdef __cplusplus
};
#endif

#endif /* XDBUS_H_ */
