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

#ifndef ONITZ_H
#define ONITZ_H

#include <string>
#include <sstream>
using namespace std ;

#include <QObject>
#include <QDateTime>

#include "../common/log.h"

#if OFONO
# include "networktime.h"
# include "networkoperator.h"
# include "networktimeinfo.h"
#endif


#include "../lib/nanotime.h"

#include "misc.h"

#include "cellular.h"

struct cellular_handler : public QObject
{
  Q_OBJECT ;
private:
  static cellular_handler *static_object ;
  virtual ~cellular_handler() ;
  cellular_handler() ;

signals:

public:
  static cellular_handler *object() ;
  static void uninitialize() ;
  void fake_nitz_signal(int mcc, int offset, int time, int dst) ;
public slots:
} ;

#endif
