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
#ifndef MAEMO_TIMED_VOLAND_REMINDER_H
#define MAEMO_TIMED_VOLAND_REMINDER_H

#include <QString>
#include <QDBusMetaType>

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <timed-qt5/qmacro.h>
#else
#include <timed/qmacro.h>
#endif

namespace Maemo { namespace Timed { namespace Voland { class Reminder ; } } }

declare_qtdbus_io(Maemo::Timed::Voland::Reminder) ;

namespace Maemo
{
  namespace Timed
  {
    namespace Voland
    {
      class Reminder
      {
        struct reminder_pimple_t *p ;
        declare_qtdbus_io_friends(Reminder) ;
      public:
        Reminder() ;
        Reminder(reminder_pimple_t *) ;
        Reminder(const Reminder &) ;
        Reminder & operator=(const Reminder &) ;
        unsigned cookie() const ;
        QString attr(const QString &key) const ;
        const QMap<QString,QString> & attributes() const ;
        unsigned buttonAmount() const ;
        bool suppressTimeoutSnooze() const ;
        bool hideSnoozeButton1() const ;
        bool hideCancelButton2() const ;
        bool isMissed() const ;
        QString buttonAttr(int x, const QString &key) const ;
       ~Reminder() ;
      } ;
    }
  }
}

#endif
