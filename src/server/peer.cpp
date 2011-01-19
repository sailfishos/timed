#include <qmlog>

#include <timed/interface>
#include "credentials.h"

#include "misc.h"
#include "peer.h"

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

string peer::cmdline_by_pid(pid_t pid)
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

string peer::info_by_dbus_message(const QDBusMessage &message)
{
  pid_t pid = peer::pid_by_dbus_message(message) ;
  ostringstream os ;
  os << "{cmd='" << cmdline_by_pid(pid) << "', pid=" << pid << ", cred=" ;
  os << credentials_t::from_given_process(pid).str() << "}" ;
  return os.str() ;
}
