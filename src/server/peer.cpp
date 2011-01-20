#include <QDBusPendingReply>


#include <qmlog>

#include <timed/interface>
#include "credentials.h"

#include "misc.h"
#include "peer.h"

#if 0
pid_t peer::pid_by_dbus_message(const QDBusMessage &message)
{
  QString sender = message.service() ;
  uint32_t owner_id = get_name_owner_from_dbus_sync(Maemo::Timed::bus(), sender) ;

  if (owner_id == ~0u)
  {
    log_warning("can't get owner (pid) of the caller, already terminated?") ;
    return -1 ;
  }

  pid_t pid = owner_id ;
  return pid ;
}
#endif

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

#if 0
string peer::info_by_dbus_message(const QDBusMessage &message)
{
  pid_t pid = peer::pid_by_dbus_message(message) ;
  ostringstream os ;
  os << "{cmd='" << cmdline_by_pid(pid) << "', pid=" << pid << ", cred=" ;
  os << credentials_t::from_given_process(pid).str() << "}" ;
  return os.str() ;
}
#endif

peer_t::peer_t(bool mode)
{
  enabled = mode ;
}

peer_t::~peer_t()
{
  for (map<string, peer_entry_t*>::const_iterator it=entries.begin(); it!=entries.end(); ++it)
    delete it->second ;
}

string peer_t::info(const string &name)
{
  const peer_entry_t *entry = NULL ;
  if (enabled)
  {
    map<string, peer_entry_t*>::const_iterator it = entries.find(name) ;
    if (it!=entries.end())
      entry = it->second ;
    else
      entry = entries[name] = new peer_entry_t(name) ;
  }
  return str_printf("PEER::%s %s", name.c_str(), entry ? entry->get_info().c_str() : "DISABLED") ;
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
    if (unix_process_id==~0)
      info = str_printf("pid: unknown") ;
    else
    {
      pid_t pid = unix_process_id ;
      string cmd_line = cmdline_by_pid(pid) ;
      credentials_t cred = credentials_t::from_given_process(pid) ;
      info = str_printf("pid=%d, cmdline='%s', creds=%s", pid, cmd_line.c_str(), cred.str().c_str()) ;
    }
  }
  delete watcher ;
  watcher = NULL ;
  log_notice("PEER::%s %s", name.c_str(), info.c_str()) ;
}

peer_entry_t::~peer_entry_t()
{
  delete watcher ;
}
