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
#ifndef FAKE_DIALOG_UI_H
#define FAKE_DIALOG_UI_H

#include <vector>
#include <sstream>
using namespace std ;

#include <QCoreApplication>
#include <QDebug>
#include <QDBusAbstractInterface>
#include <QDBusReply>
#include <QDBusAbstractAdaptor>
#include <QDBusArgument>
#include <QTimer>

#include <qmlog>

#include <timed/interface>
#include <timed-voland/reminder>
#include <timed-voland/interface>

class fake_dialog_ui_adaptor ;
class fake_dialog_ui_control ;

#define QC toStdString().c_str()

struct fake_dialog_ui : public QCoreApplication
{
  static const int idle_threshold = 10 ;
  QTimer *idle ;
  QObject *activator, *control ;
  QDBusConnection activation_bus ;
  vector<Maemo::Timed::Voland::Reminder> D ;
  fake_dialog_ui(int ac, char **av) ;

  string dialog_windows()
  {
    ostringstream os ;
    if(D.size()==0)
      os << "(empty)" ;
    else
      for(unsigned i=0; i<D.size(); ++i)
      {
        os << "[" << D[i].cookie() << ":" ;
        QString title = D[i].attr("TITLE") ;
        if(!title.isEmpty())
          os << title.QC << ":" ;
        if(!D[i].suppressTimeoutSnooze())
          os << "T" ;
        if(!D[i].hideSnoozeButton1())
          os << "S" ;
        if(!D[i].hideCancelButton2())
          os << "C" ;
        for(unsigned j=1; j<D[i].buttonAmount(); ++j)
          os << j ; // will be very funny for j>9
        os << "]" ;
      }
    return os.str() ;
  }

  void log_windows()
  {
    log_info("WINDOWS: %s", dialog_windows().c_str()) ;
  }

  bool open(const Maemo::Timed::Voland::Reminder &d)
  {
    log_info("request to 'open' dialog '%d' ignored: obsolete interface", d.cookie()) ;
    return false ; // will be ignored by timed for now
  }

  bool open(const QList<QVariant> &A)
  {
    if (A.size()==0)
    {
      log_warning("Empty reminder list? Okey...") ;
      return true ;
    }

    bool can_convert = true ;
    for(int i=0; i<A.size() && can_convert; ++i)
    {
      log_info("i=%d, type=%s", i, A[i].typeName()) ;
      if (not A[i].canConvert<QDBusArgument>())
        can_convert = false ;
    }
    if (not can_convert)
    {
      log_error("Invalid QVariant entry in the list found, rejecting the whole list") ;
      return false ;
    }

    QList<Maemo::Timed::Voland::Reminder> R ;
    for(int i=0; i<A.size(); ++i)
    {
      QDBusArgument a = A[i].value<QDBusArgument>() ;
      Maemo::Timed::Voland::Reminder rr ;
      a >> rr ;
      R.push_back(rr) ;
    }

    ostringstream os ;
    for(int i=0; i<R.size(); ++i)
      os << (i?", ":"[") << R[i].cookie() ;
    os << "]" ;

    log_info("Request to open %d reminders: %s", R.size(), os.str().c_str()) ;

    for(int i=0; i<R.size(); ++i)
      open_a_reminder(R[i]) ;

    return true ;
  }

  bool open_a_reminder(const Maemo::Timed::Voland::Reminder &d)
  {
    log_info("opening dialog %d '%s'", d.cookie(), d.attr("TITLE").QC) ;
    bool found = false ;
    for(unsigned i=0; i<D.size(); ++i)
      found = found || D[i].cookie()==d.cookie() ;
    if(found)
      log_warning("dialog %d already opened, ignoring", d.cookie()) ;
    else
      D.push_back(d) ;
    log_windows() ;
    idle->stop() ;
    return true ;
  }

  bool close_window(unsigned cookie)
  {
    for(unsigned i=0; i<D.size(); ++i)
    {
      if(D[i].cookie()!=cookie)
        continue ;
      log_info("closing dialog %d '%s", D[i].cookie(), D[i].attr("TITLE").QC) ;
      D.erase(D.begin()+i) ;
      log_windows() ;
      if(D.size()==0)
        idle->start(idle_threshold*1000) ;
      return true ;
    }
    return false ;
  }

  bool cancel(unsigned cookie)
  {
    log_info("canceling %d", cookie) ;
    return close_window(cookie) ;
  }

  bool send_stuff_back_to_timed(unsigned cookie, signed value)
  {
    for(unsigned i=0; i<D.size(); ++i)
    {
      if(D[i].cookie()!=cookie)
        continue ;
      Maemo::Timed::Interface timed ;
      if(!timed.isValid())
      {
        log_critical("timed interface not valid, can't close dialog, try later again") ;
        return false ;
      }
      QDBusReply<bool> res = timed.dialog_response_sync(cookie, value) ;
      if(!res.isValid() || !res.value())
        log_warning("failed to report human answer to timed, message: '%s'", res.error().message().QC) ;
      close_window(cookie) ;
      return res.isValid() && res.value() ;
    }
    return false ;
  }

  bool response_last(int value)
  {
    int index = D.size() - 1 ;
    if(index<0)
      return false ;
    return send_stuff_back_to_timed(D[index].cookie(), value) ;
  }

  bool response(uint cookie, int value)
  {
    return send_stuff_back_to_timed(cookie, value) ;
  }
  Q_OBJECT ;
private slots:
  void timeout()
  {
    if(isatty(0))
      return ;
    log_info("%d seconds without activities, exitting", idle_threshold) ;
    exit(0) ;
  }
} ;

struct fake_dialog_ui_adaptor : public Maemo::Timed::Voland::AbstractAdaptor
{
  fake_dialog_ui *owner ;

  fake_dialog_ui_adaptor(fake_dialog_ui *o) : Maemo::Timed::Voland::AbstractAdaptor(o), owner(o) { }

  Q_OBJECT ;

public slots:
  bool open(const Maemo::Timed::Voland::Reminder &d)
  {
    return owner->open(d) ;
  }
  bool open(const QList<QVariant> &reminders)
  {
    return owner->open(reminders) ;
  }
  bool close(uint cookie)
  {
    return owner->cancel(cookie) ;
  }
} ;

struct fake_dialog_ui_control : public Maemo::Timed::Voland::TaAbstractAdaptor
{
  fake_dialog_ui *owner ;

  fake_dialog_ui_control(QObject *o, fake_dialog_ui *ow) : TaAbstractAdaptor(o), owner(ow) { }

  Q_OBJECT ;

public slots:
  int pid()
  {
    return getpid() ;
  }
  bool answer(uint cookie, int value)
  {
    return owner->response(cookie, value) ;
  }
  uint top()
  {
    return 239 ;
  }
  void quit()
  {
    qApp->quit() ;
  }
} ;

struct fake_dialog_ui_activator : public Maemo::Timed::Voland::ActivationAbstractAdaptor
{
  fake_dialog_ui_activator(QObject *o) : Maemo::Timed::Voland::ActivationAbstractAdaptor(o) { }
  Q_OBJECT ;

public slots:
  int pid() { log_info("'pid' request via activation service") ; return getpid() ; }
  QString name() { return "fake-dialog-ui" ; }
} ;

#endif
