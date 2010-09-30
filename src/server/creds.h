#ifndef CREDS_H
#define CREDS_H

#include <string>
#include <set>
using namespace std ;

#if F_CREDS_AEGIS_LIBCREDS
#include <sys/creds.h>
#endif // F_CREDS_AEGIS_LIBCREDS

#include <QDBusMessage>

#include <iodata/iodata>

struct credentials_t
{
  string uid, gid ;
  set<string> tokens ;

  credentials_t() : uid("nobody"), gid("nogroup") { }

#if F_CREDS_AEGIS_LIBCREDS
#endif // F_CREDS_AEGIS_LIBCREDS

  bool apply() const ; // set the credentials for the current process
  void from_current_process() ; // get the credentials of the current process
  bool apply_and_compare() ; // set the credentials and check if they are really set

  static credentials_t from_dbus_connection(const QDBusMessage &msg) ;

  iodata::record *save() const ;
  void load(const iodata::record *r) ;
} ;

#if F_CREDS_AEGIS_LIBCREDS
credentials_t aegis_credentials_from_dbus_connection(const QDBusMessage &message) ;
credentials_t aegis_credentials_from_creds_t(creds_t aegis_creds) ;
bool aegis_add_string_to_creds_t(creds_t &aegis_creds, const string &token, bool silent) ;
creds_t aegis_credentials_to_creds_t(const credentials_t &creds) ;
#endif

#endif // CRED_H
