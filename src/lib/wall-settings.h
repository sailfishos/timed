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
#ifndef WALL_SETTINGS_H
#define WALL_SETTINGS_H

#include <stdint.h>

#include "wall-declarations.h"
#include "qmacro.h"

#include "nanotime.h"

struct Maemo::Timed::WallClock::wall_settings_pimple_t
{
  bool valid ;
  uint32_t opcodes ;
  nanotime_t time_at_zero ;
  int32_t offset ;
  QString zone ;
  wall_settings_pimple_t()
  {
    valid = true ;
    opcodes = 0 ;
    offset = 0 ;
  }
  QString str() const ;
} ;

declare_qtdbus_io(Maemo::Timed::WallClock::wall_settings_pimple_t) ;

#endif
