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
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "log.h"

#include "unix-signal.h"

UnixSignal * UnixSignal::object()
{
  static UnixSignal *p = NULL ;
  if(p==NULL)
    p = new UnixSignal ;
  return p ;
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
  struct sigaction sa ;
  sa.sa_flags = 0 ;
  if(enable)
  {
    sigfillset(&sa.sa_mask) ;
    sa.sa_handler = UnixSignal::handler ;
  }
  else
  {
    sigemptyset(&sa.sa_mask) ;
    sa.sa_handler = SIG_DFL ;
  }
  pending.erase(signo) ;
  return sigaction(signo, &sa, NULL) ;
}
