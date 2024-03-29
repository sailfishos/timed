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
#include <sstream>

#include "exception.h"

Maemo::Timed::Exception::Exception(const char *function, const char *message) throw()
{
    msg = message;
    fnc = function;
    human_readable = msg + " in " + fnc;
}

Maemo::Timed::Exception::~Exception() throw() {}

const char *Maemo::Timed::Exception::what() const throw()
{
    return human_readable.c_str();
}

const char *Maemo::Timed::Exception::message() const throw()
{
    return msg.c_str();
}

const char *Maemo::Timed::Exception::function() const throw()
{
    return fnc.c_str();
}
