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
#ifndef MAEMO_TIMED_CELLULAR_H
#define MAEMO_TIMED_CELLULAR_H

#include <QString>

#include "f.h"

#if OFONO
#include "networktime.h"
#include "networkoperator.h"
#endif

#include "../lib/nanotime.h"

struct cellular_operator_t
{
  std::string mcc, mnc ;
  std::string alpha2 ;
  // int mcc_value ; // =0, if mcc="", -1, else if mcc is not a number
  cellular_operator_t() ;
  cellular_operator_t(const std::string &mcc_s, const std::string &mnc_s) ;
#if OFONO
  cellular_operator_t(const QString &mcc_s, const QString &mnc_s) ;
  cellular_operator_t(const NetworkTimeInfo &cnti);
#endif
  bool operator==(const cellular_operator_t &x) const ; // same mcc & mnc
  bool operator!=(const cellular_operator_t &x) const ; // mcc or mnc differ
  std::string id() const ; // like "310/07"
  std::string location() const ; // "FI" or "001/10" for invalid MCC
  bool known_mcc() const ; // location()==iso_3166_location()
  bool empty() const ;
  std::string str() const ;
private:
  void init() ;
} ;

struct cellular_time_t
{
  time_t value ;
  nanotime_t ts ;
  cellular_time_t() ;
#if OFONO
  cellular_time_t(const NetworkTimeInfo &cnti);
#endif
  bool is_valid() const { return (bool)value ; }
  std::string str() const ;
} ;

struct cellular_offset_t
{
  cellular_operator_t oper ;
  int offset ;
  int dst ;
  time_t timestamp ;
  bool sender_time ; // is timestamp received in the same NITZ package as UTC time
  cellular_offset_t() ;
#if OFONO
  cellular_offset_t(const NetworkTimeInfo &cnti);
#endif
  std::string str() const ;
  bool is_valid() const { return (bool)timestamp ; }
} ;

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

#endif//MAEMO_TIMED_CELLULAR_H
