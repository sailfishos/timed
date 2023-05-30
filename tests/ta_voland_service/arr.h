/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef ARR_H_
#define ARR_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

/* ------------------------------------------------------------------------- *
 * arr_t
 * ------------------------------------------------------------------------- */

typedef struct arr_t arr_t;

arr_t *arr_create(void (*free_cb)(void *));
void arr_delete(arr_t *self);
void arr_delete_cb(void *self);

size_t arr_count(const arr_t *self);
void arr_clear(arr_t *self);
void *arr_get(arr_t *self, size_t i);
void arr_remove(arr_t *self, size_t i);

size_t arr_push(arr_t *self, void *elem);
void *arr_pop(arr_t *self);
void *arr_tail(arr_t *self);

#ifdef __cplusplus
};
#endif

#endif /* ARR_H_ */
