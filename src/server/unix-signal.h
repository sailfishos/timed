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
#ifndef UNIX_SIGNAL_H
#define UNIX_SIGNAL_H

#include <set>

#include <signal.h>

#include <QObject>
#include <QSocketNotifier>

class UnixSignal : public QObject
{
  UnixSignal() ;
 ~UnixSignal() ;
  static UnixSignal *static_object ;
  Q_OBJECT ;
  int pipe[2] ;
  int read_fd() { return pipe[0] ; }
  int write_fd() { return pipe[1] ; }
  std::set<int> pending ;
  std::set<int> handled ;
  QSocketNotifier *notifier ;
  static void handler(int signo) ;
  void restore_handler(int signo) ;
  void disable_signal(int signo) ;
public:
  static UnixSignal *object() ;
  int handle(int signo, bool enable=true) ;
  static void uninitialize() ;
private slots:
  void process_signal(int fd) ;
signals:
  void signal(int signo) ;
} ;

#endif
