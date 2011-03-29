#ifndef MAEMO_TIMED_DSME_MODE_H
#define MAEMO_TIMED_DSME_MODE_H

#include <string>
using std::string ;

#include <QObject>
#include <QDBusPendingCallWatcher>

#include "interfaces.h"

struct dsme_mode_t : public QObject
{
  QDBusPendingCallWatcher *request_watcher ;
  DsmeReqInterface *dsme_iface ;
  bool mode_known ;
  std::string mode ;
  dsme_mode_t() ;

  Q_OBJECT ;
Q_SIGNALS:
  void mode_reported(const string &mode) ;
  void mode_is_changing(const string &mode) ;
public Q_SLOTS:
  void init_request() ;
private Q_SLOTS:
  void request_finished(QDBusPendingCallWatcher *watcher) ;
  void dsme_state_signalled(const QString &new_mode) ;
} ;

#endif
