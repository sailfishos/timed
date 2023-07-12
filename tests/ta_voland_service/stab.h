/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef STAB_H_
#define STAB_H_

#include <stddef.h>

#include "attr.h"

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

/* ------------------------------------------------------------------------- *
 * stab_t
 * ------------------------------------------------------------------------- */

typedef struct stab_t stab_t;

stab_t *stab_create(void);
void stab_delete(stab_t *self);
void stab_delete_cb(void *self);

size_t stab_count(const stab_t *self);

void stab_add(stab_t *self, attr_t *attr);
attr_t *stab_get(const stab_t *self, size_t i);

#ifdef __cplusplus
};
#endif

#endif /* STAB_H_ */
