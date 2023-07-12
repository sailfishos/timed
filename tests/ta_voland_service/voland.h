/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef VOLAND_H_
#define VOLAND_H_

#include <stdbool.h>

#include "voland_dbus.h"

#ifdef __cplusplus
extern "C" {
#elif 0
} /* fool JED indentation ... */
#endif

bool voland_init(void);
void voland_quit(void);

/* hooks for test automation ta_voland */
int voland_pid(void);
bool voland_answer(unsigned cookie, int button);
unsigned voland_top(void);
void voland_set_ta_mode(bool active);

#ifdef __cplusplus
};
#endif

#endif /* VOLAND_H_ */
