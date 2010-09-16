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
#ifndef ADAPTOR_H
#define ADAPTOR_H

#include <sys/types.h>
#include <unistd.h>

#include <sstream>

#include <QList>
#include <QString>
#include <QDBusAbstractAdaptor>
#include <QDBusMessage>

#include "timed/event-io.h"
#include "timed/wall-info.h"
#include "timed/wall-settings.h"

#include "timed.h"
#include "misc.h"
#include "credentials.h"

#include <timed/interface> // TODO: is Maemo::Timed::bus() the correct way?

/*
 * xxx
 * The stupid and simple backup dbus interface
 */
class com_nokia_backupclient : public QDBusAbstractAdaptor
{
  Q_OBJECT ;
  Q_CLASSINFO("D-Bus Interface", "com.nokia.backupclient") ;
  Timed *timed ;

public:
  com_nokia_backupclient(Timed *parent) : QDBusAbstractAdaptor(parent), timed(parent)
  {
  }

public slots:
  unsigned char backupStarts()
  {
    log_debug("backupStarts");
    timed->backup();
    return 0;
  }

  unsigned char backupFinished()
  {
    log_debug("backupFinished");
    timed->backup_finished();
    return 0;
  }

  unsigned char restoreStarts()
  {
    log_debug("restoreStarts");
    timed->restore();
    return 0;
  }

  unsigned char restoreFinished()
  {
    log_debug("restoreFinished");
    timed->restore_finished();
    return 0;
  }
};

class com_nokia_time : public QDBusAbstractAdaptor
{
  Q_OBJECT ;
  Q_CLASSINFO("D-Bus Interface", "com.nokia.time") ;
  Timed *timed ;

public:
  com_nokia_time(Timed *parent) : QDBusAbstractAdaptor(parent), timed(parent)
  {
    setAutoRelaySignals(true) ;
  }
  // void emit_settings(Maemo::Timed::WallClock::wall_info_pimple_t *p) { emit settings_changed(p) ; }

signals:
  void settings_changed(const Maemo::Timed::WallClock::Info &, bool) ;
  void settings_changed_1(bool) ;

public slots:
  Maemo::Timed::WallClock::Info get_wall_clock_info()
  {
    log_debug() ;
    return timed->settings->get_wall_clock_info(nanotime_t()) ;
  }

  bool wall_clock_settings(const Maemo::Timed::WallClock::wall_settings_pimple_t &p)
  {
    log_debug() ;
    // log_debug("%s", string_std_to_q(p.str()).c_str()) ;
    return timed->settings->wall_clock_settings(p) ;
  }

#if 0
  uint add_event(const Alarm::event_io_t &x)
  {
    return timed->add_event(x).value() ;
  }
#else
  uint add_event(const Maemo::Timed::event_io_t &x, const QDBusMessage &msg)
  {
    log_debug() ;
    QDBusConnection bus = Maemo::Timed::bus();
    QString credentials = credentials_get_from_dbus(bus, msg);
    // FIXME: remove debug logging later ...
    log_warning("CREDS = '%s'", credentials.toLocal8Bit().constData());
    return timed->add_event(cookie_t(), x, credentials).value() ;
  }
  
  void add_events(const Maemo::Timed::event_list_io_t &lst, const QDBusMessage &msg, QList<QVariant> &res)
  {
    log_debug() ;
    QDBusConnection bus = Maemo::Timed::bus();
    QString credentials = credentials_get_from_dbus(bus, msg);
    // FIXME: remove debug logging later ...
    log_warning("CREDS = '%s'", credentials.toLocal8Bit().constData());
    timed->add_events(lst, res, credentials) ;
  }
  
  uint replace_event(const Maemo::Timed::event_io_t &x, uint old, const QDBusMessage &msg)
  {
    log_debug() ;
    QDBusConnection bus = Maemo::Timed::bus();
    QString credentials = credentials_get_from_dbus(bus, msg);
    // FIXME: remove debug logging later ...
    log_warning("CREDS = '%s'", credentials.toLocal8Bit().constData());
    return timed->add_event(cookie_t(old), x, credentials).value() ;
  }
#endif

  bool dialog_response(uint cookie, int value)
  {
    log_debug() ;
    return timed->dialog_response(cookie_t(cookie), value) ;
  }

  void query(const QMap<QString,QVariant> &words, QList<QVariant> &res)
  {
    log_debug() ;
    timed->am->query(words, res) ;
  }

  void query_attributes(uint cookie, QMap<QString,QVariant> &a)
  {
    log_debug() ;
    timed->am->get_event_attributes(cookie_t(cookie), a) ;
  }

  bool cancel(uint cookie)
  {
    log_debug() ;
    return timed->cancel(cookie_t(cookie)) ;
  }

  QString ping()
  {
    log_debug() ;
    iodata::record *r = timed->am->save() ;
    std::ostringstream s ;
    s << *r ;
    delete r ;
    return string_std_to_q(s.str()) ;
  }

  QString parse(QString text)
  {
    log_debug() ;
    return iodata::parse_and_print(text) ;
  }

  int pid()
  {
    log_debug() ;
    return getpid() ;
  }

  void enable_alarms(bool enable)
  {
    log_debug() ;
    timed->alarm_gate(true, enable) ;
  }

  bool alarms_enabled()
  {
    log_debug() ;
    return timed->alarm_gate(false) ;
  }

  bool set_default_snooze(int value)
  {
    log_debug() ;
    return timed->default_snooze(value)==value ;
  }

  int get_default_snooze()
  {
    log_debug() ;
    return timed->default_snooze(0) ;
  }

  void halt(const QString &what)
  {
    log_info("halt('%s') requested, going to sleep soon...", what.toStdString().c_str()) ;
    timed->halt(string_q_to_std(what)) ;
  }

  QMap<QString, QVariant> customization_values()
  {
    log_debug() ;
    return timed->cust_settings->get_values();
  }

  bool fake_nitz_signal(int mcc, int offset, int time, int dst)
  {
    cellular_handler::object()->fake_nitz_signal(mcc, offset, time, dst) ;
    return true ; // TODO make above method returning bool (not void) and check parameters
  }
#if 0
  uint32_t update_event(uint32_t c, const Alarm::event_t &a)
  {
    return timed->update_event(Alarm::cookie_t(c), a).value() ;
  }

  const Alarm::event_t &get_event(uint32_t c)
  {
    return timed->get_event(Alarm::cookie_t(c)) ;
  }

  bool del_event(uint32_t c)
  {
    return timed->del_event(Alarm::cookie_t(c)) ;
  }

  QVector<int> query_event(int start_time, int stop_time, int flag_mask, int flag_want)
  {
    return timed->query_event(start_time,stop_time, flag_mask, flag_want) ;
  }
  bool set_snooze(uint snooze) { return timed->set_snooze(snooze) ; }
  uint get_snooze() { return timed->get_snooze() ; }

signals:
  void queue_status_ind(int alarms, int desktop, int actdead, int noboot) ;
#endif
} ;


#endif
