/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

#ifndef ATTR_H_
# define ATTR_H_

# ifdef __cplusplus
extern "C" {
# elif 0
} /* fool JED indentation ... */
# endif

/* ------------------------------------------------------------------------- *
 * attr_t
 * ------------------------------------------------------------------------- */

typedef struct attr_t attr_t;

/** Timed (key,val) attribute pair */
struct attr_t
{
  /** Key name */
  char *key;

  /** String value */
  char *val;
};

void    attr_ctor(attr_t *self, const char *key);
void    attr_dtor(attr_t *self);

attr_t *attr_create(const char *key, const char *val);
void    attr_delete(attr_t *self);
void    attr_delete_cb(void *self);

void    attr_set(attr_t *self, const char *val);

# ifdef __cplusplus
};
# endif

#endif /* ATTR_H_ */
