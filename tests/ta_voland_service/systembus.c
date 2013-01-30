/* ------------------------------------------------------------------------- *
 * Copyright (C) 2012-2013 Jolla Ltd.
 * Contact: Simo Piiroinen <simo.piiroinen@jollamobile.com>
 * License: LGPLv2
 * ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- *
 * This is a quick and dirty hack to force programs using libdbus to
 * connect to system bus when they are trying to connect to session
 * bus.
 * ------------------------------------------------------------------------- */

#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum
{
  DBUS_BUS_SESSION,    /**< The login session bus */
  DBUS_BUS_SYSTEM,     /**< The systemwide bus */
  DBUS_BUS_STARTER     /**< The bus that started us, if any */
} DBusBusType;

static void init(void) __attribute__((constructor));
static void fini(void) __attribute__((destructor));

/** Recursion level - We only wish to trap calls from outside libdbus */
static volatile int hack = 0;

/** DONTCARE version of write() for debugging purposes */
static inline void write_no_check(int fd, const void *data, size_t size)
{
  if( write(fd, data, size) < 0 )
  {
    // I don't care and in this case neither should static analyzers ...
  }
}

/** Mark that we actually got preloaded */
static void init(void)
{
  static const char m[] = "@ HACK INIT\n";
  write_no_check(2, m, sizeof m - 1);
}

/** Mark that we got unloaded */
static void fini(void)
{
  static const char m[] = "@ HACK FINI\n";
  write_no_check(2, m, sizeof m - 1);
}

/** Supply fake results for $DBUS_SESSION_BUS_ADDRESS queries */
char *getenv(const char *key)
{
  static void *(*real)(const char *) = 0;

  dprintf(2, "XXX: %s(%s) @ hack=%d\n", __FUNCTION__, key, hack);

  if( real == 0 )
  {
    if( !(real = dlsym(RTLD_NEXT, __FUNCTION__)) )
    {
      abort();
    }
  }

  if( !strcmp(key, "DBUS_SESSION_BUS_ADDRESS") )
  {
    return "just do it";
  }

  return real(key);
}

/** Get system bus regardless of what the caller asked for */
void *dbus_bus_get_private(int type, void *error)
{
  static void *(*real)(int, void *) = 0;

  dprintf(2, "XXX: %s(%d,%p) @ hack=%d\n", __FUNCTION__, type, error, hack);

  if( real == 0 )
  {
    if( !(real = dlsym(RTLD_NEXT, __FUNCTION__)) )
    {
      abort();
    }
  }

  if( type != DBUS_BUS_SYSTEM )
  {
    static const char m[] = "BLOCK PRIVATE SESSION BUS\n";
    write_no_check(2, m, sizeof m - 1);
    type = DBUS_BUS_SYSTEM;
  }

  ++hack;
  void *res = real(type, error);
  --hack;
  return res;
}

/** Get system bus regardless of what the caller asked for */
void *dbus_bus_get(int type, void *error)
{
  static void *(*real)(int, void *) = 0;

  dprintf(2, "XXX: %s(%d,%p) @ hack=%d\n", __FUNCTION__, type, error, hack);

  if( real == 0 )
  {
    if( !(real = dlsym(RTLD_NEXT, __FUNCTION__)) )
    {
      abort();
    }
  }

  if( type != DBUS_BUS_SYSTEM )
  {
    static const char m[] = "BLOCK SHARED SESSION BUS\n";
    write_no_check(2, m, sizeof m - 1);
    type = DBUS_BUS_SYSTEM;
  }

  ++hack;
  void *res = real(type, error);
  --hack;
  return res;
}

/** Get system bus regardless of what the caller asked for */
void *dbus_connection_open_private(const char *address,
                                   void *error)
{
  static void *(*real)(const char *, void *) = 0;

  dprintf(2, "XXX: %s(%s,%p) @ hack=%d\n", __FUNCTION__, address, error, hack);

  if( real == 0 )
  {
    if( !(real = dlsym(RTLD_NEXT, __FUNCTION__)) )
    {
      abort();
    }
  }

  void *res = 0;

  if( ++hack == 1 )
  {
    dprintf(2, "XXX: REPLACE '%s' with SYSTEM-BUS\n", address);
    res = dbus_bus_get_private(DBUS_BUS_SYSTEM, error);
  }
  else
  {
    res = real(address, error);
  }
  --hack;
  return res;
}
