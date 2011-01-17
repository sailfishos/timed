#ifndef MAEMO_TIMED_CELLULAR_H
#define MAEMO_TIMED_CELLULAR_H

#include <QString>

#include "f.h"

#if F_CSD
#include <NetworkTime>
#include <NetworkOperator>
#endif

#include "timed/nanotime.h"

struct cellular_operator_t
{
  std::string mcc, mnc ;
  int mcc_value ; // =0, if mcc="", -1, else if mcc is not a number
  cellular_operator_t() ;
  cellular_operator_t(const std::string &mcc_s, const std::string &mnc_s) ;
#if F_CSD
  cellular_operator_t(const QString &mcc_s, const QString &mnc_s) ;
  cellular_operator_t(const Cellular::NetworkTimeInfo &cnti) ;
#endif
  std::string str() const ;
private:
  void parse_mcc(const char *) ;
} ;

struct cellular_time_t
{
  time_t value ;
  nanotime_t ts ;
  cellular_time_t() ;
#if F_CSD
  cellular_time_t(const Cellular::NetworkTimeInfo &cnti) ;
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
#if F_CSD
  cellular_offset_t(const Cellular::NetworkTimeInfo &cnti) ;
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
