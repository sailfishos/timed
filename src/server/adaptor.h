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
#ifndef ADAPTOR_H
#define ADAPTOR_H

#include <sys/types.h>
#include <unistd.h>

#include <sstream>

#include <QList>
#include <QString>
#include <QDBusAbstractAdaptor>
#include <QDBusMessage>

#include "../lib/event-io.h"
#include "../lib/wall-info.h"
#include "../lib/wall-settings.h"

#include "timed.h"
#include "misc.h"
#include "csd.h"
#include "credentials.h"
#include "peer.h"

#include "../lib/interface.h" // TODO: is Maemo::Timed::bus() the correct way?

#define SQC str().toStdString().c_str()
#define QC toStdString().c_str()
#define CC c_str()
#define PEER timed->peer ? timed->peer->info(message.service().toStdString()).c_str() : qPrintable(message.service())

static QDateTime time_t_to_qdatetime(time_t t)
{
  struct tm tm ;
  if(gmtime_r(&t, &tm) != &tm)
    return QDateTime() ;
  return QDateTime(QDate(tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday), QTime(tm.tm_hour,tm.tm_min,tm.tm_sec), Qt::UTC) ;
}

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
  void next_bootup_event(int next_boot_event, int next_non_boot_event);
  void alarm_triggers_changed(Maemo::Timed::Event::Triggers);

public slots:

  bool mode(const QString &mode, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.mode('%s') by %s", mode.QC, PEER) ;
    bool user_mode = mode=="USER", actdead_mode = mode=="ACTDEAD" ;
    if (not user_mode and not actdead_mode)
    {
      log_error("unknown mode '%s' in com.nokia.time.mode", mode.QC) ;
      return false ;
    }
    timed->device_mode_reached(user_mode) ;
    return true ;
  }

  Maemo::Timed::WallClock::Info get_wall_clock_info(const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.get_wall_clock_info() by %s", PEER) ;
    return timed->settings->get_wall_clock_info(nanotime_t()) ;
  }

  bool wall_clock_settings(const Maemo::Timed::WallClock::wall_settings_pimple_t &p, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.wall_clock_settings(%s) by %s", p.SQC, PEER) ;
    // log_debug("%s", string_std_to_q(p.str()).c_str()) ;
    return timed->settings->wall_clock_settings(p) ;
  }

  uint add_event(const Maemo::Timed::event_io_t &x, const QDBusMessage &message)
  {
    log_notice("DBUS::com.nokia.time.add_event(APP='%s') by %s", x.attr.txt["APPLICATION"].QC, PEER) ;
    // TODO: here we're not asking about credentials immediately
    //       because an event could contain empty action set
    //       --> forwarding the QDBusMessage to Timed::add_even
    //       -->                        then to machine::add_event
    return timed->add_event(cookie_t(), x, message).value() ;
  }

  void add_events(const Maemo::Timed::event_list_io_t &lst, const QDBusMessage &message, QList<QVariant> &res)
  {
    log_notice("DBUS::com.nokia.time.add_events([%d]) by %s", lst.ee.size(), PEER) ;
    // TODO: is Maemo::Timed::bus() the correct way?
    //       yes, but:
    //       Let's see, if there is a better way
    timed->add_events(lst, res, message) ;
  }

  void get_event(uint cookie, const QDBusMessage &message, Maemo::Timed::event_io_t &res)
  {
    log_notice("DBUS::com.nokia.time.get_event(cookie=%u) by %s", cookie, PEER) ;
    if(!timed->get_event(cookie_t(cookie), res))
    {
      QDBusMessage errorReply = message.createErrorReply(QDBusError::InvalidArgs, "Cookie is invalid or not found") ;
      Maemo::Timed::bus().send(errorReply) ;
    }
  }

  void get_events(Q_List_uint cookies, const QDBusMessage &message, Maemo::Timed::event_list_io_t &res)
  {
    log_notice("DBUS::com.nokia.time.get_events[%d] by %s", cookies.size(), PEER) ;
    if(!timed->get_events(cookies, res))
    {
      QDBusMessage errorReply = message.createErrorReply(QDBusError::InvalidArgs, "One of cookies is invalid or not found") ;
      Maemo::Timed::bus().send(errorReply) ;
    }
  }

  uint replace_event(const Maemo::Timed::event_io_t &x, uint old, const QDBusMessage &message)
  {
    log_notice("DBUS::com.nokia.time.replace_event(APP='%s', cookie=%u) by %s", x.attr.txt["APPLICATION"].QC, old, PEER) ;
    return timed->add_event(cookie_t(old), x, message).value() ;
  }

  bool dialog_response(uint cookie, int value, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.dialog_response(cookie=%u, value=%d) by %s", cookie, value, PEER) ;
    return timed->dialog_response(cookie_t(cookie), value) ;
  }

  void query(const QMap<QString,QVariant> &words, const QDBusMessage &message, QList<QVariant> &res)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.query(...) by %s", PEER) ;
    timed->am->query(words, res) ;
  }

  void query_attributes(uint cookie, const QDBusMessage &message, QMap<QString,QVariant> &a)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.query_attributes(cookie=%u, ...) by %s", cookie, PEER) ;
    timed->am->get_event_attributes(cookie_t(cookie), a) ;
  }

  void get_attributes_by_cookie(uint cookie, const QDBusMessage &message, Q_Map_String_String &a)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.get_attributes_by_cookie(%d) by %s", cookie, PEER) ;
    timed->am->get_attributes_by_cookie(cookie, a) ;
  }

  void get_attributes_by_cookies(const Q_List_uint &cookies, const QDBusMessage &message, Q_Map_uint_String_String &a)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.get_attributes_by_cookies[%d] by %s", cookies.size(), PEER) ;
    timed->am->get_attributes_by_cookies(cookies, a) ;
  }

  void get_cookies_by_attributes(const Q_Map_String_String &words, const QDBusMessage &message, Q_List_uint &res)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.get_cookies_by_attributes(...) by %s", PEER) ;
    timed->am->get_cookies_by_attributes(words, res) ;
  }

  bool cancel(uint cookie, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.cancel(cookie=%u) by %s", cookie, PEER) ;
    return timed->cancel(cookie_t(cookie)) ;
  }

  void cancel_events(const Q_List_uint &cookies, const QDBusMessage &message, Q_List_uint &failed)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.cancel_events([%d]) by %s", cookies.size(), PEER) ;
    timed->cancel_events(cookies, failed) ;
  }

  QString ping(const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.ping() by %s", PEER) ;
    iodata::record *r = timed->am->save(false) ; // false = not for backup
    std::ostringstream s ;
    s << *r ;
    delete r ;
    return string_std_to_q(s.str()) ;
  }

  QString parse(QString text, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.parse(text='%s') by %s", text.QC, PEER) ;
    return iodata::parse_and_print(text) ;
  }

  int pid(const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.pid() by %s", PEER) ;
    return getpid() ;
  }

  void enable_alarms(bool enable, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.enable_alarms(%s) by %s", enable?"true":"false", PEER) ;
    timed->settings->alarms_are_enabled = enable ;
    timed->alarm_gate(enable) ;
  }

  bool alarms_enabled(const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.alarms_enabled() by %s", PEER) ;
    return timed->settings->alarms_are_enabled ;
  }

  bool set_default_snooze(int value, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.set_default_snooze(value=%d) by %s", value, PEER) ;
    return timed->default_snooze(value)==value ;
  }

  int get_default_snooze(const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.get_default_snooze() by %s", PEER) ;
    return timed->default_snooze(0) ;
  }

  bool set_app_snooze(const QString &name, int value, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.set_app_snooze(name='%s',value=%d) by %s", name.toStdString().c_str(), value, PEER) ;
    return timed->settings->set_app_snooze(name.toStdString(), value)==value ;
  }

  int get_app_snooze(const QString &name, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.get_default_snooze(name='%s') by %s", name.toStdString().c_str(), PEER) ;
    return timed->settings->get_app_snooze(name.toStdString()) ;
  }

  void remove_app_snooze(const QString &name, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.remove_app_snooze(name='%s') by %s", name.toStdString().c_str(), PEER) ;
    timed->settings->remove_app_snooze(name.toStdString()) ;
  }

  void halt(const QString &what, const QDBusMessage &message)
  {
    Q_UNUSED(message);
    log_notice("DBUS::com.nokia.time.halt(what='%s') by %s", what.QC, PEER) ;
    timed->halt(string_q_to_std(what)) ;
  }

#if OFONO
  bool fake_csd_time_signal(const QString &mcc, const QString &mnc, int offset, int time, int dst, int seconds, int nano_seconds)
  {
    log_notice("(fake_csd_time_signal) mcc='%s' mnc='%s' offset=%d time=%d dst=%d seconds=%d nano_seconds=%d", mcc.toStdString().c_str(), mnc.toStdString().c_str(), offset, time, dst, seconds, nano_seconds) ;
    QDateTime qdt = time_t_to_qdatetime((time_t)time) ;
    if (not qdt.isValid())
    {
      log_error("invalid time=%d parameter in in fake_csd_time_signal()", time) ;
      return false ;
    }
    NetworkTimeInfo nti(qdt, dst, offset, seconds, nano_seconds, mnc, mcc);
    log_notice("FAKE_CSD::csd_time_s %s", csd_t::csd_network_time_info_to_string(nti).c_str()) ;
    timed->csd->process_csd_network_time_info(nti) ;
    return true ;
  }

  bool fake_csd_time_signal_now(const QString &mcc, const QString &mnc, int offset, int time, int dst)
  {
    log_notice("(fake_csd_time_signal_now) mcc='%s' mnc='%s' offset=%d time=%d dst=%d", mcc.toStdString().c_str(), mnc.toStdString().c_str(), offset, time, dst) ;
    nanotime_t now = nanotime_t::monotonic_now() ;
    return fake_csd_time_signal(mcc, mnc, offset, time, dst, now.sec(), now.nano()) ;
  }

  bool fake_nitz_signal(int mcc, int offset, int time, int dst)
  {
    log_notice("(fake_nitz_signal) mcc=%d offset=%d time=%d dst=%d", mcc, offset, time, dst) ;
    QDateTime qdt = time_t_to_qdatetime((time_t)time) ;
    if (not qdt.isValid())
    {
      log_error("invalid time=%d parameter in in fake_nitz_signal()", time) ;
      return false ;
    }
    nanotime_t now = nanotime_t::monotonic_now() ;
    QString mcc_s = str_printf("%d", mcc).c_str() ;
    NetworkTimeInfo nti(qdt, dst, offset, now.sec(), now.nano(), "mnc", mcc_s);
    log_notice("FAKE_CSD::csd_time_s %s", csd_t::csd_network_time_info_to_string(nti).c_str()) ;
    timed->csd->process_csd_network_time_info(nti) ;
    return true ;
  }

  bool fake_operator_signal(const QString &mcc, const QString &mnc)
  {
    log_notice("FAKE_CSD::csd_operator_s {mcc='%s', mnc='%s'}", mcc.toStdString().c_str(), mnc.toStdString().c_str()) ;
    timed->csd->process_csd_network_operator(mcc, mnc) ;
    return true ;
  }
#endif // OFONO
} ;

#endif
