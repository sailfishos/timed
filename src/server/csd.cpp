/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
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
  static const char *time_slot1 = SLOT(csd_time_s(const NetworkTimeInfo &)) ;
  static const char *time_slot2 = SLOT(csd_time_q(const NetworkTimeInfo &)) ;
  bool res1 = QObject::connect(nt, time_signal1, this, time_slot1) ;
  bool res2 = QObject::connect(nt, time_signal2, this, time_slot2) ;

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
  QMetaObject::invokeMethod(this, "csd_operator_q", Qt::QueuedConnection) ;
}

csd_t::~csd_t()
{
  delete nt ;
  delete op ;
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

void csd_t::csd_time_q(const Cellular::NetworkTimeInfo &nti)
{
  log_notice("CSD::csd_time_q %s", csd_network_time_info_to_string(nti).c_str()) ;
  process_csd_network_time_info(nti) ;
}

void csd_t::csd_time_s(const Cellular::NetworkTimeInfo &nti)
{
  log_notice("CSD::csd_time_s %s", csd_network_time_info_to_string(nti).c_str()) ;
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
#endif//F_CELLULAR_QT
