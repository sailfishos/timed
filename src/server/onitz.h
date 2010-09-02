/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
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
#if ! __MEEGO__
#define USE_CELLULAR_QT 1
#endif

#ifndef ONITZ_H
#define ONITZ_H

#include <string>
#include <sstream>
using namespace std ;

#include <QObject>
#include <QDateTime>

#if USE_CELLULAR_QT
#  include <NetworkTime>
#  include <NetworkOperator>
using Cellular::NetworkTimeInfo ;
#endif


#include "timed/nanotime.h"

#include "misc.h"
#include "log.h"

struct cellular_info_t
{
  bool flag_offset, flag_time, flag_dst, flag_mcc, flag_mnc ;

  nanotime_t time_at_zero_value, timestamp_value ;
  int offset_value, dst_value ;
  int mcc_value ;
  string mnc_value ;

  bool has_offset() const { return flag_offset ; }
  bool has_time() const { return flag_time ; }
  bool has_dst() const { return flag_dst ; }
  bool has_mcc() const { return flag_mcc ; }
  bool has_mnc() const { return flag_mnc ; }

  nanotime_t timestamp() const { return timestamp_value ; }
  int offset() const { log_assert(has_offset()) ; return offset_value ; }
  nanotime_t time_at_zero() const { log_assert(has_time()) ; return time_at_zero_value ; }
  int dst() const { log_assert(has_dst()) ; return dst_value ; }
  int mcc() const { log_assert(has_mcc()) ; return mcc_value ; }
  string mnc() const { log_assert(has_mnc()) ; return mnc_value ; }

  cellular_info_t()
  {
    flag_offset = flag_time = flag_dst = flag_mcc = flag_mnc = false ;
  } ;

  string to_string() const
  {
    ostringstream os ;
    os << str_printf("{ ts=%d.%09u", timestamp().sec(), timestamp().nano()) ;
    if(has_mcc())
      os << ", mcc=" << mcc() ;
    if(has_mnc())
      os << ", mnc='" << mnc() << "'" ;
    if(has_offset())
      os << ", offset=" << offset() ;
    if(has_time())
      os << ", time_at_zero=" << str_printf("%d.%09u", time_at_zero().sec(), time_at_zero().nano()) ;
    if(has_dst())
      os << ", dst=" << dst() ;
    os << "}" ;
    return os.str() ;
  }
} ;

#if 0
namespace Cellular
{
  struct NetworkTimeInfo
  {
    QDateTime x_dateTime;
    int x_offsetFromUtc;
    int x_daylightAdjustment;
    qlonglong x_timestampSeconds, x_timestampNanoSeconds ;
    QString x_mcc, x_mnc ;

    QDateTime dateTime() const { return x_dateTime; }
    int offsetFromUtc() const { return x_offsetFromUtc; }
    int daylightAdjustment() const { return x_daylightAdjustment; }

    qlonglong timestampSeconds() const { return x_timestampSeconds ; }
    qlonglong timestampNanoSeconds() const { return x_timestampNanoSeconds ; }
    QString mobileCountryCode() const { return x_mcc ; }
    QString mobileNetworkCode() const { return x_mnc ; }
    bool isValid() const { return true ; }
    QString toString() const
    {
      // log_debug() ;
      QString res = "INVALID" ;
      // log_debug() ;
      if(isValid())
      {
        // log_debug() ;
        res = dateTime().toString(Qt::ISODate) ;
        // log_debug() ;
        res += QString(" off: %1 dst: %2").arg(offsetFromUtc()).arg(daylightAdjustment()) ;
        // log_debug() ;
        res += QString(" timestamp: %1/%2").arg(timestampSeconds()).arg(timestampNanoSeconds()) ;
        // log_debug() ;
        res += QString(" mcc: '%1' mnc: '%2'").arg(mobileCountryCode()).arg(mobileNetworkCode()) ;
        // log_debug() ;
      }
      // log_debug() ;
      return res ;
    }
  } ;
}
#endif

struct cellular_handler : public QObject
{
  Q_OBJECT ;
#if USE_CELLULAR_QT
  Cellular::NetworkTime *cnt ;
  Cellular::NetworkOperator *cop ;
#endif
  cellular_handler() ;
#if 0
  Q_INVOKABLE void send_signal() ;
#endif
  Q_INVOKABLE void emulate_operator_signal() ;
signals:
  void cellular_data_received(const cellular_info_t &) ;
public:
  static cellular_handler *object() ;
  void fake_nitz_signal(int mcc, int offset, int time, int dst) ;
#if 0
  void invoke_signal() ;
#endif
private slots:
#if 0
  void old_nitz_signal(QDateTime dt, int timezone, int dst) ;
#endif
#if USE_CELLULAR_QT
  void new_nitz_signal(const NetworkTimeInfo &) ; // { log_assert(false, "to be implemented") ; }
#endif
  void new_operator(const QString &mnc, const QString &mcc) ;
} ;

#endif
