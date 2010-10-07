#ifndef BACKUP_H
#define BACKUP_H

#include <QObject>
#include <QDBusAbstractAdaptor>

#include "timed.h"

class com_nokia_timed_backup : public QDBusAbstractAdaptor
{
  Q_OBJECT ;
  Q_CLASSINFO("D-Bus Interface", "com.nokia.backupclient") ;
  Timed *timed ;

public:
  com_nokia_backupclient(Timed *daemon, QObject *owner) : QDBusAbstractAdaptor(owner), timed(daemon)
  {
  }

public slots:
  unsigned char backupStarts()
  {
    log_debug();
    timed->backup();
    return 0 ;
  }

  unsigned char backupFinished()
  {
    log_debug();
    timed->backup_finished() ;
    return 0 ;
  }

  unsigned char restoreStarts()
  {
    log_debug() ;
    timed->restore() ;
    return 0 ;
  }

  unsigned char restoreFinished()
  {
    log_debug() ;
    timed->restore_finished() ;
    return 0 ;
  }
} ;

#endif
