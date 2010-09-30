#if F_CREDS_AEGIS_LIBCREDS

#ifndef AEGIS_H
#define AEGIS_H

#include <QDBusMessage>

#include <sys/creds.h>

#include "credentials.h"

namespace Aegis
{
  credentials_t credentials_from_dbus_connection(const QDBusMessage &message) ;
  credentials_t credentials_from_creds_t(creds_t aegis_creds) ;
  bool add_string_to_creds_t(creds_t &aegis_creds, const string &token, bool silent) ;
  creds_t credentials_to_creds_t(const credentials_t &creds) ;
}

#endif // AEGIS_H

#endif // F_CREDS_AEGIS_LIBCREDS
