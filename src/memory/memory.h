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
#ifndef TIMED_MEMORY_H
#define TIMED_MEMORY_H

#include <QCoreApplication>
#include <QTimer>

class memory_t : public QCoreApplication
{
  Q_OBJECT ;
  QTimer *timer ;
public:
  memory_t(int &ac, char ** &av) : QCoreApplication(ac, av)
  {
    timer = new QTimer ;
    timer->setSingleShot(true) ;
    connect(timer, SIGNAL(timeout()), this, SLOT(quit())) ;
    timer->start(1000) ;
  }
  virtual ~memory_t()
  {
    delete timer ;
  }
} ;

#endif
