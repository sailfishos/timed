/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
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
