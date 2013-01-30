/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef MAINLOOP_H_
# define MAINLOOP_H_

# ifdef __cplusplus
extern "C" {
# elif 0
} /* fool JED indentation ... */
# endif

void mainloop_stop(int exit_code);
int  mainloop_run(void);

# ifdef __cplusplus
};
# endif

#endif /* MAINLOOP_H_ */
