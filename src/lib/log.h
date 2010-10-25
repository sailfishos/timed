#ifndef LIBTIMED_LOG_H
#define LIBTIMED_LOG_H

#define QMLOG_DEFAULT_DISPATCHER (& Maemo::Timed::libtimed_logging_dispatcher)

#include <qmlog.h>

namespace Maemo
{
  namespace Timed
  {
    class libtimed_logging_dispatcher_t : public qmlog::slave_dispatcher_t
    {
    public:
      libtimed_logging_dispatcher_t() ;
    } ;
    extern libtimed_logging_dispatcher_t libtimed_logging_dispatcher ;
  }
}

#endif
