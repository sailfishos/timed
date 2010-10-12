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
#include <sstream>

#include <pcrecpp.h>

#include <qm/log>

#include <timed/event>

#include "timed/event-pimple.h"
#include "timed/event-io.h"

#include "misc.h"

#include "event.h"

using namespace Maemo::Timed ;

event_t::event_t()
{
  flags = 0 ;
  tsz_max = tsz_counter = 0 ;
  st = NULL ;
#if 0
  dialog_req_watcher = NULL ;
#endif
  request_watcher = NULL ;
}

event_t::~event_t()
{
  if(request_watcher)
  {
    log_debug("request_watcher still alive, detaching") ;
    request_watcher->detach(this) ;
  }
#if 0
  if(dialog_req_watcher)
  {
    log_debug("dialog_req_watcher still alive, deleting") ;
    delete dialog_req_watcher ;
  }
#endif
}

static void copy_recr(const recurrence_io_t &from, recurrence_pattern_t &to)
{
  to.mins = from.mins & ((1LL<<60)-1) ;
  to.hour = from.hour & ((1<<24)-1) ;
  to.mday = from.mday ;
  to.wday = from.wday & ((1<< 7)-1) ;
  to.mons = from.mons & ((1<<12)-1) ;
}

bool event_t::check_attributes(string &error_message, const attribute_t &a, bool empty_only)
{
  static pcrecpp::RE known_keyword =
    "APPLICATION|TITLE|TEST|COMMAND|USER|CROUP|"
    "DBUS_SERVICE|DBUS_PATH|DBUS_INTERFACE|DBUS_METHOD|DBUS_SIGNAL|"
    "PLUGIN|BACKUP" ;
  static pcrecpp::RE upper_case = "[A-Z_]+" ;
  static pcrecpp::RE app_name = "[A-Za-z_][A-Za-z_0-9]*" ;

  bool app_name_found = false ;

  for(map<string,string>::const_iterator it=a.txt.begin(); it!=a.txt.end(); ++it)
  {
    if(it->first.empty())
      return error_message += ": empty attribute key", false ;
    if(it->second.empty())
      return error_message += ": empty value of attribute '"+it->first+"'", false ;
    if(empty_only)
      continue ;
    if(upper_case.FullMatch(it->first))
    {
      if(!known_keyword.FullMatch(it->first))
        return error_message = "unknown upper case event attribute key '"+it->first+"'", false ;
      if(it->first=="APPLICATION")
      {
        app_name_found = true ;
        if(!app_name.FullMatch(it->second))
          return error_message = "invalid application name '"+it->second+"'", false ;
      }
    }
  }

  if(empty_only || app_name_found)
    return true ;
  else
    return error_message = "no application name given", false ;
}

event_t *event_t::from_dbus_iface(const event_io_t *eio)
{
  event_t *e = new event_t ;
  e->ticker = ticker_t(eio->ticker) ;
  e->t = broken_down_t(eio->t_year, eio->t_month, eio->t_day, eio->t_hour, eio->t_minute) ;
  e->tz = eio->t_zone.toStdString() ;
  e->flags = eio->flags & EventFlags::Client_Mask ;
  e->tsz_max = eio->tsz_max ;

#define check_attr(txt, a, empty) \
  do \
  { \
    string message = txt ; \
    if(!check_attributes(message, a, empty)) \
    { \
      log_error("a new event is rejected: %s", message.c_str()) ; \
      delete e ; \
      return NULL ; \
    } \
  } \
  while(0)

  map_q_to_std(eio->attr.txt, e->attr.txt) ;
  check_attr("event attributes", e->attr, false) ;

  int R = eio->recrs.size() ;
  e->recrs.resize(R) ;
  for(int i=0; i<R; ++i)
    copy_recr(eio->recrs[i], e->recrs[i]) ;

  int A = eio->actions.size() ;
  e->actions.resize(A) ;
  for(int i=0; i<A; ++i)
  {
    e->actions[i].flags = eio->actions[i].flags ;
    map_q_to_std(eio->actions[i].attr.txt, e->actions[i].attr.txt) ;
    e->actions[i].cred_modifier.load_from_dbus_interface(eio->actions[i].cred_modifiers) ;
    check_attr(str_printf("action #%d", i), e->actions[i].attr, true) ;
  }
  e->cred_modifier.load_from_dbus_interface(eio->cred_modifiers) ;

  int B = eio->buttons.size() ;
  e->snooze.resize(B+1) ;
  e->snooze[0] = eio->tsz_length < 10 ? 1 : eio->tsz_length ;
  e->b_attr.resize(B) ;
  for(int i=0; i<B; ++i)
  {
    unsigned si = eio->buttons[i].snooze ;
    if(si>1 && si<10) // XXX: make it runtime option (10/30sec) ?
      si = 10 ;
    e->snooze[i+1] = si ;
    map_q_to_std(eio->buttons[i].attr.txt, e->b_attr[i].txt) ;
    check_attr(str_printf("button #%d", i), e->b_attr[i], true) ;
  }

  unsigned _ctzp = (e->has_ticker() << 12) | (e->has_time() << 8) | (e->has_timezone() << 4) | (e->has_recurrence() << 0 ) ;
  bool ok = false ;
  switch(_ctzp)
  {
#define _ok(msg) log_info("new event accepted, %s", msg) ; ok = true ; break ;
#define _not_ok(msg) log_error("a new event is rejected: %s", msg) ; break ;
    case 0x0000:
    case 0x0010:
      if(e->to_be_keeped())
      {
        _ok("fake event: to be keeped, never triggers") ;
      }
      else
      {
        _not_ok("neither epoch time, nor broken down tome, nor recurrence is given") ;
      }
    case 0x1100:
    case 0x1101:
    case 0x1110:
    case 0x1111: _not_ok("both epoch time and broken down time are given") ;
    case 0x1010: _not_ok("both epoch time and timezone are given") ;

    case 0x1000: _ok("epoch time is given") ;
    case 0x1001:
    case 0x1011: _ok("epoch time and recurrence pattern are given") ;
    case 0x0100: _ok("broken down time is given") ;
    case 0x0110: _ok("broken down time and the timezone are given") ;
    case 0x0101: _ok("broken down time and the recurrence pattern are given") ;
    case 0x0111: _ok("broken down time, timezone and the recurrence pattern are given") ;
    case 0x0001: _ok("recurrence pattern is given") ;
    case 0x0011: _ok("timezone and the recurrence pattern are given") ;
#undef _ok
#undef _not_ok
  }

  if(e->has_recurrence())
  {
    ostringstream os ;
    for(unsigned i=0; i<e->recrs.size(); ++i)
      os << "{" << str_printf("wday=0%o hour=0x%x mins=0x%llx", e->recrs[i].hour, e->recrs[i].mday, e->recrs[i].mins) << "}" ;
    log_debug("recrs: %s", os.str().c_str()) ;
  }

  if(ok)
    return e ;
  else
  {
    delete e ;
    return NULL ;
  }
}

iodata::bit_codec *event_t::codec ;
iodata::bit_codec *action_t::codec ;
iodata::bit_codec *recurrence_pattern_t::mins_codec ;
iodata::bit_codec *recurrence_pattern_t::hour_codec ;
iodata::bit_codec *recurrence_pattern_t::mday_codec ;
iodata::bit_codec *recurrence_pattern_t::wday_codec ;
iodata::bit_codec *recurrence_pattern_t::mons_codec ;

static string lowercase(const char *str)
{
  string res(str) ;
  int len=strlen(str) ;
  for(int i=0; i<len; ++i)
    res[i] = tolower(res[i]) ;
  return res ;
}

void event_t::codec_initializer()
{
  log_debug() ;
  event_t::codec = new iodata::bit_codec ;
  action_t::codec = new iodata::bit_codec ;
  recurrence_pattern_t::mins_codec = new iodata::bit_codec ;
  recurrence_pattern_t::hour_codec = new iodata::bit_codec ;
  recurrence_pattern_t::mday_codec = new iodata::bit_codec ;
  recurrence_pattern_t::wday_codec = new iodata::bit_codec ;
  recurrence_pattern_t::mons_codec = new iodata::bit_codec ;
#define REG(codec_ptr, space, name) ((codec_ptr)->register_name(space::name, lowercase(#name)))
  REG(event_t::codec, EventFlags, Alarm) ;
  REG(event_t::codec, EventFlags, Need_Connection) ;
  REG(event_t::codec, EventFlags, Trigger_If_Missed) ;
  REG(event_t::codec, EventFlags, Postpone_If_Missed) ;
  REG(event_t::codec, EventFlags, User_Mode) ;
  REG(event_t::codec, EventFlags, Aligned_Snooze) ;
  REG(event_t::codec, EventFlags, Boot) ;
  REG(event_t::codec, EventFlags, Reminder) ;
  REG(event_t::codec, EventFlags, Suppress0) ;
  REG(event_t::codec, EventFlags, Hide1) ;
  REG(event_t::codec, EventFlags, Hide2) ;
  REG(event_t::codec, EventFlags, Keep_Alive) ;
  REG(event_t::codec, EventFlags, Single_Shot) ;
  REG(event_t::codec, EventFlags, Backup) ;
  REG(event_t::codec, EventFlags, Snoozing) ;
  REG(event_t::codec, EventFlags, Recurring) ;
  REG(event_t::codec, EventFlags, Empty_Recurring) ;
  REG(event_t::codec, EventFlags, In_Dialog) ;
  REG(event_t::codec, EventFlags, Missed) ;

  REG(action_t::codec, ActionFlags, Send_Cookie) ;
  REG(action_t::codec, ActionFlags, Send_Event_Attributes) ;
  REG(action_t::codec, ActionFlags, Send_Action_Attributes) ;
  REG(action_t::codec, ActionFlags, Run_Command) ;
  REG(action_t::codec, ActionFlags, DBus_Method) ;
  REG(action_t::codec, ActionFlags, DBus_Signal) ;
  REG(action_t::codec, ActionFlags, Use_System_Bus) ;

  REG(action_t::codec, ActionFlags, State_Triggered) ;
  REG(action_t::codec, ActionFlags, State_Queued) ;
  REG(action_t::codec, ActionFlags, State_Missed) ;
  REG(action_t::codec, ActionFlags, State_Tranquil) ;
  REG(action_t::codec, ActionFlags, State_Finalized) ;
  REG(action_t::codec, ActionFlags, State_Due) ;
  REG(action_t::codec, ActionFlags, State_Snoozed) ;
  REG(action_t::codec, ActionFlags, State_Served) ;
  REG(action_t::codec, ActionFlags, State_Aborted) ;
  REG(action_t::codec, ActionFlags, State_Failed) ;
#undef REG
  for(unsigned i=0; i<=Maemo::Timed::Number_of_Sys_Buttons; ++i)
    action_t::codec->register_name(ActionFlags::sys_button(i), QString("sys_button_%1").arg(i).toStdString()) ;
  for(unsigned i=1; i<=Maemo::Timed::Max_Number_of_App_Buttons; ++i)
    action_t::codec->register_name(ActionFlags::app_button(i), QString("app_button_%1").arg(i).toStdString()) ;

  static const char *mon_names[12]={"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"} ;
  for(unsigned m=0; m<12; ++m)
    recurrence_pattern_t::mons_codec->register_name(1<<m, mon_names[m]) ;
  recurrence_pattern_t::mons_codec->register_name((1<<12)-1, "every_month") ;

  static const char *day_names[7]={"sun","mon","tue","wed","thu","fri","sat"} ;
  for(unsigned d=0; d<7; ++d)
    recurrence_pattern_t::wday_codec->register_name(1<<d, day_names[d]) ;
  recurrence_pattern_t::wday_codec->register_name((1<<7)-1, "whole_week") ;

  for(unsigned d=1; d<=31; ++d)
    recurrence_pattern_t::mday_codec->register_name(1<<d, str_printf("d%02d", d)) ;
  recurrence_pattern_t::mday_codec->register_name(1<<0, "last_day") ;
  recurrence_pattern_t::mday_codec->register_name((uint32_t)~1, "whole_month") ;

  for(unsigned m=0; m<60; ++m)
    recurrence_pattern_t::mins_codec->register_name((uint64_t)1<<m, str_printf("m%02d", m)) ;
  recurrence_pattern_t::mins_codec->register_name(((uint64_t)1<<60)-1, "every_minute") ;

  for(unsigned h=0; h<24; ++h)
    recurrence_pattern_t::hour_codec->register_name(1<<h, str_printf("h%02d", h)) ;
  recurrence_pattern_t::hour_codec->register_name((1<<24)-1, "every_hour") ;
  log_debug() ;
}

iodata::record *attribute_t::save() const
{
  iodata::record *r = new iodata::record ;
  iodata::array *a = new iodata::array ; // XXX what if txt is empty?
  for(map<string,string>::const_iterator it=txt.begin(); it!=txt.end(); ++it)
  {
    a->add(new iodata::bytes(it->first)) ;
    a->add(new iodata::bytes(it->second)) ;
  }
  r->add("txt", a) ;
  return r ;
}

void attribute_t::load(const iodata::record *r)
{
  const iodata::array *a = r->get("txt")->arr() ;
  // need some warning for odd length array ?
  for(unsigned i=0; i<a->size()/2; ++i)
  {
    unsigned j_key = 2*i, j_val = j_key + 1 ;
    string key = a->get(j_key)->str() ;
    string val = a->get(j_val)->str() ;
    txt[key]=val ;
  }
}

set<string> cred_modifier_t::tokens_by_value(bool value) const
{
  set<string> res ;
  for (map<string,bool>::const_iterator it=tokens.begin(); it!=tokens.end(); ++it)
    if (it->second==value)
      res.insert(it->first) ;
  return res ;
}

iodata::array *cred_modifier_t::save() const
{
  iodata::array *a = new iodata::array ;
  for(map<string,bool>::const_iterator it=tokens.begin(); it!=tokens.end(); ++it)
  {
    iodata::record *r = new iodata::record ;
    r->add("token", it->first) ;
    r->add("accrue", !! it->second) ;
    a->add(r) ;
  }
  return a ;
}

void cred_modifier_t::load(const iodata::array *a)
{
  for(unsigned i=0; i<a->size(); ++i)
  {
    const iodata::record *r = a->get(i)->rec() ;
    const string &token = r->get("token")->str() ;
    tokens[token] = !! r->get("accrue")->value() ;
  }
}

void cred_modifier_t::load_from_dbus_interface(const QVector<Maemo::Timed::cred_modifier_io_t> &cmio)
{
  for(int i=0; i<cmio.size(); ++i)
  {
    string token = string_q_to_std(cmio[i].token) ;
    if(tokens.find(token)!=tokens.end())
      log_warning("duplicate token modifier '%s'", token.c_str()) ;
    tokens[token] = cmio[i].accrue ;
  }
}

iodata::record *recurrence_pattern_t::save() const
{
  iodata::record *r = new iodata::record ;
  r->add("mins", mins, mins_codec) ; // XXX: 0-59, not zero
  r->add("hour", hour, hour_codec) ; // XXX: 0-23, not zero
  r->add("mday", mday, mday_codec) ; // XXX: 1-31, not zero
  r->add("wday", wday, wday_codec) ; // XXX: whole week, not zero
  r->add("mons", mons, mons_codec) ; // XXX: any month, not zero
  return r ;
}

void recurrence_pattern_t::load(const iodata::record *r)
{
  mins = r->get("mins")->decode(mins_codec) ;
  hour = r->get("hour")->decode(hour_codec) ;
  mday = r->get("mday")->decode(mday_codec) ;
  wday = r->get("wday")->decode(wday_codec) ;
  mons = r->get("mons")->decode(mons_codec) ;
}

string action_t::cred_key() const
{
  if (cred_key_value.empty())
  {
    ostringstream os ;
    os << "tokens={" ;
    bool first = true ;
    for (map<string,bool>::const_iterator it=cred_modifier.tokens.begin(); it!=cred_modifier.tokens.end(); ++it)
    {
      os << (first?first=false,"":", ") ;
      os << (it->second?"+":"-") ;
      os << it->first ;
    }
    os << "}" ;
    string uid = attr("USER"), gid = attr("GROUP") ;
    if (!uid.empty())
      os << ", uid=" << uid ;
    if (!gid.empty())
      os << ", gid=" << gid ;

    cred_key_value = os.str() ;
  }
  return cred_key_value ;
}

iodata::record *action_t::save() const
{
  iodata::record *r = new iodata::record ;
  r->add("attr", attr.save()) ;
  r->add("flags", new iodata::bitmask(flags, codec)) ;
  r->add("cred_modifier", cred_modifier.save()) ;
  return r ;
}

void action_t::load(const iodata::record *r)
{
  attr.load(r->get("attr")->rec()) ;
  flags = r->get("flags")->decode(codec) ;
  cred_modifier.load(r->get("cred_modifier")->arr()) ;
}
