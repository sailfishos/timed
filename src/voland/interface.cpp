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

#include "interface.h"

Maemo::Timed::Voland::Interface::Interface(QObject *parent)
  : QDBusAbstractInterface
    (
     Maemo::Timed::Voland::service(),
     Maemo::Timed::Voland::objpath(),
     Maemo::Timed::Voland::interface(),
     Maemo::Timed::Voland::bus(),
     parent
    )
{
}

Maemo::Timed::Voland::Interface::Interface(QDBusConnection bus_connection, QObject *parent)
  : QDBusAbstractInterface
    (
     Maemo::Timed::Voland::service(),
     Maemo::Timed::Voland::objpath(),
     Maemo::Timed::Voland::interface(),
     bus_connection,
     parent
    )
{
}

Maemo::Timed::Voland::TaInterface::TaInterface(QObject *parent)
  : QDBusAbstractInterface
    (
     Maemo::Timed::Voland::service(),
     Maemo::Timed::Voland::ta_objpath(),
     Maemo::Timed::Voland::ta_interface(),
     Maemo::Timed::Voland::bus(),
     parent
    )
{
}

Maemo::Timed::Voland::ActivationInterface::ActivationInterface(QObject *parent)
  : QDBusAbstractInterface
    (
     Maemo::Timed::Voland::activation_service(),
     Maemo::Timed::Voland::activation_objpath(),
     Maemo::Timed::Voland::activation_interface(),
     Maemo::Timed::Voland::bus(),
     parent
    )
{
}
