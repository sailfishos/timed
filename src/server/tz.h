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
#if 0
  bool connected ;
  enum guess_quality gq ;
#endif

  QTimer *timer ;
#if 0
  cellular_operator_t *waiting_oper ;
#endif
  history_t *history ;

  cellular_operator_t oper ;
  bool have_oper ;
  status_t stat ;

  tz_oracle_t() ;
 ~tz_oracle_t() ;
public slots:
  void waiting_for_nitz_timeout() ;
#if 0
  void nitz_data(const cellular_info_t &) ;
#else
  void cellular_operator(const cellular_operator_t &data) ;
  void cellular_offset(const cellular_offset_t &data) ;
#endif

#if 0
TODO:
  void user_input(olson *tz) ;
#endif

#if 0
  void mcc_data(int mcc, const string &mnc) ;
#endif

signals:
  void tz_detected(olson *tz, tz_suggestions_t) ;

private:
#if 0
  void handle_offset(const cellular_info_t &) ;
#endif

#if 0
  string mcc_to_xy(int mcc) ; // maps mcc to country code (2 chars)
  map<string, vector<string> > xy_to_tz ; // time zones by country code
  iodata::validator *validator() ;
  iodata::record *open_database(const char *path, const char *type) ;
  void read_timezones_by_country() ;
#endif

#if 0
  tz_single_t *tz_single ;
  tz_distinct_t *tz_distinct ;
#endif

#if 0
  bool is_single(int mcc) ;
#endif

  void set_by_offset(const cellular_offset_t &data) ;
  // void set_by_operator(const cellular_operator_t &o) ;
  void set_by_operator() ;

  void output(olson *zone, suggestion_t *s, bool sure) ;
  void output(olson *zone) ;

  Q_OBJECT ;
} ;

#if 0
struct tz_distinct_t
{
  olson * guess_timezone(int mcc, tz_suggestions_t &list) ;
  tz_distinct_t(const iodata::record *) ;
  map<int, vector<olson*> > mcc_to_tzlist ;
} ;

struct tz_single_t
{
  olson * guess_timezone(int mcc) ;
  tz_single_t(const iodata::record *) ;
  map<int, string> mcc_to_tz ;
} ;
#endif

#endif
