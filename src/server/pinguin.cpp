#include "pinguin.h"
#include "timed.h"

void pinguin_t::timeout()
{
  if(!needed) // could rarely happen
    return ;
  counter ++ ;
  log_info("pinging voland activation service, try %d out of %d", counter, max_num) ;
  ping() ;
  if(counter<max_num)
    timer->start() ;
}

void pinguin_t::voland_needed()
{
  log_debug() ;
  needed = true ;
  if(!timer->isActive())
    timeout() ;
}

void pinguin_t::voland_registered()
{
  log_debug() ;
  timer->stop() ;
  needed = false ;
  counter = 0 ;
}

pinguin_t::pinguin_t(unsigned p, unsigned n, Timed *timed) :
  QObject(timed), owner(timed), max_num(n), counter(0)
{
  timer = new simple_timer(p) ;
  needed = false ;
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout())) ;
}

pinguin_t::~pinguin_t()
{
  delete timer ;
}

void pinguin_t::ping()
{
  log_debug() ;
  const char *serv = Maemo::Timed::Voland::/*activation_*/service() ;
  const char *path = Maemo::Timed::Voland::/*activation_*/objpath() ;
  const char *ifac = Maemo::Timed::Voland::/*activation_*/interface() ;
  const char *meth = "pid" ;
  QDBusMessage mess = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
  QDBusConnection conn = owner->session_bus ;
  if(conn.send(mess))
    log_info("the 'pid' request sent asyncronosly") ;
  else
    log_error("Can't send the 'pid' request: %s", conn.lastError().message().toStdString().c_str()) ;
}

