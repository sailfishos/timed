#include <string>

#include <QDBusMessage>

namespace peer
{
  pid_t pid_by_dbus_message(const QDBusMessage &message) ;
  std::string cmdline_by_pid(pid_t pid) ;
  std::string info_by_dbus_message(const QDBusMessage &message) ;
}
