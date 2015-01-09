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
#ifndef MAEMO_TIMED_PEER_H
#define MAEMO_TIMED_PEER_H

#include <string>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>

struct peer_t ;
struct peer_entry_t ;

struct peer_t : public QObject
{
  peer_t(bool mode, QObject *parent=NULL) ;
  ~peer_t() ;
  std::string info(const std::string &) ;
private:
  bool enabled ;
} ;

struct peer_entry_t : public QObject
{
  peer_entry_t(const std::string &new_name, QObject *parent=NULL) ;
  virtual ~peer_entry_t() ;

  std::string name ;
  QDBusPendingCallWatcher *watcher ;

  Q_OBJECT ;
private Q_SLOTS:
  void reply_slot(QDBusPendingCallWatcher *w) ;
} ;

#endif//MAEMO_TIMED_PEER_H
