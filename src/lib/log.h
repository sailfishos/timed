#ifndef LIBTIMED_LOG_H
#define LIBTIMED_LOG_H

#define LIBTIMED_LOGGING_DISPATCHER (& Maemo::Timed::libtimed_logging_dispatcher)

#ifndef QMLOG_DISPATCHER
#define QMLOG_DISPATCHER LIBTIMED_LOGGING_DISPATCHER
#endif

#include <qmlog>

namespace Maemo
{
  namespace Timed
  {
    class libtimed_logging_dispatcher_t : public qmlog::dispatcher_t
    {
    protected:
      void set_process_name(const std::string &new_name) ;
    public:
      libtimed_logging_dispatcher_t() ;
     ~libtimed_logging_dispatcher_t() { }
    } ;
    extern libtimed_logging_dispatcher_t libtimed_logging_dispatcher ;
  }
}

#endif
