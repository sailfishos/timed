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
