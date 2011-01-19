#ifndef MAEMO_TIMED_PEER_H
#define MAEMO_TIMED_PEER_H

#include <string>

#include <QDBusMessage>

namespace peer
{
  pid_t pid_by_dbus_message(const QDBusMessage &message) ;
  std::string cmdline_by_pid(pid_t pid) ;
  std::string info_by_dbus_message(const QDBusMessage &message) ;
}

#endif//MAEMO_TIMED_PEER_H
