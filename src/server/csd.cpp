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

#include "timed.h"

#include "csd.h"

#if OFONO

csd_t::csd_t(Timed *owner)
  : QObject(owner)
{
  timed = owner ;
  nt = new NetworkTime;
  op = new NetworkOperator;
  static const char *time_signal1 = SIGNAL(timeInfoChanged(const NetworkTimeInfo &)) ;
  static const char *time_signal2 = SIGNAL(timeInfoQueryCompleted(const NetworkTimeInfo &)) ;
  static const char *time_slot1 = SLOT(csd_time_s(const NetworkTimeInfo &)) ;
  static const char *time_slot2 = SLOT(csd_time_q(const NetworkTimeInfo &)) ;
  bool res1 = QObject::connect(nt, time_signal1, this, time_slot1) ;
  bool res2 = QObject::connect(nt, time_signal2, this, time_slot2) ;

  if(res1 && res2)
    log_info("succesfully connected to csd time signals") ;
  else
    log_error("connection to cellular csd signals failed: %s %s", res1?"":time_signal1, res2?"":time_signal2) ;

  static const char *operator_signal = SIGNAL(operatorChanged(const QString &, const QString &)) ;
  static const char *operator_slot = SLOT(csd_operator_s(const QString &, const QString &)) ;
  bool res_op = QObject::connect(op, operator_signal, this, operator_slot);
  if(res_op)
    log_info("succesfully connected to csd network operator signal") ;
  else
    log_error("connection to csd network operator signal failed") ;

  nt->queryTimeInfo() ;
  QMetaObject::invokeMethod(this, "csd_operator_q", Qt::QueuedConnection) ;

  timer = new QTimer ;
  timer->setSingleShot(true) ;
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(wait_for_operator_timeout())) ;

  time = NULL ;
  offs = NULL ;
}

csd_t::~csd_t()
{
  delete nt ;
  delete op ;
  delete timer ;
  delete offs ;
  delete time ;
}

void csd_t::csd_operator_q()
{
  QString mcc = op->mcc(), mnc = op->mnc() ;
  log_notice("CSD::csd_operator_q {mcc='%s', mnc='%s'}", mcc.toStdString().c_str(), mnc.toStdString().c_str()) ;
  process_csd_network_operator(mcc, mnc) ;
}

void csd_t::csd_operator_s(const QString &mnc, const QString &mcc)
{
  log_notice("CSD::csd_operator_s {mcc='%s', mnc='%s'}", mcc.toStdString().c_str(), mnc.toStdString().c_str()) ;
  process_csd_network_operator(mcc, mnc) ;
}

void csd_t::csd_time_q(const NetworkTimeInfo &nti)
{
  log_notice("CSD::csd_time_q %s", csd_network_time_info_to_string(nti).c_str()) ;
  process_csd_network_time_info(nti) ;
}

void csd_t::csd_time_s(const NetworkTimeInfo &nti)
{
  log_notice("CSD::csd_time_s %s", csd_network_time_info_to_string(nti).c_str()) ;
  process_csd_network_time_info(nti) ;
}

void csd_t::input_csd_network_time_info(const NetworkTimeInfo &nti)
{
  timer->stop() ;
  cellular_time_t new_time(nti) ;
  if (offs)
    delete offs ;
  offs = new cellular_offset_t(nti) ;
  if (new_time.is_valid())
  {
    if (time)
      delete time ;
    time = new cellular_time_t(new_time) ;
  }
}

void csd_t::output_csd_network_time_info()
{
  timer->stop() ; // paranoia
  if (offs)
  {
    emit csd_cellular_offset(*offs) ;
    delete offs ;
    offs = NULL ;
  }
  if (time)
  {
    emit csd_cellular_time(*time) ;
    delete time ;
    time = NULL ;
  }
}

const nanotime_t csd_t::old_nitz_threshold(2,0) ;

void csd_t::process_csd_network_time_info(const NetworkTimeInfo &nti)
{
  if (not nti.isValid())
  {
    log_notice("empty time info, ignoring it") ;
    return ;
  }
  nanotime_t actuality = nanotime_t::monotonic_now() - nanotime_t::from_timespec (*nti.timestamp()) ;
  input_csd_network_time_info(nti) ;

  // Decide if the data is to be sent immediately or to wait for operator change signal
  bool current_operator = offs->oper.mcc == oper.mcc and offs->oper.mnc == oper.mnc ;
  bool empty_operator = offs->oper.mcc.empty() and offs->oper.mnc.empty() ;
  bool input_is_old = actuality > old_nitz_threshold ;
  bool send_now = input_is_old or (current_operator and not empty_operator) ;

  log_debug("offs->oper=%s, oper=%s, actuality=%s", offs->oper.str().c_str(), oper.str().c_str(), actuality.str().c_str()) ;
  log_debug("current_operator=%d, empty_operator=%d, input_is_old=%d", current_operator, empty_operator, input_is_old) ;
  log_debug("send_now=%d", send_now) ;

  if (send_now)
    output_csd_network_time_info() ;
  else
    timer->start(operator_wait_ms) ;
}

void csd_t::process_csd_network_operator(const QString &mcc, const QString &mnc)
{
  timer->stop() ;
  oper = cellular_operator_t(mcc, mnc) ;
  if (offs)
    offs->oper = oper ;
  output_csd_network_time_info() ; // if needed
  emit csd_cellular_operator(oper) ;
}

void csd_t::wait_for_operator_timeout() // timer slot
{
  timer->stop() ; // paranoia
  output_csd_network_time_info() ; // probably needed
}

string csd_t::csd_network_time_info_to_string(const NetworkTimeInfo &nti)
{
  if (not nti.isValid())
    return "{invalid}" ;

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
#endif//OFONO
