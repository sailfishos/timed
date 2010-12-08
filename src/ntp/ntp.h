#ifndef MAEMO_TIMED_NTP_NTP_H
#define MAEMO_TIMED_NTP_NTP_H

#include <stdint.h>

#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>

struct ntp_receiver_t : public QObject
{
  Q_OBJECT ;
public:
  QUdpSocket *socket ;
  void bind() ;
  void process(const QByteArray &, const QHostAddress &, uint16_t port) ;
  ntp_receiver_t() ;
  virtual ~ntp_receiver_t() ;
public slots:
  void read_datagram() ;
} ;


#endif
