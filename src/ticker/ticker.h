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

#include "timed/interface"
#include "timed/wallclock"

#if NEW_CELLULAR && NEW_CELLULAR_BROKEN_SIGNAL
using Cellular::NetworkTimeInfo ;
#endif

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
    timed = new Maemo::Timed::Interface ;

    QDBusReply<Maemo::Timed::WallClock::Info> x = timed->get_wall_clock_info_sync() ;
    if(x.isValid())
    {
      cout << "Timed clock settings:\n" << x.value().str().toStdString().c_str() << endl ;
      abbreviation = x.value().tzAbbreviation() ;
    }
    else
      qWarning() << "D-Bus call timed->get_wall_clock_info_sync() failed: " << timed->lastError().message() ;

    bool a = timed->settings_changed_connect(this, SLOT(settings(const Maemo::Timed::WallClock::Info &, bool))) ;
    if(a)
      cout << "connected to D-Bus signal, waiting for time settings change signal" << endl ;
    else
      qCritical() << "not connected to D-Bus signal, no time change signal will be delivered!" ;

    QDBusConnection dsme_bus = QDBusConnection::systemBus() ;
    QString path = Maemo::Timed::objpath() ;
    QString iface = Maemo::Timed::interface() ;
    QString signal = "next_bootup_event" ;
    bool aa = dsme_bus.connect("", path, iface, signal, this, SLOT(dsme(int))) ;
    if(aa)
      cout << "connected to system bus signal '" << signal.toStdString() << "'" << endl ;
    else
      qCritical() << "not connected to system bus signal" << signal ;

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

    qDebug() << "connection to timeInfoChanged:" << (cel1 ? "ok" : "failed" ) ;
    qDebug() << "connection to timeInfoQueryCompleted:" << (cel2 ? "ok" : "failed" ) ;
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
    qDebug() << "Signal" << signal << "received:" << nti ;
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
    os << " [" << (1900+tm.tm_year) << "-" << (1+tm.tm_mon) << "-" << tm.tm_mday <<
      " " << tm.tm_hour << ":" << tm.tm_min << ":" << tm.tm_sec << "]" << " "  ;
    os << "(isdst=" << tm.tm_isdst << ", gmtoff=" << tm.tm_gmtoff << ", zone='" << tm.tm_zone
      << "')" ;
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
  }
  void settings(const Maemo::Timed::WallClock::Info &info, bool time)
  {
    abbreviation = info.tzAbbreviation() ;
    cout << endl ;
    cout << "Settings change signalled (system time " << (time ? "" : "not ") << "changed), new settings:" << endl ;
    cout << info.str().toStdString().c_str() << endl ;
  }
  void dsme(int value)
  {
    cout << endl ;
    cout << "Signal for dsme detected: value=" << value ;
    if(value>1)
    {
      time_t now = time(NULL) ;
      cout << " (now=" << now << " delta=value-now=" << value-now << ")" ;
    }
    cout << endl ;
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
