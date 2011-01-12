#ifndef MAEMO_TIMED_CSD_H
#define MAEMO_TIMED_CSD_H

#include "f.h"

#if F_CELLULAR_QT

#include <string>

#include <NetworkTime>
#include <NetworkOperator>
using Cellular::NetworkTimeInfo ;

class Timed ;

struct csd_t : public QObject
{
  Q_OBJECT ;
  Timed *timed ;
  Cellular::NetworkTime *nt ;
  Cellular::NetworkOperator *op ;
  Q_INVOKABLE void csd_operator_q() ;
private Q_SLOTS:
  void csd_time_q(const NetworkTimeInfo &nti) ;
  void csd_time_s(const NetworkTimeInfo &nti) ;
  void csd_operator_s(const QString &mnc, const QString &mcc) ;
private:
  void process_csd_network_time_info(const NetworkTimeInfo &nti) ;
  void process_csd_network_operator(const QString &mcc, const QString &mnc) ;
  friend class com_nokia_time ; // these private functions can be used by dbus fake
public:
  csd_t(Timed *owner) ;
  static std::string csd_network_time_info_to_string(const NetworkTimeInfo &nti) ;
  virtual ~csd_t() ;
} ;

#endif//F_CELLULAR_QT

#endif//MAEMO_TIMED_CSD_H
