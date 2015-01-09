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
#ifndef MAEMO_TIMED_TZ_H
#define MAEMO_TIMED_TZ_H

#include <string>
#include <map>
using namespace std ;

#include <QTimer>


#include "onitz.h"
#include "olson.h"
#include "cellular.h"

struct status_t
{
  olson *last_zone ;
  bool regular ;
} ;

// fake history implementation doing nothing
struct history_t
{
  void save_status(const status_t &/*s*/, const cellular_operator_t &/*op*/) { }
  void load_status(status_t &s, const cellular_operator_t &/*op*/) { s.last_zone=NULL, s.regular=true ; }
} ;

enum guess_quality
{
  Uncertain, Reliable, Confirmed, Canceled, Waiting, Initial
} ;

struct tz_suggestions_t // obsolete
{
  vector <olson*> zones ;
  guess_quality gq ;

  void add(olson *tz) { zones.push_back(tz) ; }
  void set(vector<olson*> list) { zones = list ; }
  tz_suggestions_t() { gq = Uncertain ; }
} ;

struct suggestion_t
{
  std::map<olson *, int> s ;
  void add(olson *zone, int score) ;
} ;

struct tz_oracle_t : public QObject
{
  static const int nitz_wait_ms = 1000 ;

  QTimer *timer ;
  history_t *history ;

  cellular_operator_t oper ;
  bool have_oper ;
  status_t stat ;

  tz_oracle_t() ;
 ~tz_oracle_t() ;
public slots:
  void waiting_for_nitz_timeout() ;
  void cellular_operator(const cellular_operator_t &data) ;
  void cellular_offset(const cellular_offset_t &data) ;

signals:
  void cellular_zone_detected(olson *, suggestion_t, bool) ;

private:
  void set_by_offset(const cellular_offset_t &data) ;
  // void set_by_operator(const cellular_operator_t &o) ;
  void set_by_operator() ;

  void output(olson *zone, suggestion_t *s, bool sure) ;
  void output(olson *zone) ;

  Q_OBJECT ;
} ;

#endif
