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
#include "f.h"

#include <sys/time.h>

#include <pcrecpp.h>

#include "../common/log.h"

#include "onitz.h"
#include "misc.h"

cellular_handler::cellular_handler()
{
}

cellular_handler *cellular_handler::static_object = NULL ;

cellular_handler *cellular_handler::object()
{
  if(static_object==NULL)
    static_object = new cellular_handler ;
  return static_object ;
}

void cellular_handler::uninitialize()
{
  if(static_object==NULL)
    return ;
  delete static_object ;
  static_object = NULL ;
}

cellular_handler::~cellular_handler()
{
}
