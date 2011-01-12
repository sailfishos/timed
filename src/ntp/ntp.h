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
