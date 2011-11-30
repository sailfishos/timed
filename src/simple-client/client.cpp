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
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusAbstractInterface>
#include <QDBusReply>
#include <QDebug>
#include <QMap>
#include <QString>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
using namespace std ;

#include <pcrecpp.h>

#include <qmlog>

#include <timed/interface>
#include <timed/event>
#include <timed-voland/interface>
#include <timed-voland/ta_interface>

#include "timed/exception.h"
#include "pretty-print.h"

int add_event(const char *title) ;
int response_generic(unsigned cookie, int value) ;
int cancel_event(unsigned cookie) ;
int get_event(unsigned cookie) ;
int send_quit() ;
int query(int ac, char **av) ;
int query_attributes(char *cookie) ;
int queue() ;
int mode(int ac, char **av) ;
int time_settings(int ac, char **av) ;
int alarms(int ac, char **av) ;
int replace(int ac, char **av) ;
int ping_pong() ;
int pid() ;
int register_voland(string str) ;

int parse_data(const char *text) ;
int main_try(int ac, char **av) ;
const char *config_example() ;

int main(int ac, char **av)
{
  qmlog::enable() ;
  try
  {
    int res = main_try(ac,av) ;
    if(res)
      cerr << "Failed, error code: " << res << endl ;
    return res ;
  }
  catch(const string &message)
  {
    cerr << "trown message: " << message << endl ;
  }
  catch(Maemo::Timed::Exception &e)
  {
    cerr << e.what() << endl ;
  }
  catch(...)
  {
    cerr << "oops unknown exception ..." << endl ;
  }
  return 1 ;
}

int main_try(int ac, char **av)
{
  QCoreApplication app(ac, av) ;
  if(ac==2 && (string)av[1]=="a")
  {
    return add_event(NULL) ;
  }
  else if(ac==3 && (string)av[1]=="a")
    return add_event(av[2]) ;
  else if(ac==2 && (string)av[1]=="r") // client r
  {
    return response_generic(0,0) ;
  }
  else if(ac==3 && (string)av[1]=="r") // client r cookie
  {
    unsigned c = atoi(av[2]) ;
    return response_generic(c,0) ;
  }
  else if(ac==4 && (string)av[1]=="r") // client r cookie value
  {
    unsigned c = atoi(av[2]) ;
    int v = atoi(av[3]) ;
    return response_generic(c,v) ;
  }
  else if(ac==3 && (string)av[1]=="c") // client c cookie: cancel the given event
  {
    return cancel_event(atoi(av[2])) ;
  }
  else if(ac==3 && (string)av[1]=="g")
  {
    return get_event(atoi(av[2])) ;
  }
  else if(ac==2 && (string)av[1]=="qn") // client qn: stop notification service
  {
    return send_quit() ;
  }
  else if(ac==2 && (string)av[1]=="ping") // client ping: show the alarm queue
  {
    return ping_pong() ;
  }
  else if(ac==2 && (string)av[1]=="parse") // client parse: parse and print data structure
  {
    return parse_data(config_example()) ;
  }
  else if(ac==3 && (string)av[1]=="parse") // client parse: parse and print data structure
  {
    return parse_data(av[2]) ;
  }
  else if(ac>0 && ac%2==0 && (string)av[1]=="query") // client query [key value ...]
  {
    return query(ac-2, av+2) ;
  }
  else if(ac==3 && (string)av[1]=="query") // client query cookie
  {
    return query_attributes(av[2]) ;
  }
  else if (ac==2 && (string)av[1]=="queue") // print all the attributes
  {
    return queue() ;
  }
  else if ((ac==2 or ac==3) and (string)av[1]=="mode")
  {
    return mode(ac-2, av+2) ;
  }
  else if(ac>1 && (string)av[1]=="date") // client date [time-settings...]
  {
    return time_settings(ac-1, av+1) ;
  }
  else if(ac>1 && (string)av[1]=="alarms") // client date [time-settings...]
  {
    return alarms(ac-2, av+2) ;
  }
  else if(ac==3 && (string)av[1]=="replace") // replace cookie
  {
    return replace(ac-2, av+2) ;
  }
  else if(ac==2 && (string)av[1]=="pid") // pid: print the process ID of the server
  {
    return pid() ;
  }
  else if(ac==3 && (string)av[1]=="voland") // voland: register given voland service on session bus
  {
    return register_voland(av[2]) ;
  }
  else
  {
    qDebug() << "Usage:" << av[0] << "[a [TITLE]] | [r [COOKIE [VALUE]]] | c COOKIE | g COOKIE | qn | query [key value]* | pid" ;
    return 1 ;
  }

}

int cancel_event(unsigned cookie)
{
  Maemo::Timed::Interface iface ;
  if(!iface.isValid())
  {
    qDebug() << "not valid interface:" << iface.lastError() ;
    return 1 ;
  }
  QDBusReply<bool> res = iface.cancel_sync(cookie) ;

  if(!res.isValid())
  {
    qDebug() << "dbus call failed:" << iface.lastError() ;
    return 1 ;
  }

  bool res_value = res.value() ;

  return res_value ? 0 : 1 ;
}

int get_event(unsigned cookie)
{
  Maemo::Timed::Interface iface ;
  if(!iface.isValid())
  {
    qDebug() << "not valid interface:" << iface.lastError() ;
    return 1 ;
  }
  Maemo::Timed::Event::DBusReply res = iface.get_event_sync(cookie) ;

  if(!res.isValid())
  {
    qDebug() << "dbus call failed:" << res.error() ;
    return 1 ;
  }

  qDebug() << "Valid reply received:" ;
  qDebug() << res.value() ;

  return 0 ;
}

int response_generic(unsigned cookie, int value)
{
  Maemo::Timed::Voland::TaInterface iface ;
  if(!iface.isValid())
  {
    qDebug() << "not valid interface:" << iface.lastError() ;
    return 1 ;
  }

  if(cookie==0)
  {
    // try to get the top dialog
    QDBusReply<uint> res = iface.top_sync() ;
    if(!res.isValid())
    {
      qDebug() << "dbus call 'top' failed:" << iface.lastError() ;
      return 1 ;
    }
    cookie = res.value() ;
  }

  QDBusReply<bool> res = iface.answer_sync(cookie, value) ;

  if(!res.isValid())
  {
    qDebug() << "dbus call 'answer' failed:" << iface.lastError() ;
    return 1 ;
  }

  bool res_value = res.value() ;

  return res_value ? 0 : 1 ;
}

int response_last(int value)
{
  return response_generic(0, value) ;
}
int response(unsigned cookie, int value)
{
  return response_generic(cookie, value) ;
}

#define TRIGGER_FILE "/home/user/triggered"

int add_event(const char *title)
{
  bool need_ui = title != NULL ;
  bool recurrence = false ;
  int time_shift = 8 ;

  Maemo::Timed::Event e ;
  Maemo::Timed::Event::Action &a_trig = e.addAction() ;
  a_trig.whenTriggered() ;
  a_trig.setSendCookieFlag() ;
  a_trig.runCommand("echo cookie=[COOKIE]=<COOKIE> TRIGGERED $(date) >> " TRIGGER_FILE) ;

  if(title!=NULL)
  {
    e.setAttribute("TITLE", title) ;

    if(pcrecpp::RE("boot").PartialMatch(title))
      e.setBootFlag() ;

    if(pcrecpp::RE("alarm").PartialMatch(title))
      e.setAlarmFlag() ;

    if(pcrecpp::RE("usermode").PartialMatch(title))
      e.setUserModeFlag() ;

    if(pcrecpp::RE("silent").PartialMatch(title))
      need_ui = false ;

    if(pcrecpp::RE("recur").PartialMatch(title))
      recurrence = true ;

    if(pcrecpp::RE("calendar").PartialMatch(title))
      e.setAttribute("PLUGIN", "libCalendarReminder") ;

    int time_to_wait ;
    if(pcrecpp::RE("/(-?\\d+)/").PartialMatch(title, &time_to_wait))
      time_shift = time_to_wait ;
  }

  if(need_ui)
  {
    e.setAlignedSnoozeFlag() ;

    Maemo::Timed::Event::Button &b1 = e.addButton() ;
    Maemo::Timed::Event::Button &b2 = e.addButton() ;
    Maemo::Timed::Event::Button &b3 = e.addButton() ;
    Maemo::Timed::Event::Button &b4 = e.addButton() ;

    b1.setSnooze(10) ;
    b2.setSnooze(17) ;
    b3.setSnooze(60) ;
    (void)b4 ;
    // b4 doesn't snooze: it closes the dialog

    Maemo::Timed::Event::Action &a1 = e.addAction() ;
    a1.runCommand("echo [COOKIE] BUTTON #1 $(date) >> " TRIGGER_FILE) ;
    a1.setSendCookieFlag() ;
    a1.whenButton(b1) ;

    Maemo::Timed::Event::Action &a2 = e.addAction() ;
    a2.runCommand("echo [COOKIE] BUTTON #2 $(date) >> " TRIGGER_FILE) ;
    a2.setSendCookieFlag() ;
    a2.whenButton(b2) ;

    Maemo::Timed::Event::Action &a3 = e.addAction() ;
    a3.runCommand("echo [COOKIE] BUTTON #3 $(date) >> " TRIGGER_FILE) ;
    a3.setSendCookieFlag() ;
    a3.whenButton(b3) ;

    Maemo::Timed::Event::Action &a4 = e.addAction() ;
    a4.runCommand("echo [COOKIE] BUTTON #4 $(date) >> " TRIGGER_FILE) ;
    a4.setSendCookieFlag() ;
    a4.whenButton(b1) ;

    Maemo::Timed::Event::Action &a_0 = e.addAction() ;
    a_0.runCommand("echo [COOKIE] CANCELED BY USER $(date) >> " TRIGGER_FILE) ;
    a_0.setSendCookieFlag() ;
    a_0.whenSysButton(0) ;

    Maemo::Timed::Event::Action &a_sys1 = e.addAction() ;
    a_sys1.runCommand("echo [COOKIE] FIRST SYSTEM BYTTON $(date) >> " TRIGGER_FILE);
    a_sys1.setSendCookieFlag() ;
    a_sys1.whenSysButton(1) ;

    Maemo::Timed::Event::Action &a_sys2 = e.addAction() ;
    a_sys2.runCommand("echo [COOKIE] SECOND SYSTEM BYTTON $(date) >> " TRIGGER_FILE) ;
    a_sys2.setSendCookieFlag() ;
    a_sys2.whenSysButton(2) ;
  }

  if(recurrence) // add a recurrence at 12:34 every day
  {
    Maemo::Timed::Event::Recurrence &r = e.addRecurrence() ;
    r.everyMonth() ;
    r.everyDayOfMonth() ;
    r.everyDayOfWeek() ;
    r.addHour(12), r.addMinute(34) ;
  }

  e.setTicker(time(NULL)+time_shift) ;
  e.setAttribute("APPLICATION", "simple_client") ;

  Maemo::Timed::Interface ifc ;
  if(!ifc.isValid())
  {
    qDebug() << "not valid interface:" << ifc.lastError() ;
    return 1 ;
  }

  QDBusReply<uint> res = ifc.add_event_sync(e) ;
  if(!res.isValid())
  {
    qDebug() << "call failed:" << res.error().message() ;
    return 1 ;
  }

  qDebug() << "added event, cookie:" << res.value() ;
  return 0 ;
}

int send_quit()
{
  Maemo::Timed::Voland::TaInterface iface ;
  QDBusReply<void> res = iface.quit_sync() ;
  if(!res.isValid())
    qDebug() << "sending quit request failed:" <<iface.lastError() ;
  return res.isValid() ;
}

int query(int ac, char **av)
{
  QMap<QString,QVariant> p ;
  for(int i=0; i<ac; i+=2)
    p[av[i]] = (QString)(av[i+1]) ;

  Maemo::Timed::Interface ifc ;
  QDBusReply<QList<QVariant> > reply = ifc.query_sync(p) ;
  if(!reply.isValid())
  {
    qDebug() << "query call failed" << ifc.lastError() ;
    return 1 ;
  }
  const QList<QVariant> &result = reply.value() ;
  int s = result.size() ;
  cout << "got " << s << " cookie(s)" << ( s ? ": " : "" ) ;
  if(s>0)
    for(int i=0; i<s; ++i)
      cout << (i?", ":"") << result[i].toUInt() ;
  cout << endl ;
  return 0 ;
}

int query_attributes(char *cookie)
{
  string integer = "([0-9]+)" ;
  int value ;
  if(!pcrecpp::RE("([0-9]+)").FullMatch(cookie, &value))
  {
    qDebug() << "invalid integer:" << cookie ;
    return 1 ;
  }
  Maemo::Timed::Interface timed ;
  QDBusReply<QMap<QString,QVariant> > reply = timed.query_attributes_sync(value) ;
  if(reply.isValid())
  {
    QMap<QString,QVariant> x = reply.value() ;
    if(x.isEmpty())
    {
      qDebug() << "empty mapping returned" ;
      return 1 ;
    }
    else
    {
      qDebug() << "returned mapping:" ;
      qDebug() << x ;
      return 0 ;
    }
  }
  else
  {
    qDebug() << "D-Bus call failed:" << timed.lastError() ;
    return 1 ;
  }
}

int ping_pong()
{
  Maemo::Timed::Interface ifc ;
  QDBusReply<QString> reply = ifc.ping_sync() ;
  if(!reply.isValid())
  {
    qDebug() << "ping call failed" << ifc.lastError() ;
    return 1 ;
  }
  QString pong = reply.value() ;
  cout << "---" << endl << pong.toStdString() << "---" << endl ;
  return 0 ;
}

const char *config_example()
{
  return
   "a=-0x45, t=-3 , bbb = { a=2, x=9 }  , c = [ 1,2,3,4,5, \"aa\"+\"ddd\"], d= \"blalbla!,\", /*a \n"
   "ada das dasd sa dasd asd asd  */ xxx=\"aaa\", e= $foo| $ 30 |  $bu|$239  . " ;
}

int parse_data(const char *text)
{
  Maemo::Timed::Interface ifc ;
  QDBusReply<QString> reply = ifc.parse_sync(QString(text)) ;
  if(!reply.isValid())
  {
    qDebug() << "parse call failed" << ifc.lastError() ;
    return 1 ;
  }
  QString plain = reply.value() ;
  cout << "---" << endl << plain.toStdString() << "---" << endl ;
  return 0 ;
}

int pid()
{
  Maemo::Timed::Interface ifc ;
  QDBusReply<int> reply = ifc.pid_sync() ;
  if(!reply.isValid())
  {
    qDebug() << "pid call failed" << ifc.lastError() ;
    return 1 ;
  }
  cout << "server pid=" << reply.value() << endl ;
  return 0 ;
}

int time_settings(int ac, char **av)
{
  Maemo::Timed::Interface timed ;
  Maemo::Timed::WallClock::Settings set ;
  for(int i=1; i<ac; ++i)
  {
    using namespace pcrecpp ;
    static RE key_value = "(format|time|zone|offset)=(.*)" ;
    string integer = "(-?[0-9]+)" ;
    string key, value ;
    if(!key_value.FullMatch(av[i], &key, &value))
    {
      qDebug() << "invalid key/value pair:" << av[i] ;
      return 1 ;
    }
    if(key=="format")
    {
      int f_12_24 ;
      if(!RE(integer).FullMatch(value, &f_12_24))
      {
        qDebug() << "integer format value expected" ;
        return 1 ;
      }
      if(f_12_24==12)
        set.setFlag24(false) ;
      else if(f_12_24==24)
        set.setFlag24(true) ;
      else
      {
        qDebug() << "format value 12 or 24 expected" ;
        return 1 ;
      }
    }
    else if(key=="zone")
    {
      if(value=="auto")
        set.setTimezoneCellular() ;
      else if (value=="manual")
        set.setTimezoneManual("") ;
      else
        set.setTimezoneManual(value.c_str()) ;
    }
    else if(key=="offset")
    {
      if(value=="auto")
        set.setOffsetCellular() ;
      else if (value=="manual")
        set.setOffsetManual() ;
      else
      {
        int off ;
        if(!RE(integer).FullMatch(value, &off))
        {
          qDebug() << "integer offset value expected" ;
          return 1 ;
        }
        if(-15 < off && off < 15)
          off *= 60 ;
        // now offset is in minutes
        set.setOffsetManual(off) ;
      }
    }
    else if(key=="time")
    {
      if(value=="auto")
        set.setTimeNitz() ;
      else if (value=="manual")
        set.setTimeManual() ;
      else // let's make dumb parser of time specification
      {
        struct tm tm ;
        time_t now = time(NULL) ;
        localtime_r(&now, &tm) ;
        int HH,MM,SS, YYYY,MO,DD ;
#define XX "([0-9]{2})"
#define XXXX "([0-9]{4})"
        if(RE(XX":"XX).FullMatch(value, &HH, &MM))
          tm.tm_hour=HH, tm.tm_min=MM ;
        else if(RE(XX":"XX":"XX).FullMatch(value, &HH, &MM, &SS))
          tm.tm_hour=HH, tm.tm_min=MM, tm.tm_sec=SS ;
        else if(RE(XX":"XX","XXXX"-"XX"-"XX).FullMatch(value, &HH, &MM, &YYYY, &MO, &DD))
          tm.tm_hour=HH, tm.tm_min=MM, tm.tm_year=YYYY-1900, tm.tm_mon=MO-1, tm.tm_mday=DD ;
        else if(RE(XX":"XX":"XX","XXXX"-"XX"-"XX).FullMatch(value, &HH, &MM, &SS, &YYYY, &MO, &DD))
          tm.tm_hour=HH, tm.tm_min=MM, tm.tm_sec=SS, tm.tm_year=YYYY-1900, tm.tm_mon=MO-1, tm.tm_mday=DD ;
        else
        {
          qDebug() << "Invalid time value" ;
          return 1 ;
        }
#undef XX
#undef XXXX
        time_t new_time = mktime(&tm) ;
        if(new_time==time_t(-1))
        {
          qDebug() << "oops, can't convert time" ;
          return 1 ;
        }
        set.setTimeManual(new_time) ;
      }
    }
    else
    {
      qDebug() << "time setting key" << key.c_str() << "not implemented" ;
      return 1 ;
    }
  }
  qDebug() << "calling wall_clock_settings_sync" << set.str() ;
  QDBusReply<bool> reply = timed.wall_clock_settings_sync(set) ;
  qDebug() << "wall_clock_settings_sync done" ;
  if(!reply.isValid())
  {
    qDebug() << "pid call failed" << timed.lastError() ;
    return 1 ;
  }
  return reply.value() ? 0 : 1 ;
}

int alarms(int ac, char **av)
{
  Maemo::Timed::Interface timed ;
  if(ac>1)
    return 1 ;
  if(ac>0)
  {
    string on_off = av[0] ;
    bool new_state ;
    if(on_off=="on")
      new_state = true ;
    else if(on_off=="off")
      new_state = false ;
    else
    {
      qDebug() << "invalid argument, on/off required" ;
      return 1 ;
    }
    QDBusReply<void> reply_set = timed.enable_alarms_sync(new_state) ;
    if(!reply_set.isValid())
    {
      qDebug() << "enable_alarms call failed" << timed.lastError() ;
      return 1 ;
    }
  }
  QDBusReply<bool> reply = timed.alarms_enabled_sync() ;
  if(!reply.isValid())
  {
    qDebug() << "alarms_enabled call failed" << timed.lastError() ;
    return 1 ;
  }
  cout << "alarms are " << (reply.value() ? "on" : "off") << endl ;
  return 0 ;
}

int replace(int /* ac */, char **av)
{
  int cookie ;
  if(!pcrecpp::RE("([0-9]+)").FullMatch(av[0], &cookie))
  {
    qDebug() << "invalid integer:" << av[0] ;
    return 1 ;
  }
  QString title = QString("replacement_of_%1").arg(cookie) ;
  Maemo::Timed::Event e ;
  Maemo::Timed::Event::Action &a = e.addAction() ;
  a.whenTriggered() ;
  a.setSendCookieFlag() ;
  a.runCommand(QString("echo cookie=[COOKIE] (%1) TRIGGERED $(date) >> /tmp/aa").arg(title)) ;

  e.setAttribute("TITLE", title) ;
  e.setTicker(time(NULL)+10) ;

  Maemo::Timed::Interface timed ;
  if(!timed.isValid())
  {
    qDebug() << "not valid interface:" << timed.lastError() ;
    return 1 ;
  }

  QDBusReply<uint> res = timed.replace_event_sync(e, cookie) ;
  if(!res.isValid())
  {
    qDebug() << "replace_event call failed:" << res.error().message() ;
    return 1 ;
  }

  qDebug() << "new event cookie:" << res.value() ;
  return 0 ;
}

int register_voland(string str)
{
  if (not strchr(str.c_str(), '/'))
    str = (string) "/usr/bin/" + str ;
  const char *path = "/usr/share/dbus-1/services/com.nokia.voland.service" ;
  if (FILE *fp = fopen(path, "w"))
  {
    fprintf(fp, "[D-BUS Service]\n") ;
    fprintf(fp, "Name=com.nokia.voland\n") ;
    fprintf(fp, "Exec=%s\n", str.c_str()) ;
    if (fclose(fp)<0)
    {
      log_error("can't write to %s: %m", path) ;
      return 1 ;
    }
  }
  else
  {
    log_error("can't open file '%s' to write: %m", path) ;
    return 1 ;
  }
  return 0 ;
}

int queue()
{
  Maemo::Timed::Interface timed ;
  Q_Map_String_String attr ;
  QDBusReply< QList<uint> > r1 = timed.get_cookies_by_attributes_sync(attr) ;
  if (not r1.isValid())
  {
    log_error("get_cookies_by_attributes failed: %s", r1.error().message().toStdString().c_str()) ;
    return 1 ;
  }
  QList<uint> cookies = r1.value() ;
  QDBusReply< QMap< uint,QMap<QString,QString> > > r2 = timed.get_attributes_by_cookies_sync(cookies) ;
  if (not r2.isValid())
  {
    log_error("get_attributes_by_cookies failed: %s", r2.error().message().toStdString().c_str()) ;
    return 1 ;
  }
  QMap< uint,QMap<QString,QString> > attr_res = r2.value() ;
  qDebug() << "queue:" << attr_res ;
  return 0 ;
}

static string to_upper_cleaned(const char *s)
{
  string res ;
  for(const char *p=s; p and *p; ++p)
  {
    int ch = *p ;
    if (not isalpha(ch))
      continue ;
    ch = tolower(ch) ;
    res += ch ;
  }
  return res ;
}

static bool execute(const string &command)
{
  const char *cmd = command.c_str() ;

  log_notice("executing '%s'", cmd) ;
  int res = system(cmd) ;

  if (res == 0)
  {
    log_notice("command   '%s' successfully executed", cmd) ;
    return true ;
  }

  if(res<0)
    log_error("execution '%s' failed: %m", cmd) ;
  else if(WIFSIGNALED(res))
    log_error("command '%s' killed by signal %d", cmd, WTERMSIG(res)) ;
  else if(WIFEXITED(res))
    log_error("command '%s' failed, exit status: %d", cmd, WEXITSTATUS(res)) ;
  else
    log_error("command '%s' failed in some weird way, system() call returned %d", cmd, res) ;

  return false ;
}

int mode(int ac, char **av)
{
  if (ac<=0)
  {
    log_error("mode reporting not implemented yet") ;
    return 1 ;
  }
  string mode = to_upper_cleaned(av[0]) ;
  if (mode=="user")
  {
    bool res1 = execute ("rm -f /tmp/ACT_DEAD ; touch /tmp/USER ; echo USER > /tmp/STATE") ;
    bool res2 = execute ("dbus-send --system --dest=com.nokia.time /com/nokia/startup/signal com.nokia.startup.signal.desktop_visible") ;
    return res1 and res2 ? 0 : 1 ;
  }
  else if (mode=="actdead")
  {
    bool res1 = execute ("rm -f /tmp/USER ; touch /tmp/ACT_DEAD ; echo ACT_DEAD > /tmp/STATE") ;
    bool res2 = execute ("dbus-send --system --dest=com.nokia.time /com/nokia/startup/signal com.nokia.startup.signal.init_done int32:5") ;
    return res1 and res2 ? 0 : 1 ;
  }
  else
  {
    log_error("unknown mode: %s", av[0]) ;
    return 1 ;
  }
}
