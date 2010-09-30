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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

#include <iodata/iodata.h>

#include "onitz.h"

#include "timed/nanotime.h"
#include "timed/wall-declarations.h"
#include "timed/wall-info.h"

#include "tz.h"

using namespace std ;

class Timed ;

struct source_t
{
  virtual ~source_t() { } ;
  virtual const char* name() const = 0 ;
  virtual bool available() const = 0 ;
  virtual void load(const iodata::record *) ;
  virtual iodata::record *save() const ;
} ;

struct utc_source_t : public source_t
{
  nanotime_t value ;
  virtual nanotime_t value_at_zero() const { return nanotime_t(0,0) ; }
} ;

struct manual_utc_t : public utc_source_t
{
  manual_utc_t() { value = nanotime_t::systime_at_zero() ; }
  const char* name() const { return "manual_utc" ; }
  bool available() const { return true ; }
  nanotime_t value_at_zero() const { return value ; }
} ;

struct nitz_utc_t : public utc_source_t
{
  nitz_utc_t() ;
  const char* name() const { return "nitz_utc" ; }
  bool available() const ;
  nanotime_t value_at_zero() const ;
} ;

struct ntp_utc_t : public utc_source_t
{
  const char* name() const { return "ntp_utc" ; }
  bool available() const { return false ; }
} ;

struct gps_utc_t : public utc_source_t
{
  const char* name() const { return "gps_utc" ; }
  bool available() const { return false ; }
} ;

struct offset_source_t : public source_t
{
  int value ;
  virtual int offset() const { return value ; }
  offset_source_t() { value = -1 ; }
  bool available() const { return value!=-1 ; }
} ;

struct manual_offset_t : public offset_source_t
{
  const char* name() const { return "manual_offset" ; }
  void load(const iodata::record *) ;
  iodata::record *save() const ;
} ;

struct nitz_offset_t : public offset_source_t
{
  const char* name() const { return "nitz_offset" ; }
} ;

struct zone_source_t : public source_t
{
  string value ;
  virtual string zone() const { return value ; }
  bool available() const { return !value.empty() ; }
  void load(const iodata::record *) ;
  iodata::record *save() const ;
} ;

struct manual_zone_t : public zone_source_t
{
  const char *name() const { return "manual_zone" ; }
} ;

struct cellular_zone_t : public zone_source_t
{
  tz_suggestions_t suggestions ;
  const char *name() const { return "cellular_zone" ; }
  // void load(const iodata::record *) ;
  // iodata::record *save() const ;
} ;

struct customization_settings
{
  customization_settings() ;

  bool net_time_enabled;
  bool time_nitz;
  bool format_24;
  QString default_tz;
  QString time_nitz_str;
  QString format_24_str;
  QMap<QString, QVariant> valueMap;

  void load();
  void load(const iodata::record *) ;
  iodata::record* save();

  QMap<QString, QVariant> get_values();

  static void check_customization(int ac, char** av);
  static iodata::storage* get_storage();
  static QByteArray get_hash();

};

struct source_settings
{
  source_settings(Timed *owner) ;
  Timed *o ;

  zone_source_t *manual_zone ;
  cellular_zone_t *cellular_zone ;
  offset_source_t *manual_offset, *nitz_offset ;
  utc_source_t *manual_utc, *nitz_utc, *gps_utc, *ntp_utc ;

  bool time_nitz, local_cellular, auto_dst ;
  bool format_24 ;

  int offset() const ;
  string zone() const ;
  string human_readable_timezone() const ;
  string etc_localtime() const ;
  nanotime_t value_at_zero() const ;

  void set_system_time(const nanotime_t &) ;
#if 0 // moved to timed
  bool signal_invoked ;
  nanotime_t systime_back ;
#endif

  map<string,source_t*> src ;

  // source *src_(const string &) const ;
  void load(const iodata::record *) ;
  iodata::record *save() const ;

  Maemo::Timed::WallClock::wall_info_pimple_t *get_wall_clock_info(const nanotime_t &) const ;
  bool wall_clock_settings(const Maemo::Timed::WallClock::wall_settings_pimple_t &p) ;

#if 0 // moved to timed
  void invoke_signal(const nanotime_t &) ;
  void invoke_signal() { nanotime_t zero ; invoke_signal(zero) ; }
  void clear_invokation_flag() { signal_invoked = false ; systime_back.invalidate() ; }
#endif

  static string symlink_target(int offset_sec) ;
  static bool east_of_greenwich(string &result, int offset_sec, bool reverse=false) ;
  static string symlink_target(string zone) ;
  static int check_target(string path) ;

  void fix_etc_localtime() ;
  void postload_fix_manual_zone() ;
  void postload_fix_manual_offset() ;

  void cellular_information(const cellular_info_t &ci) ;

} ;

#endif
