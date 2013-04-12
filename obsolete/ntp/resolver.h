#ifndef MAEMO_TIMED_NTP_RESOLVER_H
#define MAEMO_TIMED_NTP_RESOLVER_H

#include <stdint.h>

#include <string>
#include <vector>
#include <set>

#include <QObject>
#include <QHostAddress>
#include <QHostInfo>
#include <QTimer>
#include <QCoreApplication>
#include <QUdpSocket>

#include <qmlog>

#include "ntp.h"
#include "xxd.h"

template<typename value_t>
value_t uniform(std::vector<value_t> &array)
{
  // TODO: srandom
  if (unsigned N=array.size())
  {
    unsigned i = random() % N ;
    value_t result = array[i] ;
    if (i!=N-1)
      array[i] = array[N-1] ;
    array.resize(N-1) ;
    return result ;
  }
  return value_t() ;
}

struct lookup_t : public QObject
{
  std::string hostname ;
  lookup_t(const std::string &h) : hostname(h) { }
  Q_OBJECT ;
public Q_SLOTS:
  void lookup_done(const QHostInfo &info)
  {
    if (info.error() != QHostInfo::NoError)
      log_error("lookup (id=%d) failed for host '%s': %s", info.lookupId(), hostname.c_str(), info.errorString().toStdString().c_str()) ;

    QList<QHostAddress> list = info.addresses() ;
    for (QList<QHostAddress>::iterator it = list.begin(); it != list.end(); ++it)
    {
      // log_debug("address for host '%s': <%s>", hostname.c_str(), it->toString().toStdString().c_str()) ;
      emit address_received(*it) ;
    }

    emit lookup_done(info.lookupId()) ;
  }
Q_SIGNALS:
  void lookup_done(int lookup_id) ;
  void address_received(QHostAddress a) ;
} ;

struct resolver_t : public QObject
{
  std::set<int> pending ;
  std::set<std::string> names ;

  resolver_t() ;
  void add_name(const std::string &host) ;
  void start() ;
  Q_OBJECT ;
public Q_SLOTS:
  void lookup_done(int lookup_id) ;
Q_SIGNALS:
  void address(QHostAddress a) ;
  void resolving_done() ;
} ;

struct concentrator_t : public QObject
{
  int primary_pause, secondary_pause, first_bunch ;
  int received, sent ;
  std::vector<QHostAddress> queue ;
  std::set<std::string> known ;
  QTimer *timer ;
  enum { WAIT, BUNCH, REGULAR, EMPTY } state ;
  concentrator_t(int p1, int p2, int b, QObject *p=NULL) : QObject(p)
  {
    primary_pause = p1, secondary_pause = p2, first_bunch = b ;
    received = sent = 0 ;
    timer = new QTimer ;
    timer->setSingleShot(true) ;
    state = WAIT ;
    connect(timer, SIGNAL(timeout()), this, SLOT(send())) ;
  }
  Q_OBJECT ;
public Q_SLOTS:
  void send()
  {
    timer->stop() ;
    if (queue.empty())
    {
      state = EMPTY ;
      return ;
    }
    QHostAddress a = uniform(queue) ;
    emit send_address(a) ;
    if (++sent==first_bunch)
      state = REGULAR ;
    timer->start(state==BUNCH ? primary_pause : secondary_pause) ;
  }
  void add_address(QHostAddress ip)
  {
    std::string ip_string = ip.toString().toStdString() ;
    if (known.count(ip_string)>0)
      return ;
    known.insert(ip_string) ;
    queue.push_back(ip) ;
    log_notice("added '%s' to queue, %d elemants now", ip_string.c_str(), queue.size()) ;
    if (state==WAIT and (int)queue.size()==first_bunch)
      state = BUNCH ;
    if (not timer->isActive() and state!=WAIT)
      timer->start(state==BUNCH ? primary_pause : state==REGULAR ? secondary_pause : 0) ;
  }
  void last_address()
  {
    if (state==WAIT)
    {
      state = BUNCH ;
      timer->start(primary_pause) ;
    }
  }
Q_SIGNALS:
  void send_address(QHostAddress ip) ;
} ;

struct ntp_request_t : public QObject
{
  QHostAddress server ;
  QUdpSocket *socket ;
  enum { NEW, RUNNING, DONE, FAILED } state ;
  nanotime_t value ; // time at zero according to NTP server response
  nanotime_t sent ; // host's system time while sending request
  int period ;
  ntp_datagram_t reply ;
  ntp_request_t(QHostAddress ip, int timeout_ms, QObject *parent=NULL) : QObject(parent)
  {
    server = ip ;
    socket = new QUdpSocket(this) ;
    socket->bind() ; // default IP=0.0.0.0, port=0
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(read_datagram())) ;
    period = timeout_ms ;
    state = NEW ;
  }
  void start()
  {
    log_assert(state==NEW, "can't start a request unless in NEW state") ;
    ntp_datagram_t D ;
    D.allocate_minimal() ;
    D.transmit = ntp_time_t::systime_now() ;
    D.write_fields() ;
    nanotime_t t1 = nanotime_t::systime_now() ;
    qint64 res = socket->writeDatagram(reinterpret_cast<const char *>(D.data), D.length, server, 123) ;
    nanotime_t t2 = nanotime_t::systime_now() ;
    if (res==(qint64)D.length)
    {
      log_notice("sent %d bytes to server %s", (int)res, server.toString().toStdString().c_str()) ;
      state = RUNNING ;
      nanotime_t half_delta = (t2-t1).div2() ;
      sent = t1 + half_delta ;
      QTimer::singleShot(period, this, SLOT(timeout())) ;
    }
    else
    {
      log_error("failed to send datagram to server %s: %s", server.toString().toStdString().c_str(), socket->errorString().toStdString().c_str()) ;
      state = FAILED ;
      emit finished(this) ;
    }
  }
  Q_OBJECT ;
public Q_SLOTS:
  void read_datagram()
  {
    nanotime_t now = nanotime_t::systime_now() ;
    while(socket->hasPendingDatagrams())
    {
      QByteArray data(socket->pendingDatagramSize(), '\0') ;
      QHostAddress sender ;
      uint16_t port ;
      socket->readDatagram(data.data(), data.size(), &sender, &port) ;
      log_notice("(%s) %s %d bytes from %s:%d", server.toString().toStdString().c_str(), (state==RUNNING?"received":"ignored"), data.size(), sender.toString().toStdString().c_str(), port) ;
      if (state!=RUNNING)
        continue ;

      if (reply.read_minimal(data))
      {
        std::vector<std::string> dump ;
#if 0
        xxd(data.data(), data.size(), 8, dump) ;
#endif
        reply.to_strings(dump) ;
        for (unsigned i=0; i<dump.size(); ++i)
          log_notice("%s", dump[i].c_str()) ;
        log_notice("<sent it>  %s", sent.str().c_str()) ;
        log_notice("<receive>  %s", now.str().c_str()) ;
        nanotime_t d1 = now-reply.transmit.to_nanotime(), d2 = sent-reply.receive.to_nanotime() ;
        nanotime_t delta = (d1+d2).div2() ;
        value = nanotime_t::systime_at_zero() - delta ;
        log_notice("< delta >  %s", delta.str().c_str()) ;
        state = DONE ;
      }
      else
        state = FAILED ;
      emit finished(this) ;
    }
  }
  void timeout()
  {
    if (state!=RUNNING)
      return ;
    state = FAILED ;
    log_error("(%s): no response after %dms, giving up", server.toString().toStdString().c_str(), period) ;
    emit finished(this) ;
  }
Q_SIGNALS:
  void finished(ntp_request_t*) ;
} ;

struct my_app : public QCoreApplication
{
  my_app(int ac, char **av) : QCoreApplication(ac, av) { }
  Q_OBJECT ;
  std::set<ntp_request_t*> pending ;
public Q_SLOTS:
  void address(QHostAddress ip)
  {
    log_notice("starting request to address: %s", ip.toString().toStdString().c_str()) ;
    ntp_request_t *r = new ntp_request_t(ip, 1500, this) ;
    pending.insert(r) ;
    r->start() ;
  }
  void request_done(ntp_request_t *)
  {
  }
  void quit_after_5sec()
  {
    log_notice("5 seconds to quit....") ;
    QTimer::singleShot(5000, this, SLOT(quit())) ;
  }
} ;


#endif//MAEMO_TIMED_NTP_RESOLVER_H

