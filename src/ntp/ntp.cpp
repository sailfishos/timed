/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2011 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
**   Author: Simo Piiroinen <simo.piiroinen@nokia.com>                    **
**   Author: Victor Portnov <ext-victor.portnov@nokia.com>                **
**                                                                        **
**     This file is part of Timed                                         **
**                                                                        **
**     Timed is free software; you can redistribute it and/or modify      **
**     it under the terms of the GNU Lesser General Public License        **
**     version 2.1 as published by the Free Software Foundation.          **
**                                                                        **
**     Timed is distributed in the hope that it will be useful, but       **
**     WITHOUT ANY WARRANTY;  without even the implied warranty  of       **
**     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               **
**     See the GNU Lesser General Public License  for more details.       **
**                                                                        **
**   You should have received a copy of the GNU  Lesser General Public    **
**   License along with Timed. If not, see http://www.gnu.org/licenses/   **
**                                                                        **
***************************************************************************/
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
