/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
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
#ifndef SINGLESHOT_H
#define SINGLESHOT_H

#include <QTimer>

class simple_timer : public QTimer
{
  unsigned period ;
  Q_OBJECT ;
public:
  simple_timer(unsigned p, QObject *parent=NULL) : QTimer(parent), period(p) { setSingleShot(true) ; }
  void start() { QTimer::start(period) ; }
  QString status() { return QString("%1, period=%2").arg(isActive()?"active":"sleeping").arg(period) ; }
} ;

#endif
