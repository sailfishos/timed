#ifndef INTERFACES_H
#define INTERFACES_H

#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusAbstractInterface>

#include <dsme/dsme_dbus_if.h>


class DsmeReqInterface : public QDBusAbstractInterface
{
  Q_OBJECT ;
public:
  DsmeReqInterface(QObject *parent=NULL) ;
  
  QDBusMessage get_state_syn(void) { return call(dsme_get_state) ; }
} ;

#endif//INTERFACES_H
