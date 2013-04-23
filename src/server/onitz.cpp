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

#include <sys/time.h>

#include <pcrecpp.h>

#include "../common/log.h"

#include "onitz.h"
#include "misc.h"

cellular_handler::cellular_handler()
{
#if 0
#if OFONO
  cnt = new NetworkTime;
  cop = new NetworkOperator;
  const char *signal1 = SIGNAL(timeInfoChanged(const NetworkTimeInfo &)) ;
  const char *signal2 = SIGNAL(timeInfoQueryCompleted(const NetworkTimeInfo &)) ;
  const char *my_slot = SLOT(new_nitz_signal(const NetworkTimeInfo &)) ;
  int res1 = QObject::connect(cnt, signal1, this, my_slot) ;
  int res2 = QObject::connect(cnt, signal2, this, my_slot) ;
  if(res1 && res2)
    log_info("succesfully connected to cellular time signals") ;
  else
    log_debug("connection to cellular time signals failed: %s %s", res1?"":signal1, res2?"":signal2) ;

  const char *signal_op = SIGNAL(operatorChanged(const QString &, const QString &)) ;
  const char *slot_op = SLOT(new_operator(const QString &, const QString &)) ;
  int res_op = QObject::connect(cop, signal_op, this, slot_op) ;
  if(res_op)
    log_info("succesfully connected to cellular operator signal") ;
  else
    log_debug("connection to cellular operator signal failed") ;

  cnt->queryTimeInfo() ;
  QMetaObject::invokeMethod(this, "emulate_operator_signal", Qt::QueuedConnection) ;
#endif
#endif
}

#if 0
void cellular_handler::emulate_operator_signal()
{
#if OFONO
  log_debug() ;
  new_operator(cop->mnc(), cop->mcc()) ;
  log_debug() ;
#endif
}
#endif

cellular_handler *cellular_handler::static_object = NULL ;

cellular_handler *cellular_handler::object()
{
  if(static_object==NULL)
    static_object = new cellular_handler ;
  return static_object ;
}

void cellular_handler::uninitialize()
{
  if(static_object==NULL)
    return ;
  delete static_object ;
  static_object = NULL ;
}

cellular_handler::~cellular_handler()
{
#if 0
  delete cop ;
  delete cnt ;
#endif
}

#if 0
void cellular_handler::fake_nitz_signal(int mcc, int offset, int time, int dst)
{
  log_debug("fake nitz requested: mcc=%d offset=%d, time=%d, dst=%d", mcc, offset, time, dst) ;
  cellular_info_t ci ;

  ci.timestamp_value = nanotime_t::monotonic_now() ;

  ci.flag_offset = true ;
  ci.offset_value = offset/*(15*60)*/ ;

  ci.flag_time = true ;
  ci.time_at_zero_value = nanotime_t::from_time_t((time_t)time) - ci.timestamp_value ;

  ci.flag_dst = ! (dst<0) ;
  ci.dst_value = dst ;

  ci.flag_mcc = true ;
  ci.mcc_value = mcc ;

  ci.flag_mnc = true ;
  ci.mnc_value = "fake" ;

  log_info("emitting fake nitz signal: ci=%s", ci.to_string().c_str()) ;
  emit cellular_data_received(ci) ;
  log_debug() ;
}
#endif

#if 0
#if OFONO
void cellular_handler::new_nitz_signal(const NetworkTimeInfo &cnti)
{
  log_debug() ;
  cellular_info_t ci ;

  // First copy the time stamp.
  ci.timestamp_value = nanotime_t::from_timespec(*cnti.timestamp()) ;

  // 1. Offset of the timezone.
  // Invalid means '-1' probably ?
  // Anyway it should divisible by a 15 mins unit
  int timezone = cnti.offsetFromUtc() ;
  if(timezone != -1 && timezone % (15*60) == 0)
  {
    ci.flag_offset = true ;
    ci.offset_value = timezone /* (15*60) */ ;

  // 2. UTC time, only possible, of the timezone offset is valid

    const QDateTime &dt = cnti.dateTime() ;
    if(dt.isValid() && dt.timeSpec()==Qt::UTC)
    {
      ci.flag_time = true ;
      nanotime_t sent = nanotime_t::from_time_t(dt.toTime_t()) ;
      ci.time_at_zero_value = sent - ci.timestamp_value ;

  // 3. DST flag, only possible if 1&2 above are given

      int dst = cnti.daylightAdjustment() ;
      if(dst!=-1) // '-1' is the 'invalid' value
      {
        ci.flag_dst = true ;
        ci.dst_value = dst ; // in hours
      }
    }
  }
  else
    log_error("invalid timezone=%d value from cellular daemon", timezone) ;

  // Now the network information
  QString mcc = cnti.mcc() ;
  QString mnc = cnti.mnc() ;

  // MCC is being sent as a string (why?)
  // Thus we have to convert it to an integer

  if(!mcc.isEmpty())
  {
    string mcc_str = string_q_to_std(mcc) ;
    int mcc_dec ;
    static pcrecpp::RE integer = "(\\d+)" ;
    if(integer.FullMatch(mcc_str, &mcc_dec))
    {
      ci.flag_mcc = true ;
      ci.mcc_value = mcc_dec ;

      // If we have a valid MCC, let's check for MNC
      if(!mnc.isEmpty())
      {
        ci.flag_mnc = true ;
        ci.mnc_value = string_q_to_std(mnc) ;
      }
    }
    else
      log_error("invalid mcc value from cellular daemon: '%s'", mcc.toStdString().c_str()) ;
  }

  log_debug() ;
  emit cellular_data_received(ci) ;
  log_debug() ;
}
#endif
#endif

#if 0
void cellular_handler::new_operator(const QString &mnc, const QString &mcc)
{
  log_debug("mnc='%s', mcc='%s'", string_q_to_std(mnc).c_str(), string_q_to_std(mcc).c_str()) ;
  cellular_info_t ci ;

  ci.flag_mcc = true ;

  string mcc_str = string_q_to_std(mcc) ;
  if(!mcc_str.empty())
  {
    static pcrecpp::RE integer = "(\\d+)" ;
    int mcc_dec ;
    if(integer.FullMatch(mcc_str, &mcc_dec))
      ci.mcc_value = mcc_dec ;
    else
    {
      log_error("invalid mcc='%s' received", mcc_str.c_str()) ;
      return ;
    }
  }
  else
    ci.mcc_value = 0 ;

  ci.flag_mnc = true ;
  ci.mnc_value = string_q_to_std(mnc) ;

  log_debug() ;
  emit cellular_data_received(ci) ;
  log_debug() ;
}
#endif
