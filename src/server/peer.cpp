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
#include <QDBusPendingReply>


#include "../common/log.h"

#include "../lib/interface.h"
#include "credentials.h"

#include "misc.h"
#include "peer.h"

static string cmdline_by_pid(pid_t pid)
{
  const int max_len = 1024 ;
  char buf[max_len+1] = "<no-cmd-line>" ;
  if(FILE *fp = fopen(str_printf("/proc/%d/cmdline", pid).c_str(), "r"))
  {
    size_t n = fread(buf, 1, max_len,fp) ;
    if (n>0)
    {
      buf[n]='\0' ;
      for (unsigned i=0; i+1<n; ++i)
        if (not buf[i])
          buf[i]=' ' ;
    }
    fclose(fp) ;
  }
  return (string) buf ;
}

peer_t::peer_t(bool mode, QObject *parent) :
  QObject(parent)
{
  enabled = mode ;
}

peer_t::~peer_t()
{
}

string peer_t::info(const string &name)
{
  if (enabled)
  {
      new peer_entry_t(name, this) ;
  }
  return str_printf("PEER::%s %s", name.c_str(), enabled ? "info requested" : "DISABLED") ;
}

peer_entry_t::peer_entry_t(const string &new_name, QObject *parent) :
  QObject(parent), name(new_name)
{
  QString service   =  "org.freedesktop.DBus" ;
  QString path      = "/org/freedesktop/DBus" ;
  QString interface =  "org.freedesktop.DBus" ;
  QString method    = "GetConnectionUnixProcessID" ;
  QDBusMessage req  = QDBusMessage::createMethodCall(service, path, interface, method) ;
  req << (QString)name.c_str() ;

  watcher = new QDBusPendingCallWatcher(Maemo::Timed::bus().asyncCall(req)) ;
  bool ok = QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(reply_slot(QDBusPendingCallWatcher*))) ;
  log_assert(ok, "oops, connection failed in peer_entry_t contructor") ;
}

void peer_entry_t::reply_slot(QDBusPendingCallWatcher *w)
{
  std::string info ;

  if (w!=watcher)
    log_critical("QDBusPendingCallWatcher mismatch: w=%p, watcher=%p", w, watcher) ;
  QDBusPendingReply<uint> reply = *w ;
  if (reply.isError())
  {
    QDBusError e = reply.error() ;
#define QC toStdString().c_str()
    info = str_printf("ERROR: [name='%s', message='%s', type=%d]", e.name().QC, e.message().QC, e.type()) ;
#undef QC
  }
  else
  {
    uint unix_process_id = reply.value() ;
    if (unix_process_id==~0u)
      info = str_printf("pid: unknown") ;
    else
    {
      pid_t pid = unix_process_id ;
      string cmd_line = cmdline_by_pid(pid) ;
      credentials_t cred = credentials_t::from_given_process(pid) ;
      info = str_printf("pid=%d, cmdline='%s', creds=%s", pid, cmd_line.c_str(), cred.str().c_str()) ;
    }
  }
  log_notice("PEER::%s %s", name.c_str(), info.c_str()) ;
  delete this;
}

peer_entry_t::~peer_entry_t()
{
  delete watcher ;
  watcher = NULL ;
}
