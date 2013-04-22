/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2011 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
**   Author: Victor Portnov <ext-victor.portnov@nokia.com>                **
**                                                                        **
**     This file is part of Timed                                         **
**                                                                        **
**     Timed is free software; you can redistribute it and/or modify      **
**     it under the terms of the GNU Lesser General Public License        **
**     version 2.1 as published by the Free Software Foundation.          **
**                                                                        **
**     Timed is distributed in the hope that it will be useful, but       **
**     WITHOUT ANY WARRANTY;  without even the implied warranty  of       **
**     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               **
**     See the GNU Lesser General Public License  for more details.       **
**                                                                        **
**   You should have received a copy of the GNU  Lesser General Public    **
**   License along with Timed. If not, see http://www.gnu.org/licenses/   **
**                                                                        **
***************************************************************************/
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

# ifndef __cplusplus
#  error This is a C++ only header
# endif

#include <string>
#include <set>
using namespace std ;

#include <sys/types.h>
#include <QDBusMessage>
#include <QDBusConnection>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <iodata-qt5/iodata>
#else
#include <iodata/iodata>
#endif

uint32_t get_name_owner_from_dbus_sync(const QDBusConnection &bus, const QString &name) ;

struct credentials_t
{
  string uid, gid ;
  set<string> tokens ;

  // TODO: make nobody/nobody run-time option: /etc/timed.rc
  credentials_t() : uid("nobody"), gid("nobody") { }
  credentials_t(const credentials_t &x) : uid(x.uid), gid(x.gid), tokens(x.tokens) { }
  credentials_t(const string &u, const string &g) : uid(u), gid(g) { }

  bool apply() const ; // set the credentials for the current process
  bool apply_and_compare() ; // set the credentials and check if they are really set

  static credentials_t from_current_process() ; // get the credentials of the current process
  static credentials_t from_given_process(pid_t) ; // get the credentials of some other process
  credentials_t(const QDBusMessage &msg) ; // get from dbus client

  iodata::record *save() const ;
  credentials_t(const iodata::record *r) ; // load

  std::string str() const ;
} ;

#endif // CREDENTIALS_H
