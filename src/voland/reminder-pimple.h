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
#ifndef REMINDER_PIMPLE_H
#define REMINDER_PIMPLE_H

#include <stdint.h>

#include <QMap>
#include <QString>
#include <QVector>

#include "../lib/qmacro.h"
#include "interface.h"

namespace Maemo
{
  namespace Timed
  {
    namespace Voland
    {
      struct button_io_t
      {
        QMap<QString, QString> attr ;
      } ;
      struct reminder_pimple_t
      {
        uint32_t cookie, flags ;
        QMap<QString, QString> attr ;
        QVector<button_io_t> buttons ;
        reminder_pimple_t()
        {
          cookie = flags = 0 ;
        }
      } ;
    }
  }
}

declare_qtdbus_io(Maemo::Timed::Voland::button_io_t) ;

#endif
