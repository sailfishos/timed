#include <QCoreApplication>

#include <qmlog>

#include "ntp.h"

void ntp_receiver_t::read_datagram()
{
  while(socket->hasPendingDatagrams())
  {
    QByteArray data(socket->pendingDatagramSize(), '\0') ;
    QHostAddress sender ;
    uint16_t port ;

    socket->readDatagram(data.data(), data.size(), &sender, &port) ;
    process(data, sender, port) ;
  }
}

void ntp_receiver_t::process(const QByteArray &d, const QHostAddress &a, uint16_t port)
{
  log_debug("received %d bytes from %s:%d", d.size(), a.toString().toStdString().c_str(), port) ;
}

void ntp_receiver_t::bind()
{
  socket -> bind(QHostAddress::LocalHost, 123) ;
}

ntp_receiver_t::ntp_receiver_t()
{
  socket = new QUdpSocket(this) ;
  QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(read_datagram())) ;
}

ntp_receiver_t::~ntp_receiver_t()
{
  delete socket ;
}

int main(int ac, char **av)
{
  QCoreApplication a(ac, av) ;
  ntp_receiver_t S ;
  return a.exec() ;
}
