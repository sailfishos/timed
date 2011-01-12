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

#include <iodata/validator>
#include <iodata/storage>

#include "onitz.h"
#include "olson.h"

struct tz_single_t ;
struct tz_distinct_t ;

enum guess_quality
{
  Uncertain, Reliable, Confirmed, Canceled, Waiting, Initial
} ;

struct tz_suggestions_t
{
  vector <olson*> zones ;
  guess_quality gq ;

  void add(olson *tz) { zones.push_back(tz) ; }
  void set(vector<olson*> list) { zones = list ; }
  tz_suggestions_t() { gq = Uncertain ; }
} ;

struct tz_oracle_t : public QObject
{
  int mcc ;
  bool connected ;
  enum guess_quality gq ;
  QTimer *delay ; // TODO: make a fuzzy timer

  tz_oracle_t() ;
 ~tz_oracle_t() ;
public slots:
  void mcc_delay_timeout() ;
  void nitz_data(const cellular_info_t &) ;
  void user_input(olson *tz) ;
  void mcc_data(int mcc, const string &mnc) ;

signals:
  void tz_detected(olson *tz, tz_suggestions_t) ;

private:
  void handle_offset(const cellular_info_t &) ;
  string mcc_to_xy(int mcc) ; // maps mcc to country code (2 chars)
  map<string, vector<string> > xy_to_tz ; // time zones by country code
  iodata::validator *validator() ;
  iodata::record *open_database(const char *path, const char *type) ;
  void read_timezones_by_country() ;

  tz_single_t *tz_single ;
  tz_distinct_t *tz_distinct ;

  bool is_single(int mcc) ;

  Q_OBJECT ;
} ;

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
