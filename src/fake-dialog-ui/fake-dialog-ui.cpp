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
#include <iostream>
using namespace std ;

#include <QDBusConnection>
#include <QDebug>

#include <qm/log>

#include "timed-voland/interface"

#include "fake-dialog-ui.h"

fake_dialog_ui::fake_dialog_ui(int ac, char **av)
  : QCoreApplication(ac, av),
    activation_bus("")
{
  idle = new QTimer() ;
  idle->setSingleShot(true) ;
  QObject::connect(idle, SIGNAL(timeout()), this, SLOT(timeout())) ;
  idle->start(idle_threshold*1000) ;
  using namespace Maemo::Timed ;
  new fake_dialog_ui_adaptor(this) ;

  bool service_result = Voland::bus().registerService(Voland::service()) ;
  if(!service_result)
  {
    log_critical("can't register service '%s' on bus '%s': %s", Voland::service(), Voland::bus().name().QC, Voland::bus().lastError().message().QC) ;
    exit(1) ;
  }
  bool obj_result = Maemo::Timed::Voland::bus().registerObject(Maemo::Timed::Voland::objpath(), this) ;
  if(!obj_result)
  {
    log_critical("can't register object '%s' on bus '%s': %s", Voland::objpath(), Voland::bus().name().QC, Voland::bus().lastError().message().QC) ;
    exit(1) ;
  }

  activator = new QObject ;
  new fake_dialog_ui_activator(activator) ;
  activation_bus = QDBusConnection::connectToBus(QDBusConnection::ActivationBus, "activation_bus") ;
  bool reg_res = activation_bus.registerService(Voland::activation_service()) ;
  if(!reg_res)
  {
    log_critical("can't register service '%s' on bus '%s': %s", Voland::activation_service(), activation_bus.name().QC, activation_bus.lastError().message().QC) ;
    exit(1) ;
  }
  bool obj_res = activation_bus.registerObject(Maemo::Timed::Voland::activation_objpath(), activator) ;
  if(!obj_res)
  {
    log_critical("can't register object '%s' on bus '%s': %s", Voland::activation_objpath(), activation_bus.name().QC, activation_bus.lastError().message().QC) ;
    exit(1) ;
  }

  control = new QObject ;
  new fake_dialog_ui_control(control, this) ;

  bool obj3_result = Maemo::Timed::Voland::bus().registerObject(Maemo::Timed::Voland::ta_objpath(), control) ;
  if(!obj3_result)
  {
    log_critical("can't register object '%s' on bus '%s': %s", Voland::ta_objpath(), Voland::bus().name().QC, Voland::bus().lastError().message().QC) ;
    exit(1) ;
  }
}

int main(int ac, char **av)
{
  log_init("fake-dialog-ui", "/tmp/fake-dialog-ui.log", true, true) ;
  log_info("started") ;
  fake_dialog_ui *ui = new fake_dialog_ui(ac, av) ;
  int res = ui->exec() ;
  if(res>0)
    log_error("terminating with error code=%d", res) ;
  log_info("see you soon again, bye") ;
  return res ;
}
