/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef XTIMED_H_
# define XTIMED_H_

# include "xtimed_dbus.h"
# include <stdbool.h>

# ifdef __cplusplus
extern "C" {
# elif 0
} /* fool JED indentation ... */
# endif

bool xtimed_init(void);
void xtimed_quit(void);

void xtimed_set_runstate_cb(void (*cb)(bool));

bool xtimed_dialog_response(unsigned cookie, int button);

# ifdef __cplusplus
};
# endif

#endif /* XTIMED_H_ */
