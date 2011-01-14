#include <string>
#include <sstream>
using namespace std;

#include <pcrecpp.h>
#include <qmlog>

#if F_CSD
#include <NetworkTime>
#include <NetworkOperator>
#endif

#include "f.h"
#include "cellular.h"
#include "misc.h"

#if F_CSD
#include "csd.h"
#endif

cellular_operator_t::cellular_operator_t()
{
  mcc = mnc = "" ;
  mcc_value = 0 ;
}

#if F_CSD
cellular_operator_t::cellular_operator_t(const QString &mcc_s, const QString &mnc_s)
{
  mcc = mcc_s.toStdString(), mnc = mnc_s.toStdString() ;
  parse_mcc(mcc.c_str()) ;
}

cellular_operator_t::cellular_operator_t(const Cellular::NetworkTimeInfo &cnti)
{
  if (cnti.isValid())
    mcc = cnti.mcc().toStdString(), mnc = cnti.mnc().toStdString() ;
  parse_mcc(mcc.c_str()) ;
}
#endif

string cellular_operator_t::str() const
{
  ostringstream os ;
  os << "{mcc=" ;
  if (mcc_value>0)
    os << mcc_value ;
  else
    os << "'" << mcc << "'" ;
  os << ",mnc='" << mnc << "'}" ;
  return os.str() ;
}

void cellular_operator_t::parse_mcc(const char *p)
{
  static pcrecpp::RE integer = "(\\d+)" ;
  if (p[0]=='\0') //empty string
    mcc_value = 0 ;
  else if (not integer.FullMatch(mcc, &mcc_value))
    mcc_value = -1 ;
}

cellular_time_t::cellular_time_t() :
  value(0), ts(0)
{
  log_debug("constructed %s by default", str().c_str()) ;
}

#if F_CSD
cellular_time_t::cellular_time_t(const Cellular::NetworkTimeInfo &cnti) :
  value(0), ts(0)
{
  if (cnti.isValid() and cnti.dateTime().isValid())
  {
    value = cnti.dateTime().toTime_t() ;
    ts = nanotime_t::from_timespec (*cnti.timestamp()) ;
  }
  log_debug("constructed %s out of %s", str().c_str(), csd_t::csd_network_time_info_to_string(cnti).c_str()) ;
}
#endif

cellular_zone_t::cellular_zone_t() :
  offset(0), dst(-1), timestamp(0), sender_time(false)
{
  log_debug("constructed %s by default", str().c_str()) ;
}

#if F_CSD
cellular_zone_t::cellular_zone_t(const Cellular::NetworkTimeInfo &cnti) :
  oper(cnti),
  offset(0), dst(-1), timestamp(0), sender_time(false)
{
  log_debug() ;
  if (cnti.isValid())
  {
    offset = cnti.offsetFromUtc() ;

    // first of all check, if we can support this offset
    static const int offset_threshold = 15*3600 ; // 15 hours from Greenwich
    bool too_large = offset < -offset_threshold or offset_threshold < offset ;
    bool not_divisible = offset % (15*60) ;
    if (too_large or not_divisible)
    {
      log_error("GMT offset %d seconds is not supported", offset) ;
      return ;
    }

    dst = cnti.daylightAdjustment() ;

    if (cnti.dateTime().isValid() and cnti.dateTime().timeSpec()==Qt::UTC)
    {
      sender_time = true ;
      timestamp = cnti.dateTime().toTime_t() ;
    }
    else
    {
      // the exact moment of sending isn't clear, let's guess it
      nanotime_t monotonic_received = nanotime_t::from_timespec(*cnti.timestamp()) ;
      nanotime_t system_received = nanotime_t::systime_at_zero() + monotonic_received ;
      timestamp = system_received.sec() ;
    }
  }
  log_debug("constructed %s out of %s", str().c_str(), csd_t::csd_network_time_info_to_string(cnti).c_str()) ;
}
#endif

string cellular_zone_t::str() const
{
  ostringstream os ;
  if (is_valid())
  {
    os << "{offset=" << offset ;
    if (offset)
    {
      int offset_min = offset / 60 ;
      os << "=" << (offset_min<0 ? (offset_min=-offset_min, "-") : "+") ;
      int hour = offset_min / 60 ;
      os << hour ;
      if (int mins = offset_min % 60)
        os << str_printf(":%02d", mins) ;
      else
        os << "h" ;
    }
    os << ", dst=" ;
    if (dst<0)
      os << "n/a" ;
    else
      os << dst ;

    os << (sender_time ? "sender" : "receiver") << " time=" << timestamp << "=" << str_iso8601(timestamp) ;
    os << "}" ;
  }
  else
    os << "{cellular_zone_t::invalid}" ;
  return os.str() ;
}
