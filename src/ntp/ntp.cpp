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
#include <vector>
#include <string>
using namespace std ;

#include <QCoreApplication>

#include <qmlog>

#include <timed/nanotime.h>

#include "xxd.h"
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

#if 0
static char print_char(unsigned char ch)
{
  if (ch<0x20 or ch>0x7E)
    return '.' ;
  return ch ;
}
#endif

void ntp_receiver_t::process(const QByteArray &d, const QHostAddress &a, uint16_t port)
{
  log_notice("received %d bytes from %s:%d", d.size(), a.toString().toStdString().c_str(), port) ;
  vector<string> dump ;
  xxd(d.data(), d.size(), 8, dump) ;
  for (unsigned i=0; i<dump.size(); ++i)
    log_notice("Data: %s", dump[i].c_str()) ;
#if 0
  for (int k=0; k<d.size(); k+=8)
  {
    char buf[80] = "" ;
    int remains = d.size() - k ;
    int len = remains < 8 ? remains : 8 ;
    for (int ii=0, i; i=k+ii, ii<len; ++ii)
      sprintf(buf+strlen(buf), "%02X ", d.at(i)) ;
    for (int i=0; i<8-len; ++i)
      sprintf(buf+strlen(buf), "%2s ", "") ;
    sprintf(buf+strlen(buf), "| ") ;
    for (int ii=0, i; i=k+ii, ii<len; ++ii)
      sprintf(buf+strlen(buf), "%c", print_char(d.at(i))) ;
    for (int i=0; i<8-len; ++i)
      sprintf(buf+strlen(buf), " ") ;
    sprintf(buf+strlen(buf), " | %2x", k) ;
    log_notice("Data: %s", buf) ;
  }
#endif
}

void ntp_receiver_t::bind()
{
  socket -> bind(QHostAddress("0.0.0.0"), 123) ;
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

#if 0
int main(int ac, char **av)
{
  QCoreApplication a(ac, av) ;
  ntp_receiver_t S ;
  return a.exec() ;
}
#endif
