/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "stab.h"

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

/* ------------------------------------------------------------------------- *
 * button_t
 * ------------------------------------------------------------------------- */

typedef struct button_t button_t;

/** Timed button data */
struct button_t
{
    /** Array of button attributes */
    stab_t *stab; // stab_get(i) -> attr_t *
};

void button_ctor(button_t *self);
void button_dtor(button_t *self);

button_t *button_create(void);
void button_delete(button_t *self);
void button_delete_cb(void *self);

#ifdef __cplusplus
};
#endif

#endif /* BUTTON_H_ */
