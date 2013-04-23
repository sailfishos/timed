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
#ifndef MAEMO_TIMED_WALL_DECLARATIONS_H
#define MAEMO_TIMED_WALL_DECLARATIONS_H

#include <sys/time.h>
#include <time.h>
#include <QString>
#include <QVariant>

#include "qmacro.h"

namespace Maemo
{
  namespace Timed
  {
    class Interface ;
    namespace WallClock
    {
      class Info ;
      class Settings ;
      enum UtcSource { UtcManual, UtcNitz, UtcGps, UtcNtp } ;
      enum TimezoneSource { TimezoneManual, TimezoneCellular } ;
      enum OffsetSource { OffsetManual, OffsetNitz } ;
    }
  }
}

declare_qtdbus_io(Maemo::Timed::WallClock::Info) ;

class Maemo::Timed::WallClock::Info
{
  declare_qtdbus_io_friends(Info) ;
  struct wall_info_pimple_t *p ;
public:
  Info() ;
 ~Info() ;
  Info(const Info &) ;
  const Info &operator=(const Info &) ;
  Info(wall_info_pimple_t *p) ;
public:
  bool flagTimeNitz() const ;
  bool flagLocalCellular() const ;
  bool flagAutoDst() const ;
  bool flagFormat24() const ;

  QString etcLocaltime() const ;
  QString humanReadableTz() const ;
  int secondsEastOfGmt() const ;
  QString tzAbbreviation() const ;

  time_t utc() const ;
  enum UtcSource utcSource() const ;
  enum TimezoneSource timezoneSource() const ;
  enum OffsetSource offsetSource() const ;

  bool utcAvailable(enum UtcSource) const ;
  bool timezoneAvailable(enum TimezoneSource) const ;
  bool offsetAvailable(enum OffsetSource) const ;

  time_t utc(enum UtcSource) const ;
  QString timezone(enum TimezoneSource) const ;
  int offset(enum OffsetSource) const ;

  time_t clockDiff() const ;
  struct timespec clockDiffNano() const ;

  bool nitzSupported() const ;
  QString defaultTimezone() const ;

  QString str() const ;
} ;

// struct wall_settings_pimple_t ;

class Maemo::Timed::WallClock::Settings
{
  struct wall_settings_pimple_t *p ;
public:
  Settings() ;
 ~Settings() ;
private:
  Settings(const Settings &) ;
  const Settings &operator=(const Settings &) ;
public:
  void setFlag24(bool format24) ;
  void setTimeNitz() ;
  void setTimeManual() ;
  void setTimeManual(time_t value) ;
  void setOffsetCellular() ;
  void setOffsetManual() ;
  void setOffsetManual(int value) ;
  void setTimezoneCellular() ;
  void setTimezoneCellular(const QString &fallback_zone) ;
  void setTimezoneManual(const QString &zone) ;
  bool check() const ;
  QString str() const ;
private:
  friend class Maemo::Timed::Interface ;
  QVariant dbus_output(const char *) const ;
} ;

#endif
