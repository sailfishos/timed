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
#ifndef LIBTIMED_LOG_H
#define LIBTIMED_LOG_H

#define LIBTIMED_LOGGING_DISPATCHER (Maemo::Timed::get_libtimed_logging_dispatcher())

#ifndef QMLOG_DISPATCHER
#define QMLOG_DISPATCHER LIBTIMED_LOGGING_DISPATCHER
#endif

namespace qmlog { class dispatcher_t ; }

namespace Maemo
{
  namespace Timed
  {
    qmlog::dispatcher_t *get_libtimed_logging_dispatcher() ;
    void croak() ;
  }
}

#include <qmlog>

namespace Maemo
{
  namespace Timed
  {
    class libtimed_logging_dispatcher_t : public qmlog::dispatcher_t
    {
    protected:
      void set_process_name(const std::string &new_name) ;
    public:
      libtimed_logging_dispatcher_t() ;
      virtual ~libtimed_logging_dispatcher_t() ;
    } ;
    extern libtimed_logging_dispatcher_t libtimed_logging_dispatcher ;
    qmlog::dispatcher_t *get_libtimed_logging_dispatcher() ;
  }
}

#endif
