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
#include <string>
using namespace std ;

#include "log.h"

Maemo::Timed::libtimed_logging_dispatcher_t::libtimed_logging_dispatcher_t()
//  : qmlog::slave_dispatcher_t("libtimed")
{
  qmlog::log_file *varlog = new qmlog::log_file("/var/log/libtimed.log", qmlog::Full, this) ;

  varlog->enable_fields(qmlog::Monotonic_Milli | qmlog::Time_Milli) ;

  attach(qmlog::stderr()) ;

  set_process_name(qmlog::process_name()) ;

  // log_debug("blah in constructor... (in libtimed, pid=%d)", getpid()) ;
}

Maemo::Timed::libtimed_logging_dispatcher_t::~libtimed_logging_dispatcher_t()
{
  // log_debug("blah in destructor... (in libtimed, pid=%d)", getpid()) ;
}

void Maemo::Timed::libtimed_logging_dispatcher_t::set_process_name(const std::string &new_name)
{
  string name = "libtimed" ;
  if (not new_name.empty())
    name += "|", name += new_name ;
  dispatcher_t::set_process_name(name) ;
}

Maemo::Timed::libtimed_logging_dispatcher_t Maemo::Timed::libtimed_logging_dispatcher ;
