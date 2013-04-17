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
#ifndef WALL_INFO_H
#define WALL_INFO_H

#include <stdint.h>
#include <QString>
#include <QVector>

#include "../lib/wall-declarations.h"
#include "nanotime.h"

struct Maemo::Timed::WallClock::wall_info_pimple_t
{
  bool flag_time_nitz ;
  bool flag_local_cellular ;
  bool flag_auto_dst ;
  bool flag_format_24 ;

  QVector<nanotime_t> clocks ;
  QVector<QString> zones ;
  QVector<int> offsets ;

  QVector<int> data_sources ;
  enum { CL=0,ZO=1,OF=2,N_DATA=3, N_CLOCKS=4, N_ZONES=2, N_OFFSETS=2 } ;

  QString localtime_symlink ; // "/usr/share/zoneinfo/Europe/Helsinki"
  QString human_readable_tz ; // "Europe/Babruisk" or "GMT+5:30"
  int seconds_east_of_gmt ;  // -6x3600 (USA), 3x3600 (MSK), 5.5x3600 (India)
  QString abbreviation ; // "MSK", "EEST", "UTC"

  bool nitz_supported ;
  QString default_timezone ;

  nanotime_t systime_diff ;
} ;

// declare_qtdbus_io(Maemo::Timed::WallClock::Info) ;

#endif
