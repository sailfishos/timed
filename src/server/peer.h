#ifndef MAEMO_TIMED_PEER_H
#define MAEMO_TIMED_PEER_H

#include <string>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>

struct peer_t ;
struct peer_entry_t ;

struct peer_t
{
  peer_t(bool mode) ;
  ~peer_t() ;
  std::string info(const std::string &) ;
private:
  bool enabled ;
  std::map<std::string, peer_entry_t*> entries ;
} ;

struct peer_entry_t : public QObject
{
  peer_entry_t(const std::string &new_name, QObject *parent=NULL) ;
  virtual ~peer_entry_t() ;
  bool is_pending() const { return watcher != NULL ; }
  std::string get_info() const { return info ; }

  std::string name, info ;
  QDBusPendingCallWatcher *watcher ;

  Q_OBJECT ;
private Q_SLOTS:
  void reply_slot(QDBusPendingCallWatcher *w) ;
} ;

#if 0
namespace peer
{
  pid_t pid_by_dbus_message(const QDBusMessage &message) ;
  std::string cmdline_by_pid(pid_t pid) ;
  std::string info_by_dbus_message(const QDBusMessage &message) ;
}
#endif
#endif//MAEMO_TIMED_PEER_H
