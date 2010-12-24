#include "interfaces.h"

DsmeReqInterface::DsmeReqInterface(QObject *parent)
  : QDBusAbstractInterface(dsme_service, dsme_req_path, dsme_req_interface, QDBusConnection::systemBus(), parent)
{
}
