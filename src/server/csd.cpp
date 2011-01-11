#include "f.h"

#include "timed.h"

#include "csd.h"

#if F_CELLULAR_QT

#include <NetworkTime>
#include <NetworkOperator>

csd_t::csd_t(Timed *owner)
{
  timed = owner ;
  nt = new Cellular::NetworkTime ;
  op = new Cellular::NetworkOperator ;
  static const char *time_signal1 = SIGNAL(timeInfoChanged(const NetworkTimeInfo &)) ;
  static const char *time_signal2 = SIGNAL(timeInfoQueryCompleted(const NetworkTimeInfo &)) ;
  static const char *time_slot = SLOT(csd_network_time_info(const NetworkTimeInfo &)) ;
  bool res1 = QObject::connect(nt, time_signal1, this, time_slot) ;
  bool res2 = QObject::connect(nt, time_signal2, this, time_slot) ;

  if(res1 && res2)
    log_info("succesfully connected to csd time signals") ;
  else
    log_error("connection to cellular csd signals failed: %s %s", res1?"":time_signal1, res2?"":time_signal2) ;

  static const char *operator_signal = SIGNAL(operatorChanged(const QString &, const QString &)) ;
  static const char *operator_slot = SLOT(csd_network_operator(const QString &, const QString &)) ;
  int res_op = QObject::connect(op, operator_signal, this, operator_slot) ;
  if(res_op)
    log_info("succesfully connected to csd network operator signal") ;
  else
    log_error("connection to csd network operator signal failed") ;

  nt->queryTimeInfo() ;
  QMetaObject::invokeMethod(this, "initial_csd_operator_query", Qt::QueuedConnection) ;
}

csd_t::~csd_t()
{
  delete nt ;
  delete op ;
}

void csd_t::initial_csd_operator_query()
{
  QString mcc = op->mcc(), mnc = op->mnc() ;
  log_notice("CSD::initial_csd_operator_query {mcc='%s', mnc='%s'}", mcc.toStdString().c_str(), mnc.toStdString().c_str()) ;
  process_csd_network_operator(mcc, mnc) ;
}

void csd_t::csd_network_operator(const QString &mnc, const QString &mcc)
{
  log_notice("CSD::csd_network_operator {mcc='%s', mnc='%s'}", mcc.toStdString().c_str(), mnc.toStdString().c_str()) ;
  process_csd_network_operator(mcc, mnc) ;
}

void csd_t::csd_network_time_info(const Cellular::NetworkTimeInfo &nti)
{
  log_notice("CSD::csd_network_time_info %s", csd_network_time_info_to_string(nti).c_str()) ;
  process_csd_network_time_info(nti) ;
}

void csd_t::process_csd_network_operator(const QString &mcc, const QString &mnc)
{
  timed->nitz_object->new_operator(mnc, mcc) ;
}

void csd_t::process_csd_network_time_info(const Cellular::NetworkTimeInfo &nti)
{
  timed->nitz_object->new_nitz_signal(nti) ;
}

string csd_t::csd_network_time_info_to_string(const Cellular::NetworkTimeInfo &nti)
{
  ostringstream os ;

  os << "{zone=" << nti.offsetFromUtc() ;

  QDateTime t = nti.dateTime() ;
  if (t.isValid())
  {
    string utc = str_printf("%04d-%02d-%02d,%02d:%02d:%02d", t.date().year(), t.date().month(), t.date().day(), t.time().hour(), t.time().minute(), t.time().second())  ;
    os << ", utc=" << utc ;
  }

  int dst = nti.daylightAdjustment() ;
  if (dst!=-1)
    os << ", dst=" << dst ;

  os << ", mcc='" << nti.mcc().toStdString() << "'" ;
  os << ", mnc='" << nti.mnc().toStdString() << "'" ;

  os << ", received=" << str_printf("%lld.%09lu", (long long)nti.timestamp()->tv_sec, nti.timestamp()->tv_nsec) ;

  os << "}" ;

  return os.str() ;
}
#endif//F_CELLULAR_QT
