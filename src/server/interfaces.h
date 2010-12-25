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
  DsmeReqInterface(QObject *parent=NULL)
    : QDBusAbstractInterface(dsme_service, dsme_req_path, dsme_req_interface, QDBusConnection::systemBus(), parent)
  { }

  QDBusMessage get_state_sync(void) { return call(dsme_get_state) ; }
} ;

#endif//INTERFACES_H
