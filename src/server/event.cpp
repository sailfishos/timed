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
#include <sstream>

#include <pcrecpp.h>

#include "../common/log.h"
#include "../lib/aliases.h"
#include "misc.h"
#include "machine.h"
#include "timed.h"
#include "event.h"
#include "f.h"

using namespace Maemo::Timed ;

event_t::event_t()
{
  flags = 0 ;
  tsz_max = tsz_counter = 0 ;
  cred_modifier = NULL ;
  client_creds = NULL ;
  state = NULL ;
  request_watcher = NULL ;
  cookie = cookie_t(0) ; // paranoid
  trigger_if_missed = false;
}

event_t::~event_t()
{
  delete cred_modifier ;
  delete client_creds ;
  if(request_watcher)
  {
    log_debug("request_watcher still alive, detaching") ;
    request_watcher->detach(this) ;
  }
}

static void copy_recr(const recurrence_io_t &from, recurrence_pattern_t &to)
{
  to.mins = from.mins & ((1LL<<60)-1) ;
  to.hour = from.hour & ((1<<24)-1) ;
  to.mday = from.mday ;
  to.wday = from.wday & ((1<< 7)-1) ;
  to.mons = from.mons & ((1<<12)-1) ;
  to.flags = from.flags ;
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
    if (eio->actions[i].cred_modifiers.size()>0)
      e->actions[i].cred_modifier = new cred_modifier_t(eio->actions[i].cred_modifiers) ;
    check_attr(str_printf("action #%d", i), e->actions[i].attr, true) ;
  }
  if (eio->cred_modifiers.size()>0)
    e->cred_modifier = new cred_modifier_t(eio->cred_modifiers) ;

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

  if (e->has_timezone() and not Maemo::Timed::is_tz_name(e->tz))
  {
    log_error("a new event is rejected because of invalid time zone: '%s'", e->tz.c_str()) ;
    delete e ;
    return NULL ;
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

void event_t::to_dbus_iface(const event_t &event, Maemo::Timed::event_io_t &res)
{
  res.ticker = event.ticker.value() ;
  res.t_year = event.t.year ;
  res.t_month = event.t.month ;
  res.t_day = event.t.day ;
  res.t_hour = event.t.hour ;
  res.t_minute = event.t.minute ;
  map_std_to_q(event.attr.txt, res.attr.txt) ;
  res.flags = event.flags & EventFlags::Client_Mask ;
  res.tsz_max = event.tsz_max ;
  res.tsz_length = event.snooze[0] ;
  if(event.cred_modifier)
  {
    res.cred_modifiers = event.cred_modifier->to_q_vector() ;
  }
  int A = event.actions.size() ;
  for(int i = 0; i < A; ++i)
  {
    Maemo::Timed::action_io_t aio;
    const action_t &action = event.actions[i] ;
    map_std_to_q(action.attr.txt, aio.attr.txt) ;
    aio.flags = action.flags ;
    if(action.cred_modifier)
    {
      aio.cred_modifiers = action.cred_modifier->to_q_vector() ;
    }
    res.actions << aio ;
  }
  int B = event.b_attr.size() ;
  for(int i = 0; i < B; ++i)
  {
    Maemo::Timed::button_io_t bio;
    map_std_to_q(event.b_attr[i].txt, bio.attr.txt) ;
    bio.snooze = event.snooze[i+1] ;
    res.buttons << bio ;
  }
  int R = event.recrs.size() ;
  for(int i = 0; i < R; ++i)
  {
    Maemo::Timed::recurrence_io_t rio;
    const recurrence_pattern_t &recurrence = event.recrs[i] ;
    rio.mons = recurrence.mons ;
    rio.mday = recurrence.mday ;
    rio.wday = recurrence.wday ;
    rio.hour = recurrence.hour ;
    rio.mins = recurrence.mins ;
    rio.flags = recurrence.flags ;
    res.recrs << rio ;
  }
}

iodata::bit_codec *event_t::codec ;
iodata::bit_codec *action_t::codec ;
iodata::bit_codec *recurrence_pattern_t::codec ; // flags
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
  recurrence_pattern_t::codec = new iodata::bit_codec ;
  recurrence_pattern_t::mins_codec = new iodata::bit_codec ;
  recurrence_pattern_t::hour_codec = new iodata::bit_codec ;
  recurrence_pattern_t::mday_codec = new iodata::bit_codec ;
  recurrence_pattern_t::wday_codec = new iodata::bit_codec ;
  recurrence_pattern_t::mons_codec = new iodata::bit_codec ;
#define REG(codec_ptr, space, name) ((codec_ptr)->register_name(space::name, lowercase(#name)))
  REG(event_t::codec, EventFlags, Alarm) ;
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

  REG(recurrence_pattern_t::codec, RecurrenceFlags, Fill_Gaps) ;
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
    recurrence_pattern_t::mins_codec->register_name((qulonglong)1<<m, str_printf("m%02d", m)) ;
  recurrence_pattern_t::mins_codec->register_name(((qulonglong)1<<60)-1, "every_minute") ;

  for(unsigned h=0; h<24; ++h)
    recurrence_pattern_t::hour_codec->register_name(1<<h, str_printf("h%02d", h)) ;
  recurrence_pattern_t::hour_codec->register_name((1<<24)-1, "every_hour") ;
  log_debug() ;
}

void event_t::codec_destructor()
{
  delete event_t::codec ;
  delete action_t::codec ;
  delete recurrence_pattern_t::codec ;
  delete recurrence_pattern_t::mins_codec ;
  delete recurrence_pattern_t::hour_codec ;
  delete recurrence_pattern_t::mday_codec ;
  delete recurrence_pattern_t::wday_codec ;
  delete recurrence_pattern_t::mons_codec ;
}

iodata::array *attribute_t::save() const
{
  iodata::array *a = new iodata::array ; // XXX what if txt is empty?
  for(map<string,string>::const_iterator it=txt.begin(); it!=txt.end(); ++it)
  {
    iodata::record *r = new iodata::record ;
    r->add("key", new iodata::bytes(it->first)) ;
    r->add("val", new iodata::bytes(it->second)) ;
    a->add(r) ;
  }
  return a ;
}

void attribute_t::load(const iodata::array *a)
{
  for(unsigned i=0; i<a->size(); ++i)
  {
    const iodata::record *r = a->get(i)->rec() ;
    string key = r->get("key")->str() ;
    string val = r->get("val")->str() ;
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

QVector<Maemo::Timed::cred_modifier_io_t> cred_modifier_t::to_q_vector() const
{
  QVector<Maemo::Timed::cred_modifier_io_t> res ;
  map<string, bool>::const_iterator it;
  for(it = tokens.begin(); it != tokens.end(); ++it)
  {
    cred_modifier_io_t crio ;
    crio.token = it->first.c_str() ;
    crio.accrue = it->second ;
    res << crio ;
  }
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

cred_modifier_t::cred_modifier_t(const iodata::array *a)
{
  for(unsigned i=0; i<a->size(); ++i)
  {
    const iodata::record *r = a->get(i)->rec() ;
    const string &token = r->get("token")->str() ;
    tokens[token] = !! r->get("accrue")->value() ;
  }
}

cred_modifier_t::cred_modifier_t(const QVector<Maemo::Timed::cred_modifier_io_t> &cmio)
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
  r->add("flags", flags, codec) ;
  return r ;
}

void recurrence_pattern_t::load(const iodata::record *r)
{
  mins = r->get("mins")->decode(mins_codec) ;
  hour = r->get("hour")->decode(hour_codec) ;
  mday = r->get("mday")->decode(mday_codec) ;
  wday = r->get("wday")->decode(wday_codec) ;
  mons = r->get("mons")->decode(mons_codec) ;
  flags = r->get("flags")->decode(codec) ;
}

const char *action_t::cred_key() const
{
  if (cred_key_value==NULL)
  {
    ostringstream os ;
    if (cred_modifier!=NULL)
    {
      os << "tokens={" ;
      bool first = true ;
      for (map<string,bool>::const_iterator it=cred_modifier->tokens.begin(); it!=cred_modifier->tokens.end(); ++it)
      {
        os << (first?first=false,"":", ") ;
        os << (it->second?"+":"-") ;
        os << it->first ;
      }
      os << "}" ;
    }
    string uid = attr("USER"), gid = attr("GROUP") ;
    if (!uid.empty())
      os << ", uid=" << uid ;
    if (!gid.empty())
      os << ", gid=" << gid ;

    cred_key_value = new string(os.str()) ;
  }
  return cred_key_value->c_str() ;
}

iodata::record *action_t::save() const
{
  iodata::record *r = new iodata::record ;
  r->add("attr", attr.save()) ;
  r->add("flags", new iodata::bitmask(flags, codec)) ;
  if (cred_modifier!=NULL)
    r->add("cred_modifier", cred_modifier->save()) ;
  return r ;
}

void action_t::load(const iodata::record *r)
{
  attr.load(r->get("attr")->arr()) ;
  flags = r->get("flags")->decode(codec) ;
  const iodata::array *cred_modifier = r->get("cred_modifier")->arr() ;
  if (cred_modifier->size()>0)
    this->cred_modifier = new cred_modifier_t(cred_modifier) ;
}

void event_t::sort_and_run_actions(uint32_t mask)
{
  if (mask==0) // TODO: check, if this check is not yet done on state_* layer...
    return ;

  // don't want to call unnecesary constructor here
  vector <unsigned> *r = NULL ;

  for (unsigned i=0; i<actions.size(); ++i)
    if(actions[i].flags & mask) // something to run
    {
      if (r==NULL)
        r = new vector <unsigned> ;
      r->push_back(i) ;
    }

  if (r==NULL) // nothing to do
    return ;

  log_assert(r) ;

  log_debug("Beginning the action list dump") ;
  for(vector<unsigned>::const_iterator it=r->begin(); it!=r->end(); ++it)
    log_debug("Action %d, cred_key: '%s'", *it, actions[*it].cred_key()) ;
  log_debug("Done:     the action list dump") ;

  log_debug("Beginning the action array sorting") ;
  sort(r->begin(), r->end(), action_comparison_t(this)) ;
  log_debug("Done:     the action array sorting") ;

  unsigned start_index = ~0 ; // invalid value
  string current_key ; // empty key (invalid?)
  for(unsigned i=0, have_exec=false; i < r->size(); ++i)
  {
    action_t &a = actions[(*r) [i]] ;
    bool i_exec = a.flags & ActionFlags::Run_Command ; // current action is an exec action
    bool start_now = a.cred_key() != current_key || (have_exec && i_exec) ; // either new key or second exec
    if (start_now && i>0)
      run_actions(*r, start_index, i) ;
    if (start_now || i==0) // i==0 is probably redundant, as cred_key should never be empty
    {
      have_exec = i_exec ;
      start_index = i ;
      current_key = a.cred_key() ;
    }
  }

  log_assert(r->size()>0) ; // to be sure for the run_actions() call below
  run_actions(*r, start_index, r->size()) ;

  delete r ;
}

void event_t::run_actions(const vector<unsigned> &acts, unsigned begin, unsigned end)
{
  log_debug("begin=%d, end=%d, actions: %s", begin, end, print_vector(acts, begin, end).c_str()) ;
  const action_t &a_begin = actions [acts[begin]] ;

  // we want to detect, it the exeption was thrown in the daemon itself or in a child
  pid_t daemon = getpid() ;

  try
  {
    if (fork_and_set_credentials_v3(a_begin)>0) // parent
      return ;
  }
  catch(const event_exception &e)
  {
    log_error("action list %d%s failed: %s", cookie.value(), print_vector(acts, begin, end).c_str(), e.message.c_str()) ;
    pid_t process = e.pid() ;
    if (process!=daemon)
    {
      log_info("terminating child (pid=%d) of daemon (pid=%d)", process, daemon) ;
      ::exit(1) ;
    }
    else // main daemon process
      return ;
  }

  // now we're running in the child with proper credentials, so let's execute dbus stuff !

  unsigned exec_action_i = ~0 ; // not found yet
  QDBusConnection *conn[2] = {NULL, NULL} ; // 0->system, 1->session
  const QString cname = "timed-private" ;

  int dbus_fail_counter = 0 ;

  for (unsigned i=begin; i<end; ++i)
  {
    const unsigned acts_i = acts[i] ;
    const action_t &a = actions [acts_i] ;

    if (a.flags & ActionFlags::Run_Command)
      exec_action_i = acts_i ;

    if((a.flags & ActionFlags::DBus_Action) == 0)
      continue ;

    try
    {
      // set up message to be sent
      QString path = string_std_to_q(find_action_attribute("DBUS_PATH", a)) ;
      QString ifac = string_std_to_q(find_action_attribute("DBUS_INTERFACE", a, (a.flags & ActionFlags::DBus_Signal)!=0)) ;

      QDBusMessage message ;

      if (a.flags & ActionFlags::DBus_Method)
      {
        QString serv = string_std_to_q(find_action_attribute("DBUS_SERVICE", a)) ;
        QString meth = string_std_to_q(find_action_attribute("DBUS_METHOD", a)) ;
        message = QDBusMessage::createMethodCall(serv, path, ifac, meth) ;
      }
      else
      {
        QString sgnl = string_std_to_q(find_action_attribute("DBUS_SIGNAL", a)) ;
        message = QDBusMessage::createSignal(path, ifac, sgnl) ;
      }

      QMap<QString,QVariant> param ;

      if (a.flags & ActionFlags::Send_Cookie)
        param["COOKIE"] = QString("%1").arg(cookie.value()) ;

      if (flags & EventFlags::Trigger_When_Adjusting)
        param["ADJUSTMENT"] = QString::fromStdString(state->machine->transition_time_adjustment.str()) ;

      if (a.flags & ActionFlags::Send_Event_Attributes)
        add_strings(param, attr.txt) ;

      if (a.flags & ActionFlags::Send_Action_Attributes)
        add_strings(param, a.attr.txt) ;

      message << QVariant::fromValue(param) ;

      int bus = (a.flags & ActionFlags::Use_System_Bus) ? 0 : 1 ;
      QDBusConnection * &c = conn[bus] ;

      log_debug() ;
      if (c==NULL) // not used yes, have to create object and connect
      {
        if (bus==0) // system bus
          c = new QDBusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, cname)) ;
        else
        {
          c = new QDBusConnection(QDBusConnection::sessionBus());
        }
      }
      log_debug() ;

      if (c->send(message))
        log_debug("%u[%d]: D-Bus Message sent", cookie.value(), acts_i) ;
      else
        throw event_exception(c->lastError().message().toStdString().c_str()) ;
    }
    catch(const event_exception &e)
    {
      log_error("%u[%d]: dbus-action not executed: %s", cookie.value(), acts_i, e.message.c_str()) ;
      ++ dbus_fail_counter ;
    }
  }

  for(int c=0; c<2; ++c)
    if(QDBusConnection *cc = conn[c])
    {
      // as we are about to exit immediately after queuing
      // and there seems to be no way to flush the connection
      // and be sure that we have actually transmitted the
      // message -> do a dummy synchronous query from dbus
      // daemon and hope that is enough to get the actual
      // message to be delivered ...
      QString connection_name  = cc->baseService() ;

      // it should be us (either pid or uid dependin on feature set) ...
      log_debug("pid=%d, ruid=%d, euid=%d", getpid(), getuid(), geteuid()) ;
      QDBusConnection::disconnectFromBus(connection_name) ;
      delete cc ;
    }

  if(dbus_fail_counter)
    log_warning("failed to exeute %d out of %u dbus actions", dbus_fail_counter, end-begin) ;

  if(exec_action_i == ~0u) // no exec action in this action package
    ::exit(dbus_fail_counter<100 ? dbus_fail_counter : 100) ; // exit value can't be toooo large

  // All the dbus actions are done. Now we have to execute the only command line action

  const action_t &a = actions [exec_action_i] ;

  string cmd = find_action_attribute("COMMAND", a) ;
  if(a.flags & ActionFlags::Send_Cookie)
  {
    log_debug("cmd='%s', COOKIE to be replaced by value", cmd.c_str()) ;
    using namespace pcrecpp ;
    static RE exp("(<COOKIE>)|\\b(COOKIE)\\b", UTF8()) ;
    ostringstream decimal ;
    decimal << cookie.value() ;
    exp.GlobalReplace(decimal.str(), &cmd);
    log_debug("cmd='%s', COOKIE replaced", cmd.c_str()) ;
  }
  if (flags & EventFlags::Trigger_When_Adjusting)
  {
    string adj = state->machine->transition_time_adjustment.str() ;
    log_debug("cmd='%s', ADJUSTMENT to be replaced by '%s'", cmd.c_str(), adj.c_str()) ;
    using namespace pcrecpp ;
    static RE exp("(<ADJUSTMENT>)|\\b(ADJUSTMENT)\\b", UTF8()) ;
    exp.GlobalReplace(adj, &cmd);
    log_debug("cmd='%s', ADJUSTMENT replaced", cmd.c_str()) ;
  }
  log_info("execuing command line action %u[%d]: '%s'", cookie.value(), exec_action_i, cmd.c_str());
  execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), NULL) ;
  log_error("execl(/bin/sh -c '%s') failed: %m", cmd.c_str());

  ::exit(101) ; // use dbus_fail_counter here as well?
}

bool event_t::accrue_privileges(const action_t &a)
{
  credentials_t creds = credentials_t::from_current_process() ;

#if F_TOKENS_AS_CREDENTIALS
  const cred_modifier_t *E = cred_modifier, *A = a.cred_modifier ;

  // tokens_to_accrue1 := EVENT.add - ACTION.drop
  set<string> tokens_to_accrue1 ;
  if (E)
    tokens_to_accrue1 = E->tokens_by_value(true) ;
  if (E and A)
    set_change<string> (tokens_to_accrue1, A->tokens_by_value(false), false) ;

  // tokens_to_accrue2 := ACTION.add - EVENT.drop
  set<string> tokens_to_accrue2 ;
  if (A)
    tokens_to_accrue2 = A->tokens_by_value(true) ;
  if (A and E)
    set_change<string> (tokens_to_accrue2, E->tokens_by_value(false), false) ;

  // creds += (tokens_to_accrue 1+2)
  set_change<string> (creds.tokens, tokens_to_accrue1, true) ;
  set_change<string> (creds.tokens, tokens_to_accrue2, true) ;
#endif // F_TOKENS_AS_CREDENTIALS

  string uid = find_action_attribute("USER", a, false) ;
  string gid = find_action_attribute("GROUP", a, false) ;

  if (!uid.empty())
    creds.uid = uid ;
  if (!gid.empty())
    creds.gid = gid ;

  return creds.apply_and_compare() ;
}

bool event_t::drop_privileges(const action_t &a)
{
  log_assert(client_creds!=NULL) ;
  credentials_t creds = *client_creds ;

#if F_TOKENS_AS_CREDENTIALS
  const cred_modifier_t *E = cred_modifier, *A = a.cred_modifier ;

  // tokens_to_remove1 := EVENT.drop - ACTION.add
  set<string> tokens_to_remove1 ;
  if (E)
    tokens_to_remove1 = E->tokens_by_value(false) ;
  if (E and A)
    set_change<string> (tokens_to_remove1, A->tokens_by_value(true), false) ;

  // tokens_to_remove2 := ACTION.drop - EVENT.add
  set<string> tokens_to_remove2 ;
  if (A)
    tokens_to_remove2 = A->tokens_by_value(false) ;
  if (A and E)
    set_change<string> (tokens_to_remove2, E->tokens_by_value(true), false) ;

  // creds := client_creds - (tokens_to_remove 1+2)
  set_change<string> (creds.tokens, tokens_to_remove1, false) ;
  set_change<string> (creds.tokens, tokens_to_remove2, false) ;
#else
  Q_UNUSED(a);
#endif // F_TOKENS_AS_CREDENTIALS

  return creds.apply_and_compare() ;
}

string event_t::find_action_attribute(const string &key, const action_t &a, bool exc)
{
  string value = a.attr(key) ;
  if(value.empty())
    value = attr(key) ;
  if(value.empty() && exc)
    throw event_exception(string("empty attribute '")+key+"'") ;
  return value ;
}

void event_t::add_strings(QMap<QString, QVariant> &x, const map<string,string> &y)
{
  for(map<string,string>::const_iterator it=y.begin(); it!=y.end(); ++it)
    x.insert(string_std_to_q(it->first), QVariant::fromValue(string_std_to_q(it->second))) ;
}

pid_t event_t::fork_and_set_credentials_v3(const action_t &action)
{
  log_debug("forking for action execution") ;
  pid_t pid = fork() ;
  log_debug("fork() returned %d", pid) ;

  if (pid<0) // can't fork
  {
    log_error("fork() failed: %m") ;
    throw event_exception("can't fork");
  }
  else if (pid>0) // parent
  {
    log_info("forked successfully, child pid: '%d'", pid) ;
    state->machine->emit_child_created(cookie.value(), pid) ;
    return pid ;
  }
  else // child
  {
    log_info("event [%u]: in child process", cookie.value()) ;

    if (setsid() < 0) // detach from session
    {
      log_error("setsid() failed: %m") ;
      throw event_exception("can't detach from session") ;
    }

    log_debug() ;
    if (!drop_privileges(action))
      throw event_exception("can't drop privileges") ;

    log_debug() ;
    if (!accrue_privileges(action))
      log_warning("can't accrue privileges, still continuing") ;

    // Skipping all the uid/gid settings, because it's part of
    // credentials_t::apply() (or it should be at least)

    // TODO: go to home dir _here_ !

    // That's it then, isn't it?

    log_notice("set credentais to %s", credentials_t::from_current_process().str().c_str()) ;
    return pid ;
  }
}

iodata::record *event_t::save(bool for_backup)
{
  iodata::record *r = new iodata::record ;
  r->add("cookie", cookie.value()) ;
  r->add("ticker", ticker.value()) ;
  r->add("t", t.save()) ;
  r->add("tz", new iodata::bytes(tz)) ;
  r->add("attr", attr.save() ) ;
  r->add("flags", new iodata::bitmask(flags &~ EventFlags::Cluster_Mask, codec)) ;
  r->add("recrs", iodata::save(recrs)) ;
  r->add("snooze", iodata::save_int_array(snooze)) ;

  iodata::array *ba = new iodata::array ;
  for(unsigned i=0; i<b_attr.size(); ++i)
  {
    iodata::record *rr = new iodata::record ;
    rr->add("attr", b_attr[i].save()) ;
    ba->add(rr) ;
  }
  r->add("b_attr", ba) ;

  r->add("dialog_time", (flags & EventFlags::In_Dialog) ? last_triggered.value() : 0) ;
  r->add("tsz_max", tsz_max) ;
  r->add("tsz_counter", tsz_counter) ;

  if(not for_backup)
    r->add("actions", iodata::save(actions)) ;
  if(not for_backup and client_creds)
    r->add("client_creds", client_creds->save()) ;
  if(not for_backup and cred_modifier)
    r->add("cred_modifier", cred_modifier->save()) ;
  return r ;
}

bool event_t::compute_recurrence()
{
  log_assert(false, "obsolete function") ;
  return false ;
}

namespace
{
  struct singleton_t
  {
    singleton_t()
    {
      event_t::codec_initializer() ;
    }
   ~singleton_t()
    {
      event_t::codec_destructor() ;
    }
  } singleton ;
}
