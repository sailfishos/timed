#include "log.h"

Maemo::Timed::libtimed_logging_dispatcher_t::libtimed_logging_dispatcher_t()
  : qmlog::slave_dispatcher_t("libtimed")
{
  new qmlog::log_syslog(qmlog::Full, this) ;
  new qmlog::log_stderr(qmlog::Full, this) ;
  qmlog::abstract_log_t *varlog = new qmlog::log_file("/var/log/libtimed.log", qmlog::Full, this) ;

  varlog->enable_fields(qmlog::All_Fields) ;
  varlog->disable_fields(qmlog::Monotonic_Mask | qmlog::Time_Mask) ;
  varlog->enable_fields(qmlog::Monotonic_Milli | qmlog::Time_Milli) ;

  // log_critical("blah (in libtimed, pid=%d)", getpid()) ;
}

Maemo::Timed::libtimed_logging_dispatcher_t Maemo::Timed::libtimed_logging_dispatcher ;
