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
#if ! __MEEGO__
#define USE_CELLULAR_QT 1
#endif

#ifndef TICKER_H
#define TICKER_H

#include <iostream>
#include <sstream>
#include <string>
using namespace std ;

#include <QObject>
#include <QDebug>
#include <QDBusReply>
#include <QTimer>
#include <QDateTime>
#include <QCoreApplication>

#if USE_CELLULAR_QT
#define NEW_CELLULAR 1
#define NEW_CELLULAR_BROKEN_SIGNAL 1
#include <NetworkTime>
#endif

#include <qmlog>

#include "timed/interface"
#include "timed/wallclock"

#if NEW_CELLULAR && NEW_CELLULAR_BROKEN_SIGNAL
using Cellular::NetworkTimeInfo ;
#endif

static string str_printf(const char *format, ...)
{
  const int buf_len = 1024, max_buf_len = buf_len*1024 ;
  char buf[buf_len], *p = buf ;
  va_list varg ;
  va_start(varg, format) ;
  int iteration = 0, printed = false ;
  string formatted ;
  do
  {
    int size = buf_len << iteration ;
    if(size>max_buf_len)
    {
      log_error("Can't format string, the result is too long") ;
      return format ;
    }
    if(iteration>0)
      p = new char[size] ;
    int res = vsnprintf(p, size, format, varg) ;
    if(res < 0)
    {
      log_error("Can't format string, vsnprintf() failed") ;
      return format ;
    }
    if(res < size)
    {
      printed = true ;
      formatted = p ;
    }
    if(iteration > 0)
      delete[] p ;
    ++ iteration ;
  } while(!printed) ;

  return formatted ;
}
static string csd_network_time_info_to_string(const Cellular::NetworkTimeInfo &nti)
{
  if (not nti.isValid())
    return "{invalid}" ;

  ostringstream os ;

  os << "{zone=" << nti.offsetFromUtc() ;

  QDateTime t = nti.dateTime() ;
  if (t.isValid())
  {
    string utc = str_printf("%04d-%02d-%02d,%02d:%02d:%02d", t.date().year(), t.date().month(), t.date().day(), t.time().hour(), t.time().minute(), t.time().second())  ;
    os << ", utc=" << utc ;
  }

  int dst = nti.daylightAdjustment() ;
  if (dst!=-1)
    os << ", dst=" << dst ;

  os << ", mcc='" << nti.mcc().toStdString() << "'" ;
  os << ", mnc='" << nti.mnc().toStdString() << "'" ;

  os << ", received=" << str_printf("%lld.%09lu", (long long)nti.timestamp()->tv_sec, nti.timestamp()->tv_nsec) ;

  os << "}" ;

  return os.str() ;
}


class ticker : public QCoreApplication
{
  Q_OBJECT ;
  Maemo::Timed::Interface *timed ;
  QString abbreviation ;
  QTimer *ticker_timer ;
#if USE_CELLULAR_QT
  Cellular::NetworkTime *cellular_time ;
#endif
public:
  ticker(int ac, char **av) : QCoreApplication(ac,av), abbreviation("[N/A]")
  {
    // set up logging
    // qmlog::syslog()->reduce_max_level(qmlog::Warning) ;
    qmlog::enable() ;
    qmlog::stderr()->reduce_max_level(qmlog::Notice) ;
    qmlog::log_file *logfile = new qmlog::log_file("/ticker.log", qmlog::Debug) ;
    logfile->enable_fields(qmlog::Monotonic_Nano | qmlog::Time_Micro) ;
    logfile->disable_fields(qmlog::Multiline|qmlog::Line|qmlog::Function) ;

    timed = new Maemo::Timed::Interface ;

    QDBusReply<Maemo::Timed::WallClock::Info> x = timed->get_wall_clock_info_sync() ;
    if(x.isValid())
    {
      log_notice("Timed clock settings: %s", x.value().str().toStdString().c_str()) ;
      abbreviation = x.value().tzAbbreviation() ;
    }
    else
      log_warning("D-Bus call timed->get_wall_clock_info_sync() failed: %s", timed->lastError().message().toStdString().c_str()) ;

    bool a = timed->settings_changed_connect(this, SLOT(settings(const Maemo::Timed::WallClock::Info &, bool))) ;
    if(a)
      log_notice("connected to D-Bus signal, waiting for time settings change signal") ;
    else
      log_critical("not connected to D-Bus signal, no time change signal will be delivered!") ;

    QDBusConnection dsme_bus = QDBusConnection::systemBus() ;
    QString path = Maemo::Timed::objpath() ;
    QString iface = Maemo::Timed::interface() ;
    QString signal = "next_bootup_event" ;
    bool aa = dsme_bus.connect("", path, iface, signal, this, SLOT(dsme(int))) ;
    if(aa)
      log_notice("connected to system bus signal '%s'", signal.toStdString().c_str()) ;
    else
      log_critical("not connected to system bus signal '%s'",signal.toStdString().c_str()) ;

#if USE_CELLULAR_QT
    cellular_time = new Cellular::NetworkTime ;
#  if NEW_CELLULAR

#    if NEW_CELLULAR_BROKEN_SIGNAL
    int cel1 = QObject::connect(cellular_time, SIGNAL(timeInfoChanged(const NetworkTimeInfo &)), this, SLOT(cellular_changed(const NetworkTimeInfo &))) ;
    int cel2 = QObject::connect(cellular_time, SIGNAL(timeInfoQueryCompleted(const NetworkTimeInfo &)), this, SLOT(cellular_queried(const NetworkTimeInfo &))) ;
#    else
    int cel1 = QObject::connect(cellular_time, SIGNAL(timeInfoChanged(const Cellular::NetworkTimeInfo &)), this, SLOT(cellular_changed(const Cellular::NetworkTimeInfo &))) ;
    int cel2 = QObject::connect(cellular_time, SIGNAL(timeInfoQueryCompleted(const Cellular::NetworkTimeInfo &)), this, SLOT(cellular_queried(const Cellular::NetworkTimeInfo &))) ;
#    endif

    if (cel1)
      log_notice("connected to 'timeInfoChanged' cellular signal") ;
    else
      log_warning("can't connect to 'timeInfoChanged' cellular signal") ;
    if (cel2)
      log_notice("connected to 'timeInfoQueryCompleted' cellular signal") ;
    else
      log_warning("can't connect to 'timeInfoQueryCompleted' cellular signal") ;

    cellular_time->queryTimeInfo() ;
#  else

    int cel = QObject::connect(cellular_time, SIGNAL(dateTimeChanged(QDateTime, int, int)), this, SLOT(cellular_changed(QDateTime, int, int))) ;
    qDebug() << "connection to old cellular signal:" << (cel ? "ok" : "failed" ) ;
    qDebug() << "current values:" << cellular_time->isValid() << cellular_time->dateTime() << cellular_time->timezone() << cellular_time->dst() ;
#  endif // NEW_CELLULAR
#endif

    ticker_timer = new QTimer ;
    QObject::connect(ticker_timer, SIGNAL(timeout()), this, SLOT(tick())) ;
    ticker_timer->start(1000) ;
  }
#if NEW_CELLULAR
  void print_cellular_info(const char *signal, const Cellular::NetworkTimeInfo &nti)
  {
    cout << endl ;
    log_notice("Signal '%s' received: %s", signal, csd_network_time_info_to_string(nti).c_str()) ;
  }
#endif // NEW_CELLULAR
public slots:
  void tick()
  {
    static int last_len = 0 ;
    ostringstream os ;
    QDateTime dt = QDateTime::currentDateTime () ;
    os << "* " << dt.toString().toStdString().c_str() << " (" << abbreviation.toStdString().c_str() << ")" ;
    struct tm tm ;
    time_t now = time(NULL) ;
    localtime_r(&now, &tm) ;
    os << str_printf(" [%04d-%02d-%02d %02d:%02d:%02d] ", (1900+tm.tm_year), (1+tm.tm_mon), tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) ;
    os << "(isdst=" << tm.tm_isdst << ", gmtoff=" << tm.tm_gmtoff << ", zone='" << tm.tm_zone << "')" ;
    string s = os.str() ;
    int len = s.length() ;
    string spaces = "" ;
    for(int i=len; i<last_len; ++i)
      spaces += " " ;
    cout << s << spaces ;
    cout.flush() ;
    last_len = len ;
    len += spaces.length() ;
    for(int i=0; i<len; ++i)
      cout << (char)8 ; // ^H
    cout.flush() ;
    log_info("*** tick ***") ;
  }
  void settings(const Maemo::Timed::WallClock::Info &info, bool time)
  {
    abbreviation = info.tzAbbreviation() ;
    cout << endl ;
    log_notice("Settings change signalled (system time %schanged), new settings: %s", time ? "" : "not ", info.str().toStdString().c_str()) ;
  }
  void dsme(int value)
  {
    ostringstream os ;
    os << "Signal for dsme detected: value=" << value ;
    if(value>1)
    {
      time_t now = time(NULL) ;
      os << " (now=" << now << " delta=value-now=" << value-now << ")" ;
    }
    cout << endl ;
    log_notice("%s", os.str().c_str()) ;
  }
#if NEW_CELLULAR

#if NEW_CELLULAR_BROKEN_SIGNAL
  void cellular_changed(const NetworkTimeInfo &nti)
#else
  void cellular_changed(const Cellular::NetworkTimeInfo &nti)
#endif
  {
    print_cellular_info("timeInfoChanged", nti) ;
  }
#if NEW_CELLULAR_BROKEN_SIGNAL
  void cellular_queried(const NetworkTimeInfo &nti)
#else
  void cellular_queried(const Cellular::NetworkTimeInfo &nti)
#endif
  {
    print_cellular_info("timeInfoQueryCompleted", nti) ;
  }
#else
  void cellular_changed(QDateTime dt, int timezone, int dst)
  {
    cout << endl ;
    qDebug() << "Old Cellular Signal detected" << "datetime" << dt << "timezone" << timezone << "dst" << dst ;
    cout << endl ;
  }
#endif // NEW_CELLULAR
} ;

#endif
