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
#include <set>
using namespace std ;

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "../common/log.h"

#include "unix-signal.h"

UnixSignal * UnixSignal::static_object = NULL ;

UnixSignal * UnixSignal::object()
{
  if (static_object==NULL)
    static_object = new UnixSignal ;
  return static_object ;
}

UnixSignal::UnixSignal()
  : notifier(NULL)
{
  // int pipe_res = pipe2(this->pipe, O_NONBLOCK | O_CLOEXEC) ;
  int pipe_res = ::pipe(this->pipe) ;
  if(pipe_res<0)
  {
    log_critical("creating a pipe pair failed: %m") ;
    pipe[0] = pipe[1] = -1 ;
    return ;
  }
  for(int i=0; i<2; ++i)
  {
    bool ok = fcntl(pipe[i], F_SETFD, FD_CLOEXEC) == 0 ;
    if(!ok)
      log_critical("can't set FD_CLOEXEC flag for pipe[%d]: %m", i) ;
    else
    {
      ok = fcntl(pipe[i], F_SETFL, O_NONBLOCK) == 0 ;
      if(!ok)
        log_critical("can't set O_NONBLOCK flag for pipe[%d]: %m", i) ;
    }
    if(!ok)
    {
      close(pipe[0]), close(pipe[1]) ;
      pipe[0] = pipe[1] = -1 ;
      return ;
    }
  }
  notifier = new QSocketNotifier(read_fd(), QSocketNotifier::Read) ;
  QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(process_signal(int))) ;
}

UnixSignal::~UnixSignal()
{
  delete notifier ;
  set<int> sigs = handled ;
  for (set<int>::iterator it=sigs.begin(); it!=sigs.end(); ++it)
    handle(*it, false) ;
  close(write_fd()) ;
  close(read_fd()) ;
}

void UnixSignal::uninitialize()
{
  if (static_object==NULL)
    return ;
  delete static_object ;
  static_object = NULL ;
}

void UnixSignal::handler(int signo)
{
  if(object()->pending.count(signo))
    return ;
  object()->pending.insert(signo) ;
  ssize_t res = write(object()->write_fd(), &signo, sizeof(int)) ;
  if(res != sizeof(int))
  {
    log_critical("can't write the signal id through the pipe: %m") ;
    object()->pending.erase(signo) ;
  }
}

void UnixSignal::process_signal(int fd)
{
  log_assert(fd==read_fd()) ;
  int signo ;
  ssize_t res = read(fd, &signo, sizeof(int)) ;
  if(res != sizeof(int))
  {
    log_critical("can't read signal id (some signals could be blocked now), %m") ;
    return ;
  }
  pending.erase(signo) ;
  emit signal(signo) ;
}

int UnixSignal::handle(int signo, bool enable)
{
  if (enable and handled.count(signo))
    return 0 ;
  if (not enable and handled.count(signo)==0)
    return 0 ;

  struct sigaction sa ;
  sa.sa_flags = 0 ;
  if(enable)
  {
    sigfillset(&sa.sa_mask) ;
    sa.sa_handler = UnixSignal::handler ;
    handled.insert(signo) ;
  }
  else
  {
    sigemptyset(&sa.sa_mask) ;
    sa.sa_handler = SIG_DFL ;
    handled.erase(signo) ;
  }
  pending.erase(signo) ;
  return sigaction(signo, &sa, NULL) ;
}
