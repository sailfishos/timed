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
#include <arpa/inet.h>

#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>

#include <qmlog>

#include <timed/nanotime.h>

#include "xxd.h"

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

struct ntp_time_t
{
  uint32_t sec, frac ;
  ntp_time_t() { sec = frac = 0 ; }
  ntp_time_t(uint32_t s) { sec = s, frac = 0 ; }
  ntp_time_t(uint32_t s, uint32_t f) { sec = s, frac = f ; }
  static const uint32_t  epoch1900 = 2208988800u ; // seconds in years 1900..1969
  static const double nano_to_fraction = 4294967296e-9 ; // = 2^32 / 10^9
  static ntp_time_t from_nanotime(const nanotime_t &t)
  {
    uint32_t sec = (uint32_t)t.sec() + epoch1900 ; // overfolow condition: sec<=epoch1900
    uint32_t frac = t.nano() * nano_to_fraction + 0.5 ; // will not overflow for 0 <= nano < 10^9
    return ntp_time_t(sec, frac) ;
  }
  nanotime_t to_nanotime() const
  {
    nanotime_t res(sec-epoch1900, (uint32_t) (frac / nano_to_fraction + 0.5)) ;
    res.fix_overflow() ; // overflow:  2^31-1 --> 999999999.7 -> 1000000000
    return res ;
  }
  static ntp_time_t systime_now() { return from_nanotime(nanotime_t::systime_now()) ; }
} ;

struct ntp_short_t
{
  uint16_t sec, frac ;
  ntp_short_t() { sec = frac = 0 ; }
  ntp_short_t(uint16_t s) { sec = s, frac = 0 ; }
  ntp_short_t(uint16_t s, uint16_t f) { sec = s, frac = f ; }
  uint32_t to_uint32() const { return (sec<<16) | frac ; }
  static ntp_short_t from_uint32(uint32_t x) { return ntp_short_t(x>>16, x & 0xFFFF) ; }
  std::string str() { return nanotime_t(sec, frac/65536e-9).str() ; }
} ;

struct ntp_datagram_t
{
  short leap_indicator, version_number, mode ;
  uint8_t stratum ;
  int8_t poll, precision ;
  ntp_short_t root_delay, root_dispersion ;
  uint32_t reference_id ;
  ntp_time_t reference, origin, receive, transmit ;
  uint8_t *data ;
  unsigned length ;

  ntp_datagram_t() :
    leap_indicator(3), // unknown
    version_number(4),
    mode(3), // client
    stratum(0),
    poll(3),
    precision(-6),
    root_delay(1,0),
    root_dispersion(1,0),
    reference_id(0),
    reference(0,0),
    origin(0,0),
    receive(0,0),
    transmit(0,0)
  {
    data = NULL ;
    length = 0 ;
  }

  ~ntp_datagram_t()
  {
    delete data ;
  }
  void allocate(int len)
  {
    log_assert(data==NULL, "datagram buffer already allocated") ;
    length = len, data = new uint8_t[len] ;
  }
  void allocate_minimal()
  {
    allocate(48) ;
  }
  void write_uint32(uint32_t x, unsigned offset)
  {
    log_assert(data and length >= offset + 4, "out of bounds: 4 bytes requested, length=%d, offset=%d", length, offset) ;
    uint32_t big = htonl(x) ;
    memcpy(data+offset, &big, 4) ;
  }
  void write_timestamp_short(const ntp_short_t &x, unsigned offset)
  {
    write_uint32(x.to_uint32(), offset) ;
  }
  void write_timestamp(const ntp_time_t &x, unsigned offset)
  {
    write_uint32(x.sec, offset) ;
    write_uint32(x.frac, offset+4) ;
  }
  void write_fields()
  {
    // First word: LI:2, VN:3, Mode:3, stratum:8, poll:8, precision:8
    uint32_t word0 = (leap_indicator << 30) | (version_number << 27) | (mode << 24)
      | ((uint8_t)stratum << 16) | ((uint8_t)poll << 8) | ((uint8_t)precision) ;
    write_uint32(word0, 0x0) ;
    write_timestamp_short(root_delay, 0x4) ;
    write_timestamp_short(root_dispersion, 0x8) ;
    write_uint32(reference_id, 0xC) ;
    write_timestamp(reference, 0x10) ;
    write_timestamp(origin, 0x18) ;
    write_timestamp(receive, 0x20) ;
    write_timestamp(transmit, 0x28) ;
  }
  bool read_minimal(const QByteArray &data)
  {
    if (data.size()<48)
    {
      log_error("Datagram is too short: %d bytes", data.size()) ;
      return false ;
    }
    if (data.size()>48)
      log_warning("datagram is too large (%d bytes), using only first 48 bytes", data.size()) ;
    const uint8_t *p = reinterpret_cast<const uint8_t *> (data.data()) ;
    leap_indicator = (p[0] & 0300 ) >> 6 ;
    version_number = (p[0] & 0070 ) >> 3 ;
    mode = p[0] & 0007 ;
    stratum = p[1], poll = p[2], precision = p[3] ;
#define U32(i) ntohl(*(const uint32_t*)(p+(i)))
#define U3232(i) U32(i),U32((i)+4)
    root_delay = ntp_short_t::from_uint32(U32(4)) ;
    root_dispersion = ntp_short_t::from_uint32(U32(8)) ;
    reference_id = U32(0xC) ;
    reference = ntp_time_t(U3232(0x10)) ;
    origin = ntp_time_t(U3232(0x18)) ;
    receive = ntp_time_t(U3232(0x20)) ;
    transmit = ntp_time_t(U3232(0x28)) ;
    return true ; // XXX: do some check ???
  }
  void to_strings(std::vector<std::string> &res)
  {
    res.push_back(str_printf("LI=%d VN=%d MD=%d STR=%d POLL=%d PREC=%d DEL=%s DISP=%s REFID=%08X",
          leap_indicator, version_number, mode, stratum, poll, precision,
          root_delay.str().c_str(), root_dispersion.str().c_str(), reference_id)) ;
    res.push_back(str_printf("reference: %s", reference.to_nanotime().str().c_str())) ;
    res.push_back(str_printf("origin   : %s", origin   .to_nanotime().str().c_str())) ;
    res.push_back(str_printf("receive  : %s", receive  .to_nanotime().str().c_str())) ;
    res.push_back(str_printf("transmit : %s", transmit .to_nanotime().str().c_str())) ;
  }
} ;

#endif
