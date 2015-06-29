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
#ifndef BACKUP_H
#define BACKUP_H

#include <QObject>
#include <QDBusAbstractAdaptor>

#include "timed.h"

class com_nokia_backupclient : public QDBusAbstractAdaptor
{
  Q_OBJECT ;
  Q_CLASSINFO("D-Bus Interface", "com.nokia.backupclient") ;

public:
  com_nokia_backupclient(QObject *owner) : QDBusAbstractAdaptor(owner) { }

public slots: /* Excerpts from Backup User Guide v0.2 */

  /* 2.1 Backup Start */
  virtual unsigned char backupStarts() = 0 ;
  /* This method will be called just before the actual backup
   * starts. The application can dump all the data in RAM to their
   * persistent storage. If the application wish to exit on getting
   * this call, it can do that after responding DBus calls. (The
   * application which uses temporary backup files can generate the
   * the file on getting this call.)
   */

  //-----------------------------------------------------------//

  /* 2.2 Backup Finished */
  virtual unsigned char backupFinished() = 0 ;
  /*
   * The applications are notified that backup is finished. (The
   * applications which uses the temporary backup files can delete
   * the file on getting this call.)
   */

  //-----------------------------------------------------------//

  /* 2.3 Restore Start */
  virtual unsigned char restoreStarts() = 0 ;
  /* The applications are notified that restore operation is going
   * to start. If the application wish to exit on getting this call,
   * it can do that.
   */
  //-----------------------------------------------------------//

  /* 2.4 Restore Finished */
  virtual unsigned char restoreFinished() = 0 ;
  /* The applications are notified that one restore operation is
   * finished. On getting this method call the application should
   * reload all data from their persistent storage and update the
   * UI. (The applications which uses the temporary backup files can
   * import data from temporary files and delete them after it.)
   *
   * Not quite so easy:
   *
   * In reality device is rebooting, so no need to "update UI".
   * Thus we will try to write event and settings files, and then
   * wait for the reboot -- I.D.
   */
} ;

class backup_t : public com_nokia_backupclient
{
  Q_OBJECT ;
  Timed *timed ;
public:
  backup_t(Timed *daemon, QObject *owner) : com_nokia_backupclient(owner), timed(daemon) { }

public slots:
  unsigned char backupStarts() { clean_up() ; dump_data() ; return 0 ; }
  unsigned char backupFinished() { clean_up() ; return 0 ; }
  unsigned char restoreStarts() { clean_up() ; return 0 ; }
  unsigned char restoreFinished() { disable_triggering() ; read_data() ; clean_up() ; return 0 ; }

private:
  void dump_data() ;
  void read_data() ;
  void disable_triggering() ;
  void clean_up() ;

  bool save_queue_as(const string &path) ;
  bool save_settings_as(const string &path) ;

  bool read_queue_from(const string &path) ;
} ;

#endif
