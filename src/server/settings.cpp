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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QMetaMethod>

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>

#include "daemon/flags.h"
#include "timed/wall-settings.h"

#include "timed.h"
#include "settings.h"
#include "misc.h"
#include "states.h"
#include "onitz.h"

void source_t::load(const iodata::record *)
{
  // empty, do nothing
}

iodata::record *source_t::save() const
{
  return new iodata::record ;
}

nitz_utc_t::nitz_utc_t()
{
  value.invalidate() ;
}

bool nitz_utc_t::available() const
{
  return ! value.is_invalid() ;
}

nanotime_t nitz_utc_t::value_at_zero() const
{
  return value ;
}

void manual_offset_t::load(const iodata::record *r)
{
  value = r->get("value")->value() ;
}

iodata::record *manual_offset_t::save() const
{
  iodata::record *r = new iodata::record ;
  r->add("value", value) ;
  return r ;
}

void zone_source_t::load(const iodata::record *r)
{
  value = r->get("value")->str() ;
}

iodata::record *zone_source_t::save() const
{
  iodata::record *r = new iodata::record ;
  r->add("value", value) ;
  return r ;
}

#if 0
#define CUST_FILE "/etc/clockd/clockd-settings.default"
QByteArray customization_settings::get_hash()
{
  QByteArray hash;
  QFile custFile(CUST_FILE);
  if (custFile.open(QIODevice::ReadOnly))
  {
    QByteArray bytes = custFile.read(10*1000);
    hash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha1);
  }
  else
  {
    log_error("Could not open customization file "CUST_FILE);
  }
  return hash.toBase64();
}

iodata::storage* customization_settings::get_storage()
{
    iodata::storage *s = new iodata::storage();
    s->set_primary_path("/etc/timed-cust.rc");
    s->set_validator("/usr/share/timed/typeinfo/timed-cust-rc.type", "timed_cust_rc_t");
    return s;
}

void customization_settings::check_customization(int ac, char **av)
{
    /**
     * If we have --cust, read the values from the script file,
     * and save them with iodata::storage.
     */
    const char cust[] = "--cust";
    for (int i=1; i < ac; i++)
    {
      if (!strncmp(cust, av[i], strlen(cust)))
      {
        log_debug("--cust");

        // Get the envs to a record.
        struct customization_settings settings;
        char *envVal = NULL;

        envVal = getenv("CLOCKD_NET_TIME");
        if (envVal)
        {
            log_debug("CUST CLOCKD_NET_TIME: %s", envVal);
          if (!strncmp(envVal, "yes", 3) ||
              !strncmp(envVal, "no", 2) ||
              !strncmp(envVal, "disabled", 8))
          {
              settings.time_nitz_str = envVal;
          }
          else
          {
            log_error("Invalid CLOCKD_NET_TIME value");
          }
        }
        else
        {
          log_error("no CLOCKD_NET_TIME");
        }

        envVal = getenv("CLOCKD_TIME_FORMAT");
        if (envVal)
        {
          log_debug("CUST CLOCKD_TIME_FORMAT: %s", envVal);
          if (!strncmp(envVal, "R", 1) ||
              !strncmp(envVal, "r", 1))
          {
            settings.format_24_str = envVal;
          }
          else
          {
            log_error("Invalid CLOCKD_TIME_FORMAT value");
          }

        }
        else
        {
          log_error("no CLOCKD_TIME_FORMAT");
        }
        envVal = getenv("CLOCKD_DEFAULT_TZ");
        if (envVal)
        {
            log_debug("CUST CLOCKD_DEFAULT_TZ: %s", envVal);
          settings.default_tz = envVal;
        }

        iodata::record *r = settings.save();

        QByteArray hash = get_hash();
        log_debug("CUST hash: %s", hash.data());
        r->add("hash", hash.data());

        // Save the record to a storage.
        iodata::storage *s = get_storage();
        s->save(r);
        delete s;
        delete r;

        exit(0);
      }
    }

    /**
     * Check that have up to date customization data.
     * If not, fork timed with --cust and do it.
     */
    QByteArray hash = get_hash();
    if (!hash.isEmpty())
    {
      iodata::storage *custStorage = get_storage();
      iodata::record *custRecord = custStorage->load();
      bool newCust = true;
      if (custStorage->source() != -1 && custRecord)
      {
        const iodata::item *sum = custRecord->get("hash");
        if (sum && sum->str() == hash.data())
        {
          newCust = false;
        }
        else
        {
          newCust = true;
        }
      }
      delete custRecord;
      delete custStorage;

      if (newCust)
      {
        log_debug("CUST forking...");
        QString cmd = "source "CUST_FILE;
        cmd += "; ";
        cmd += av[0];
        cmd += " --cust";
        log_debug("CUST %s", cmd.toAscii().data());
        system(cmd.toAscii().data());
      }
    }
}

customization_settings::customization_settings()
{
  log_debug() ;

  net_time_enabled = true;
  time_nitz = true;
  format_24 = false;
  default_tz = "327";
}

void customization_settings::load(const iodata::record * record)
{
  log_debug() ;

  time_nitz_str = record->get("time_nitz")->str().c_str();
  format_24_str = record->get("format_24")->str().c_str();
  default_tz = record->get("default_tz")->str().c_str();

  log_debug("CUST time_nitz_str: %s", time_nitz_str.toAscii().data());
  log_debug("CUST format_24_str: %s", format_24_str.toAscii().data());
  log_debug("CUST default_tz: %s", default_tz.toAscii().data());

  if (time_nitz_str == "disabled")
  {
    net_time_enabled=false;
  }
  else
  {
    net_time_enabled=true;
  }
  if (time_nitz_str == "yes")
  {
    time_nitz = true;
  }
  else if (time_nitz_str == "no")
  {
    time_nitz = false;
  }
  if (format_24_str == "r")
  {
    format_24 = false;
  }
  else if (format_24_str == "R")
  {
    format_24 = true;
  }

  log_debug("CUST time_nitz: %d", time_nitz);
  log_debug("CUST format_24: %d", format_24);
  log_debug("CUST net_time_enabled: %d", net_time_enabled);

  valueMap.insert(QString("CLOCKD_NET_TIME"), time_nitz_str);
  valueMap.insert(QString("CLOCKD_TIME_FORMAT"), format_24_str);
  valueMap.insert(QString("CLOCKD_DEFAULT_TZ"), default_tz);
}

QMap<QString, QVariant> customization_settings::get_values()
{
  return valueMap;
}

void customization_settings::load()
{
    iodata::storage *custStorage = get_storage();
    iodata::record *custRecord = custStorage->load();
    load(custRecord);
    delete custRecord;
    delete custStorage;
}

iodata::record* customization_settings::save()
{
  iodata::record *r = new iodata::record;
  r->add("time_nitz", time_nitz_str.toAscii().data());
  r->add("format_24", format_24_str.toAscii().data());
  r->add("default_tz", default_tz.toAscii().data());
  return r;
}
#endif

source_settings::source_settings(Timed *owner)
{
  log_debug() ;
  o = owner ;
  time_nitz = true ;
  local_cellular = true ;
  auto_dst = true ;
  format_24 = true ;
  // new options:
  alarms_are_enabled = false ;
  default_snooze_value = 300 ;
#define _creat(xxx) src[#xxx] = xxx = new xxx##_t ;
  _creat(manual_utc) ;
  _creat(nitz_utc) ;
  _creat(gps_utc) ;
  _creat(ntp_utc) ;
  _creat(manual_offset) ;
  _creat(nitz_offset) ;
  _creat(manual_zone) ;
  _creat(cellular_zone) ;
#undef _creat // spell it without 'e' ;-)
}

int source_settings::default_snooze() const
{
  return default_snooze_value ;
}

int source_settings::default_snooze(int new_value)
{
  if(30 <= new_value) // TODO: make it configurierable?
  {
    default_snooze_value = new_value ;
    o->save_settings() ;
  }
  return default_snooze_value ;
}

void source_settings::load(const iodata::record *r)
{
  log_debug() ;
  log_debug("CUST time_nitz: %d", r->get("time_nitz")->value());
  log_debug("CUST format_24: %d", r->get("format_24")->value());
  time_nitz = r->get("time_nitz")->value() != 0 ;
  local_cellular = r->get("local_cellular")->value() != 0 ;
  auto_dst = r->get("auto_dst")->value() != 0 ;
  format_24 = r->get("format_24")->value() != 0 ;
  default_snooze_value = r->get("default_snooze")->value() ;
  alarms_are_enabled = r->get("alarms")->value() ;
  log_debug("alarms_are_enabled := alarms == %d", alarms_are_enabled) ;
  for(map<string,source_t*>::iterator it=src.begin(); it!=src.end(); ++it)
  {
    log_debug("it: '%s'", it->first.c_str()) ;
    it->second->load(r->get(it->first)->rec()) ;
    log_debug() ;
  }
#if 0
  postload_fix_manual_zone() ;
  postload_fix_manual_offset() ;
  if(check_target(etc_localtime()) != 0)
    o->invoke_signal() ;
#endif
}

iodata::record *source_settings::save() const
{
  iodata::record *r = new iodata::record ;
  r->add("time_nitz",time_nitz) ;
  r->add("local_cellular", local_cellular) ;
  r->add("auto_dst", auto_dst) ;
  r->add("format_24", format_24) ;
  r->add("default_snooze", default_snooze_value) ;
  r->add("alarms", alarms_are_enabled) ;
  for(map<string,source_t*>::const_iterator it=src.begin(); it!=src.end(); ++it)
    r->add(it->first, it->second->save()) ;
  return r ;
}

Maemo::Timed::WallClock::wall_info_pimple_t *source_settings::get_wall_clock_info(const nanotime_t &back) const
{
  Maemo::Timed::WallClock::wall_info_pimple_t *p = new Maemo::Timed::WallClock::wall_info_pimple_t ;
  p->flag_time_nitz = time_nitz ;
  p->flag_local_cellular = local_cellular ;
  p->flag_auto_dst = auto_dst ;
  p->flag_format_24 = format_24 ;

  log_assert(Maemo::Timed::WallClock::TimezoneManual==0) ;
  p->zones.push_back(string_std_to_q(manual_zone->zone())) ;
  log_assert(Maemo::Timed::WallClock::TimezoneCellular==1) ;
  p->zones.push_back(string_std_to_q(cellular_zone->zone())) ;

  log_assert(Maemo::Timed::WallClock::OffsetManual==0) ;
  p->offsets.push_back(manual_offset->offset()) ;
  log_assert(Maemo::Timed::WallClock::OffsetNitz==1) ;
  p->offsets.push_back(nitz_offset->offset()) ;

  log_assert(Maemo::Timed::WallClock::UtcManual==0) ;
  p->clocks.push_back(manual_utc->value_at_zero()) ;
  log_assert(Maemo::Timed::WallClock::UtcNitz==1) ;
  p->clocks.push_back(nitz_utc->value_at_zero()) ;
  log_assert(Maemo::Timed::WallClock::UtcGps==2) ;
  p->clocks.push_back(gps_utc->value_at_zero()) ;
  log_assert(Maemo::Timed::WallClock::UtcNtp==3) ;
  p->clocks.push_back(ntp_utc->value_at_zero()) ;

#define _import(x) static const int x=Maemo::Timed::WallClock::wall_info_pimple_t::x ;
  _import(CL) ;
  _import(ZO) ;
  _import(OF) ;
  _import(N_DATA) ;
#undef _import

  p->data_sources.resize(N_DATA) ;
  p->data_sources[CL]=Maemo::Timed::WallClock::UtcManual ;
  p->data_sources[ZO]=Maemo::Timed::WallClock::TimezoneManual ;
  p->data_sources[OF]=Maemo::Timed::WallClock::OffsetManual ;

  if(time_nitz && nitz_utc->available())
    p->data_sources[CL]=Maemo::Timed::WallClock::UtcNitz ;

  if(local_cellular && cellular_zone->available())
    p->data_sources[ZO]=Maemo::Timed::WallClock::TimezoneCellular ;

  if(local_cellular && nitz_offset->available())
    p->data_sources[OF]=Maemo::Timed::WallClock::OffsetNitz ;

  string etc_localtime_value = etc_localtime() ;

  p->human_readable_tz = string_std_to_q(human_readable_timezone()) ;
  p->localtime_symlink = string_std_to_q(etc_localtime_value) ;

  string abb ;

  if(!zone_info(&p->seconds_east_of_gmt, etc_localtime_value, &abb))
    p->abbreviation = "<n/a>", p->seconds_east_of_gmt = 0 ;
  else
    p->abbreviation = string_std_to_q(abb) ;

  p->systime_diff = back ;
  log_debug("back=%s", back.str().c_str()) ;

  return p ;
}

bool source_settings::east_of_greenwich(string &result, int offset_sec, bool reverse)
{
  static const int h12 = 12*60*60, h14 = 14*60*60 ;
  if(offset_sec<-h12 || offset_sec>h14 || offset_sec%(60*15)) // only 15min units are accepted
    return false ;
  int offset_min = offset_sec / 60 ;
  if(reverse)
    offset_min = - offset_min ;
  result = "" ;
  if(offset_min==0)
    return true ;
  result += offset_min<0 ? (offset_min=-offset_min, "-") : "+"  ;
  int hour = offset_min / 60, mins = offset_min % 60 ;
  result += str_printf("%d", hour) ;
  if(mins)
    result += str_printf(":%02d", mins) ;
  return true ;
}

string source_settings::symlink_target(int offset_sec)
{
  string shift ;
#define MAEMO_ZONES 0
#if MAEMO_ZONES
  string base = "/usr/share/zoneinfo/Maemo/UTC" ;
  if(east_of_greenwich(shift, offset_sec))
#else
  string base = "/usr/share/zoneinfo/Etc/GMT" ;
  if(east_of_greenwich(shift, offset_sec, true))
#endif
    return base + shift ;
  else
    return "" ;
}

string source_settings::human_readable_timezone() const
{
  static string unknown = "<unknown>" ;
  if(auto_dst)
  {
    string z = zone() ;
    if(z.empty())
      return unknown ;
    else
      return z ;
  }
  else
  {
    string shift ;
    if(east_of_greenwich(shift, offset()))
      return (string) "GMT" + shift ;
    else
      return unknown ;
  }
}

string source_settings::symlink_target(string zone)
{
  if(zone.empty() || zone==":")
    return "" ;
  const char *p = zone.c_str() ;
  if(zone[0]==':')
    ++ p ;
  bool in = false ;
  for(const char *q=p; *q; ++q)
  {
    if(*q=='/')
      if(!in)
        return "" ;
      else
        in = false ;
    else
      if(*q!='.')
        in = true ;
  }
  if(!in)
    return "" ;
  return (string)"/usr/share/zoneinfo/" + p ;
}

// returning: -1 if not exists
// 0 if already in use by /etc/localtime,
// 1 if file exists but not linked
int source_settings::check_target(string path)
{
  if(path.empty())
    return -1 ;
  if(access(path.c_str(), F_OK) != 0)
    return -1 ;
  struct stat s ;
  int res = stat(path.c_str(), &s) ;
  if(res!=0)
    return -1 ;
  if(!S_ISREG(s.st_mode))
    return -1 ;
  // The file exists and is a regular file !
  ino_t inode = s.st_ino ;
  res = stat("/etc/localtime", &s) ;
  if(res!=0 || inode!=s.st_ino)
    return 1 ;
  return 0 ;
}

string source_settings::etc_localtime() const
{
  if(auto_dst)
    return symlink_target(zone()) ;
  else
    return symlink_target(offset()) ;
}

int source_settings::offset() const
{
  if(local_cellular && nitz_offset->available())
    return nitz_offset->offset() ;
  else
    return manual_offset->offset() ;
}

string source_settings::zone() const
{
  if(local_cellular && cellular_zone->available())
    return cellular_zone->zone() ;
  else
    return manual_zone->zone() ;
}

nanotime_t source_settings::value_at_zero() const
{
  if(time_nitz && nitz_utc->available())
    return nitz_utc->value_at_zero() ;
  else
    return manual_utc->value_at_zero() ;
}

void source_settings::fix_etc_localtime()
{
  string target = etc_localtime() ;
  int x = check_target(target) ;
  if(x==0) // already okey
    return ;
  // Now try to set symlink target
  struct stat l ;
  int lstat_res = lstat("/etc/localtime", &l) ;
  if(lstat_res==0) // file exists, we can list it
  {
    int unlink_res = unlink("/etc/localtime") ;
    tzset() ;
    if(unlink_res)
    {
      log_error("can't unlink /etc/localtime: %m") ;
      return ;
    }
  }
  if(x==-1) // target doesn't exist, thus don't link
    return ;
  int symlink_res = symlink(target.c_str(), "/etc/localtime") ;
  tzset() ;
  if(symlink_res==0)
    log_info("created symlink: /etc/localtime -> %s", target.c_str()) ;
  else
    log_error("symlinking /etc/localtime -> %s failed: %m", target.c_str()) ;
}

void source_settings::postload_fix_manual_zone()
{
  log_debug() ;
  string tz = local_cellular ? cellular_zone->value : manual_zone->value ;
  string target = symlink_target(tz) ;
  if(check_target(target) < 0) // invalid time zone
  {
    if(auto_dst)
    {
      tz = o->get_default_timezone() ;
      if(check_target(symlink_target(manual_zone->value)) < 0)
        tz = "Etc/GMT" ;
    }
    else
      manual_zone->value = "" ;
  }
  (local_cellular ? cellular_zone->value : manual_zone->value) = tz ;
  log_debug() ;
}

void source_settings::postload_fix_manual_offset()
{
  if(!manual_offset->available() || check_target(symlink_target(manual_offset->offset())) < 0)
  {
    if(auto_dst)
      manual_offset->value = -1 ; // dont't needed, thus never mind invalid
    else // try to guess
    {
      manual_offset->value = o->get_default_gmt_offset() ; // if we don't have a better guess
      if(manual_zone->available())
      {
        int east_of_gmt, res = zone_info(&east_of_gmt, manual_zone->zone()) ;
        if(res)
          manual_offset->value = east_of_gmt ;
      }
    }
  }
}

bool source_settings::wall_clock_settings(const Maemo::Timed::WallClock::wall_settings_pimple_t &p)
{
  bool signal_needed = false ;

  using namespace WallOpcode ;
  // stage 1: sanity checking

  if(uint32_t op_time = p.opcodes & Op_Set_Time_Mask)
  {
    if(n_bits_32(op_time)>1)
    {
      log_error("several time operations at once requested") ;
      return false ;
    }
    if(op_time == Op_Set_Time_Manual_Val && p.time_at_zero.is_invalid())
    {
      log_error("setting invalid time request rejected") ;
      return false ;
    }
    signal_needed = true ;
  }

  if(uint32_t op_12_24 = p.opcodes & Op_Set_Format_12_24_Mask)
  {
    if(op_12_24==Op_Set_Format_12_24_Mask)
    {
      log_error("both 12 and 24 time format at once requested") ;
      return false ;
    }
    bool new_format_24 = op_12_24 == Op_Set_24 ;
    signal_needed = signal_needed || (new_format_24!=format_24) ;
  }

  string p_zone ;
  if(uint32_t op_zone = p.opcodes & Op_Set_Zone_Mask)
  {
    signal_needed = true ; // XXX: check it, not always needed
    if(n_bits_32(op_zone)>1)
    {
      log_error("several timezone operations at once requested") ;
      return false ;
    }
    p_zone = string_q_to_std(p.zone) ;
    if(op_zone & (Op_Set_Timezone_Manual | Op_Set_Timezone_Cellular_Fbk))
    {
      if(check_target(symlink_target(p_zone)) < 0)
      {
        log_error("rejecting invalid timezone: '%s'", p_zone.c_str()) ;
        return false ;
      }
    }
    if(op_zone & Op_Set_Offset_Manual_Val)
    {
      if(check_target(symlink_target(p.offset)) < 0)
      {
        log_error("rejecting invalid offset=%d", p.offset) ;
        return false ;
      }
    }
  }

  // Check if the network time is disabled by customization
  if (not o->is_nitz_supported() and (p.opcodes & Op_Set_Time_Nitz))
  {
    log_error("can't enable NITZ as time source: not supported by the device");
    return false;
  }

  // Stage 2: really do changes
  switch(/*uint32_t op_time =*/ p.opcodes & Op_Set_Time_Mask)
  {
    case Op_Set_Time_Nitz:
      time_nitz = true ;
      if(nitz_utc->available())
      {
        set_system_time(nitz_utc->value_at_zero()) ;
        o->open_epoch() ;
      }
      break ;
    case Op_Set_Time_Manual:
      manual_utc->value = value_at_zero() ;
      time_nitz = false ;
      break ;
    case Op_Set_Time_Manual_Val:
      time_nitz = false ;
      manual_utc->value = p.time_at_zero ;
      set_system_time(p.time_at_zero) ;
      o->open_epoch() ;
      break ;
  }

  if(uint32_t op_12_24 = p.opcodes & Op_Set_Format_12_24_Mask)
  {
    bool new_format_24 = op_12_24 == Op_Set_24 ;
    if(new_format_24!=format_24)
      log_info("Time format flag set to '%d'", format_24 ? 24:12) ;
    format_24 = new_format_24 ;
  }

  switch(/*uint32_t op_zone =*/ p.opcodes & Op_Set_Zone_Mask)
  {
    case Op_Set_Offset_Cellular:
      auto_dst = false ;
      local_cellular = true ;
      postload_fix_manual_offset() ;
      break ;
    case Op_Set_Offset_Manual:
      manual_offset->value = offset() ;
      auto_dst = false ;
      local_cellular = false ;
      postload_fix_manual_offset() ;
      break ;
    case Op_Set_Offset_Manual_Val:
      manual_offset->value = p.offset ;
      auto_dst = false ;
      local_cellular = false ;
      break ;
    case Op_Set_Timezone_Cellular:
      auto_dst = true ;
      local_cellular = true ;
      o->update_oracle_context(true) ;
      break ;
    case Op_Set_Timezone_Cellular_Fbk:
      auto_dst = true ;
      local_cellular = true ;
      manual_zone->value = p_zone ;
      o->update_oracle_context(true) ;
      break ;
    case Op_Set_Timezone_Manual:
      auto_dst = true ;
      local_cellular = false ;
      manual_zone->value = p_zone ;
      o->update_oracle_context(false) ;
      int new_offset ;
      if(zone_info(&new_offset, p_zone))
      {
        if(check_target(symlink_target(new_offset))>=0)
          manual_offset->value = new_offset ;
      }
      break ;
  }

  if(p.opcodes & Op_Set_Zone_Mask)
    fix_etc_localtime() ;

  // That's it. Emit a signal and it's done.

  if(signal_needed)
    o->invoke_signal() ;

  return true ;
}

void source_settings::set_system_time(const nanotime_t &t)
{
  ostringstream log ;
  nanotime_t back = nanotime_t::systime_at_zero() ;
  log << "time_back=" << back.str() << " " ;
  int res = nanotime_t::set_systime_at_zero(t) ;
  if(res<0)
    log_critical("can't set system time: %m") ;

  // TODO: do it without 'hwclock'
  res = system("hwclock -w") ;
  if(res!=0)
    log_critical("hwclock -w failed: %m") ;
// #warning  TODO: reshuffle event queue... it's done, isn't it?
  nanotime_t new_time = nanotime_t::systime_at_zero() ;
  log << "new_time=" << new_time.str() << " " ;
  back -= new_time ;
  log << "diff=" << back.str() ;
  o->invoke_signal(back) ;
  log_debug("Time change: %s", log.str().c_str()) ;
#if 0
  o->save_time_to_file() ;
#endif
}

void source_settings::cellular_information(const cellular_info_t &ci)
{
  log_debug() ;

  if(ci.has_time())
  {
    nitz_utc->value = ci.time_at_zero() ;
    if(time_nitz) // we want to use nitz as time source
    {
      set_system_time(nitz_utc->value_at_zero()) ;
      o->open_epoch() ;
    }
  }
}
