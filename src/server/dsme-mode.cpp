#include <string>
using namespace std ;

#include <QDBusReply>
#include <QTimer>

#include <dsme/dsme_dbus_if.h>
#include <qmlog>

#include "dsme-mode.h"

dsme_mode_t::dsme_mode_t()
{
  mode_known = false ;
  dsme_iface = new DsmeReqInterface(this) ;
  request_watcher = NULL ;
  bool res = QDBusConnection::systemBus().connect(dsme_service, dsme_sig_path, dsme_sig_interface, dsme_state_change_ind, this, SLOT(dsme_state_signalled(const QString &))) ;
  if (not res)
    log_critical("can't connect to dsme state change signal") ;
}

void dsme_mode_t::init_request()
{
  QDBusPendingCall call = dsme_iface->get_state_async() ;
  request_watcher = new QDBusPendingCallWatcher(call, this) ;
  QObject::connect(request_watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(request_finished(QDBusPendingCallWatcher*))) ;
}

void dsme_mode_t::dsme_state_signalled(const QString &new_mode)
{
  string m = new_mode.toStdString() ;
  if (m.empty())
  {
    log_error("dsme signalled empty mode") ;
    return ;
  }
  if (m==mode)
  {
    log_notice("dsme signalled the current mode='%s' again", m.c_str()) ;
    return ;
  }
  log_notice("dsme signalled mode='%s', old mode was '%s'", m.c_str(), mode.c_str()) ;
  mode = m ;
  emit mode_is_changing(mode) ;
}

void dsme_mode_t::request_finished(QDBusPendingCallWatcher *watcher)
{
  log_debug() ;
  if (not mode_known) // ignore, if device mode already known
  {
    QDBusPendingReply<QString> r = *watcher ;

    if (r.isError())
    {
      log_error("dsme state query error: '%s", r.error().message().toStdString().c_str()) ;
      log_notice("dsme state query will be restarted in 2 seconds") ;
      QTimer::singleShot(2000, this, SLOT(init_request())) ;
      return ;
    }
    mode = r.value().toStdString() ;
    emit mode_reported(mode) ;
  }
  else
    log_warning("ignoring dsme state request reply, current state is '%s'", mode.c_str()) ;
}

