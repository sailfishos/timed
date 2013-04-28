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
#ifndef MAEMO_TIMED_VOLAND_INTERFACE_H
#define MAEMO_TIMED_VOLAND_INTERFACE_H

#include <stdint.h>

#include <QList>
#include <QVector>
#include <QDBusAbstractInterface>
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <timed-qt5/qmacro.h>
#else
#include <timed/qmacro.h>
#endif

#include "reminder.h"

namespace Maemo
{
  namespace Timed
  {
    namespace Voland
    {
      inline const char *service()
      {
        return "com.nokia.voland" ;
      }
      inline const char *interface()
      {
        return "com.nokia.voland" ;
      }
      inline const char *objpath()
      {
        return "/com/nokia/voland" ;
      }
      inline QDBusConnection bus()
      {
        return QDBusConnection::systemBus() ;
      }
      class Interface : public QDBusAbstractInterface
      {
        Q_OBJECT ;
      public:
        Interface(QObject *parent=NULL) ;
        Interface(QDBusConnection bus_connection, QObject *parent=NULL) ;
        qtdbus_method(open, (const Maemo::Timed::Voland::Reminder &data), QVariant::fromValue(data)) ;
        qtdbus_method(open, (const QList<QVariant> &data), data) ;
        qtdbus_method(close, (uint32_t cookie), cookie) ;
      } ;
      class AbstractAdaptor : public QDBusAbstractAdaptor
      {
        Q_OBJECT ;
        Q_CLASSINFO("D-Bus Interface", "com.nokia.voland") ; // check: same as interface()
      public:
        AbstractAdaptor(QObject *owner) : QDBusAbstractAdaptor(owner) { }
      public Q_SLOTS:
        virtual bool open(const Maemo::Timed::Voland::Reminder &data) = 0 ;
        virtual bool open(const QList<QVariant> &data) = 0 ; //{ (void)data ; return false ; } ;
        virtual bool close(uint cookie) = 0 ;
      } ;

      // Test Automation

      inline const char *ta_interface()
      {
        return "com.nokia.ta_voland" ;
      }
      inline const char *ta_objpath()
      {
        return "/com/nokia/ta_voland" ;
      }
      class TaInterface : public QDBusAbstractInterface
      {
        Q_OBJECT ;
      public:
        TaInterface(QObject *parent=NULL) ;
        qtdbus_method(pid, (void)) ;
        qtdbus_method(answer, (uint cookie, int button), cookie, button) ;
        qtdbus_method(top, (void)) ;
        qtdbus_method(quit, (void)) ;
      } ;
      class TaAbstractAdaptor : public QDBusAbstractAdaptor
      {
        Q_OBJECT ;
        Q_CLASSINFO("D-Bus Interface", "com.nokia.ta_voland") ; // check: same as ta_interface()
      public:
        TaAbstractAdaptor(QObject *owner) : QDBusAbstractAdaptor(owner) { }
      public Q_SLOTS:
        virtual int pid() = 0 ;
        virtual bool answer(uint cookie, int button) = 0 ;
        virtual uint top() = 0 ;
        virtual void quit() = 0 ;
      } ;

      // D-Bus based activation of Voland service

      inline const char *activation_service()
      {
        return "com.nokia.voland_activation" ;
      }
      inline const char *activation_interface()
      {
        return "com.nokia.voland_activation" ;
      }
      inline const char *activation_objpath()
      {
        return "/com/nokia/voland_activation" ;
      }
      class ActivationInterface : public QDBusAbstractInterface
      {
        Q_OBJECT ;
      public:
        ActivationInterface(QObject *parent=NULL) ;
        qtdbus_method(pid, (void)) ;
        qtdbus_method(name, (void)) ;
      } ;
      class ActivationAbstractAdaptor : public QDBusAbstractAdaptor
      {
        Q_OBJECT ;
        Q_CLASSINFO("D-Bus Interface", "com.nokia.voland_activation") ; // check: same as activation_interface()
      public:
        ActivationAbstractAdaptor(QObject *owner) : QDBusAbstractAdaptor(owner) { }
      public Q_SLOTS:
        virtual int pid() = 0 ;
        virtual QString name() = 0 ;
      } ;
    }
  }
}

#endif
