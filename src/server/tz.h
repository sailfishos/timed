/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
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

#if 0
struct tz_default_t ;
#endif

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
public slots:
  void mcc_delay_timeout() ;
  void nitz_data(const cellular_info_t &) ;
  void user_input(olson *tz) ;
  void mcc_data(int mcc, const string &mnc) ;

signals:
  void tz_detected(olson *tz, tz_suggestions_t) ;

private:
  iodata::validator *validator() ;
  iodata::record *open_database(const char *path, const char *type) ;

  tz_single_t *tz_single ;
  tz_distinct_t *tz_distinct ;
#if 0
  tz_default_t *tz_default ;
#endif

  bool is_single(int mcc) ;

  Q_OBJECT ;
} ;

#if 0

// If nothing else works, just use Greenwich (why not Moscow, or Asia/Gaza)
struct tz_total_fail_t
{
  // string guess_timezone(const cellular_info_t *) { return "Fixed/UTC" ; }
  // Fixed/* not available yet
  // Anyway, it's a bad idea, because we want to have a _geographic_, not fixed zone.
  // Need some better solution!
  //
  // Maybe just a special case: auto_dst in on, and the zone is Fixed/UTC ?
  string guess_timezone(const cellular_info_t *) { return "Europe/London" ; }
} ;

struct tz_default_t : public tz_oracle_t
{
  // Timo will implement customization settings and put it here
  string guess_timezone(const cellular_info_t *) { return "Europe/Helsinki" ; }
} ;

#endif


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

#if 0

struct tz_pure_offset_t : public tz_oracle_t
{
  string guess_timezone(const cellular_info_t *) ;
  tz_pure_offset_t() ;
  // map<int, string> mcc_to_tz ;
} ;


// ---------------------------------------------------------

struct tz_mcc_info

struct tz_country
{
  const olson *single_zone() ;
  const olson *main_zone() ;
} ;

struct tz_country
{
  tz_country *cinfo ;
  const olson *tz ;
  set<const olson *> candidates, foreign ;
  vector<const olson *> history ;
  int offset, dst_flag ;
  time_t valid_from, valid_to ;

  bool valid_at(time_t now) { return valid_from <= now && now < valid_to ; }
  bool compute_candidates(time_t now) ;

  void gmt_offset(time_t now, int new_offset, int new_dst_flag) ;
  void user_input(time_t now, const olson *utz) ;
  void update_time(time_t now) ;
} ;

struct tz_cellular_engine
{
  int mcc ;
  bool connected ;
  map<int, tz_local_state*> ;
} ;
#endif


#endif
