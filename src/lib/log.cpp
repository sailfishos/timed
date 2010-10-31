#include <string>
using namespace std ;

#include "log.h"

Maemo::Timed::libtimed_logging_dispatcher_t::libtimed_logging_dispatcher_t()
//  : qmlog::slave_dispatcher_t("libtimed")
{
  qmlog::log_file *varlog = new qmlog::log_file("/var/log/libtimed.log", qmlog::Full, this) ;

  varlog->enable_fields(qmlog::Monotonic_Milli | qmlog::Time_Milli) ;

  set_process_name(qmlog::process_name()) ;

  // log_critical("blah (in libtimed, pid=%d)", getpid()) ;
}

void Maemo::Timed::libtimed_logging_dispatcher_t::set_process_name(const std::string &new_name)
{
  string name = "libtimed" ;
  if (not new_name.empty())
    name += "|", name += new_name ;
  dispatcher_t::set_process_name(name) ;
}

Maemo::Timed::libtimed_logging_dispatcher_t Maemo::Timed::libtimed_logging_dispatcher ;
