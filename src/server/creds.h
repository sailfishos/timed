#ifndef CREDS_H
#define CREDS_H

#include <string>
#include <set>
using namespace std ;

#if 1 // F_CREDS_AEGIS
#include <sys/creds.h>
#endif // F_CREDS_AEGIS

#include <QDBusMessage>

#include <iodata/iodata>

struct credentials_t
{
  string uid, gid ;
  set<string> tokens ;

  credentials_t() : uid("nobody"), gid("nogroup") { }

#if 1 // F_CREDS_AEGIS
  static credentials_t from_aegis_creds(creds_t aegis_creds) ;
  static bool aegis_add_string_to_creds(creds_t &aegis_creds, const string &token, bool silent) ;
  creds_t to_aegis_creds() const ;
#endif // F_CREDS_AEGIS

  bool apply() const ; // set the credentials for the current process
  void from_current_process() ; // get the credentials of the current process
  bool apply_and_compare() ; // set the credentials and check if they are really set

  static credentials_t from_dbus_connection(const QDBusMessage &msg) ;

  iodata::record *save() const ;
  void load(const iodata::record *r) ;
} ;

#endif // CRED_H
