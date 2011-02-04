#include <string>
#include <vector>
#include <set>
using namespace std ;

#include "resolver.h"
#include "ntp.h"

resolver_t::resolver_t()
{
}

void resolver_t::add_name(const string &host)
{
  names.insert(host) ;
}

void resolver_t::start()
{
  for (set<string>::const_iterator it=names.begin(); it!=names.end(); ++it)
  {
    QString host(it->c_str()) ;
    // first try to parse it, may be we don't need DNS lookup
    QHostAddress a(host) ;
    if (not a.isNull())
    {
      emit address(a) ;
      continue ;
    }

    // starting DNS lookup
    lookup_t *lookup = new lookup_t(*it) ;
    QObject::connect(lookup, SIGNAL(address_received(QHostAddress)), this, SIGNAL(address(QHostAddress))) ;
    QObject::connect(lookup, SIGNAL(lookup_done(int)), this, SLOT(lookup_done(int)), Qt::QueuedConnection) ;
    int id = QHostInfo::lookupHost(host, lookup, SLOT(lookup_done(const QHostInfo &))) ;
    pending.insert(id) ;
  }
}

void resolver_t::lookup_done(int id)
{
  pending.erase(id) ;
  if (pending.empty())
    emit resolving_done() ;
}

#include <QCoreApplication>

int main(int ac, char **av)
{
  qmlog::enable() ;
  qmlog::stderr()->disable_fields(qmlog::Pid | qmlog::Name | qmlog::Timezone_Symlink) ;
  qmlog::stderr()->enable_fields(qmlog::Time_Micro) ;
  my_app a(ac, av) ;
  resolver_t R ;
  concentrator_t C(50, 900, 5) ;
  ntp_receiver_t N ;
  N.bind() ;
  for (int i=1; i<ac; ++i)
    R.add_name(av[i]) ;
  // QObject::connect(&R, SIGNAL(resolving_done()), &a, SLOT(quit_after_5sec())) ;
  QObject::connect(&R, SIGNAL(address(QHostAddress)), &C, SLOT(add_address(QHostAddress))) ;
  QObject::connect(&R, SIGNAL(resolving_done()), &C, SLOT(last_address())) ;
  QObject::connect(&C, SIGNAL(send_address(QHostAddress)), &a, SLOT(address(QHostAddress))) ;
  R.start() ;
  return a.exec() ;
}
