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
#include "f.h"

#ifndef ONITZ_H
#define ONITZ_H

#include <string>
#include <sstream>
using namespace std ;

#include <QObject>
#include <QDateTime>

#include "../common/log.h"

#if OFONO
# include "networktime.h"
# include "networkoperator.h"
# include "networktimeinfo.h"
#endif


#include "../lib/nanotime.h"

#include "misc.h"

#include "cellular.h"
#if 0
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
#endif

struct cellular_handler : public QObject
{
  Q_OBJECT ;
private:
  static cellular_handler *static_object ;
  virtual ~cellular_handler() ;
#if 0
#if OFONO
  NetworkTime *cnt;
  NetworkOperator *cop;
#endif
#endif
  cellular_handler() ;
#if 0
  Q_INVOKABLE void emulate_operator_signal() ;
#endif

signals:
#if 0
  void cellular_data_received(const cellular_info_t &) ;
#endif

public:
  static cellular_handler *object() ;
  static void uninitialize() ;
  void fake_nitz_signal(int mcc, int offset, int time, int dst) ;
public slots:
#if 0
#if OFONO
  void new_nitz_signal(const NetworkTimeInfo &) ; // { log_assert(false, "to be implemented") ; }
#endif
  void new_operator(const QString &mnc, const QString &mcc) ;
#endif
} ;

#endif
