#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#include "../common/log.h"

#include "notification.h"

static const time_t timestamp = 2147483645; // three seconds before apocalypse

#ifndef TFD_TIMER_CANCELON_SET
#define TFD_TIMER_CANCELON_SET (1 << 1)
#endif

kernel_notification_t::kernel_notification_t(QObject *parent)
    : QObject(parent)
{
    fd = -1;
    timerfd = NULL;
    is_running = false;
}

kernel_notification_t::~kernel_notification_t()
{
    if (fd >= 0)
        close(fd);
    if (timerfd)
        delete timerfd;
}

void kernel_notification_t::start()
{
    log_debug();
    if (is_running) {
        log_abort("already running while kernel_notification_t:start() called");
        return;
    }
    is_running = true;
    time_at_zero = nanotime_t::systime_at_zero();

    fd = timerfd_create(CLOCK_REALTIME, O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        timerfd = NULL;
        log_error("timerfd_create failed: %m");
        log_notice("kernel system time change notification not available");
    } else {
        timerfd = new QSocketNotifier(fd, QSocketNotifier::Read);
        QObject::connect(timerfd, SIGNAL(activated(int)), this, SLOT(ready_to_read(int)));
    }
    if (fd < 0) {
        log_warning("can't start kernel system time change notification (fd=%d)", fd);
        return;
    }
    struct itimerspec its;
    memset(&its, 0, sizeof(its));
    its.it_value.tv_sec = timestamp;
    int res = timerfd_settime(fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCELON_SET, &its, NULL);
    if (res < 0)
        log_error("can't start system change notification: %m");
    else
        log_notice("started kernel system time change notification watcher (time_at_zero=%s)",
                   time_at_zero.str().c_str());
}

void kernel_notification_t::stop()
{
    is_running = false;
    log_debug();
    if (timerfd) {
        delete timerfd;
        timerfd = NULL;
    }
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }

#if LOG_LEVEL >= LOG_NOTICE
    nanotime_t new_time_at_zero = nanotime_t::systime_at_zero();
    nanotime_t drift = new_time_at_zero - time_at_zero;
    log_notice("stopped kernel system time change notification watcher (time_at_zero=%s, drift=%s)",
               time_at_zero.str().c_str(),
               drift.str().c_str());
#endif
}

void kernel_notification_t::ready_to_read(int fd)
{
    if (fd != this->fd) {
        log_error("wrong descriptor in kernel_notification_t::ready_to_read (expected=%d, got=%d)",
                  this->fd,
                  fd);
        return;
    }
    uint64_t counter;
    int res = read(fd, &counter, sizeof(counter));
    if (res < 0 and errno == ECANCELED) // that's it!
    {
        nanotime_t jump = nanotime_t::systime_at_zero() - time_at_zero;
        log_notice("kernel system time change notification detected, jump: %s", jump.str().c_str());

        stop();

        // The time may have changed because of a wake up from suspend.
        // The QTimer tracking next alarm trigger time may ignore time spent in suspend,
        // restarting the alarm timer will recalculate the trigger time.
        emit restart_alarm_timer();

        // Do not react to small changes in time, which may be a result of
        // timers being a bit off after waking up from suspend.
        //
        // Must use nanotime_t operators when determining that the jump is > 2 seconds,
        // the operators in nanotime_t do various adjustments to the values stored internally,
        // whereas the getters like nanotime_t::sec() do not.
        if (jump <= nanotime_t(-2) || jump >= nanotime_t(2)) {
            log_debug("system time change > 0 secs (%i), emitting system_time_changed", jump.s);
            emit system_time_changed(jump);
        }
        start();
    } else if (res <= 0)
        log_error("unexpected read error in kernel_notification_t::ready_to_read: %m");
    else
        log_error("unexpected successful read in kernel_notification_t::ready_to_read: res=%d, "
                  "counter=%llu",
                  res,
                  static_cast<unsigned long long int>(counter));
}
