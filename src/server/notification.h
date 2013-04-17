#ifndef TIMED_NOTIFICATION_H
#define TIMED_NOTIFICATION_H

#include <QObject>
#include <QSocketNotifier>

#include "../lib/nanotime.h"

struct kernel_notification_t : public QObject
{
  QSocketNotifier *timerfd ;
  int fd ;

  bool is_running ;
  nanotime_t time_at_zero ;

  kernel_notification_t() ;
 ~kernel_notification_t() ;
  void start() ;
  void stop() ;
  Q_OBJECT ;
private Q_SLOTS:
  void ready_to_read(int fd) ;
Q_SIGNALS:
  void system_time_changed(const nanotime_t &) ;
} ;


#endif//TIMED_NOTIFICATION_H
