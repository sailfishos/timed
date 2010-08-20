/***************************************************************************
**                                                                        **
**   Copyright (C) 2009-2010 Nokia Corporation.                           **
**                                                                        **
**   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    **
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
#include <timed/event.h>
#include <timed/action.h>
#include <timed/exception.h>

#include "flags.h"

void Maemo::Timed::EventAction::setAttribute(const QString &key, const QString &value)
{
  // TODO: check key and value ain't empty
  attr[key] = value ;
}
void Maemo::Timed::EventAction::runCommand()
{
  flags |= ActionFlags::Run_Command ;
}
void Maemo::Timed::EventAction::runCommand(const QString &cmd)
{
  runCommand() ;
  setAttribute("COMMAND", cmd) ;
}
void Maemo::Timed::EventAction::runCommand(const QString &cmd, const QString &user)
{
  runCommand(cmd) ;
  setAttribute("USER", user) ;
}
void Maemo::Timed::EventAction::dbusMethodCall()
{
  flags |= ActionFlags::DBus_Method ;
}
void Maemo::Timed::EventAction::dbusMethodCall(const QString &srv, const QString &mtd, const QString &obj, const QString &ifc)
{
  dbusMethodCall() ;
  setAttribute("DBUS_SERVICE", srv) ;
  setAttribute("DBUS_METHOD", mtd) ;
  setAttribute("DBUS_PATH", obj) ;
  if(!ifc.isEmpty())
    setAttribute("DBUS_INTERFACE", ifc) ;
}
void Maemo::Timed::EventAction::dbusSignal()
{
  flags |= ActionFlags::DBus_Signal ;
}
void Maemo::Timed::EventAction::dbusSignal(QString obj, QString sig, QString ifc)
{
  dbusSignal() ;
  setAttribute("DBUS_PATH", obj) ;
  setAttribute("DBUS_SIGNAL", sig) ;
  setAttribute("DBUS_INTERFACE", ifc) ;
}
void Maemo::Timed::EventAction::setSendCookieFlag()
{
  flags |= ActionFlags::Send_Cookie ;
}
void Maemo::Timed::EventAction::setSendAttributesFlag()
{
  flags |= ActionFlags::Send_Action_Attributes ;
}
void Maemo::Timed::EventAction::setSendEventAttributesFlag()
{
  flags |= ActionFlags::Send_Event_Attributes ;
}
void Maemo::Timed::EventAction::setUseSystemBusFlag()
{
  flags |= ActionFlags::Use_System_Bus ;
}
void Maemo::Timed::EventAction::whenQueued()
{
  flags |= ActionFlags::State_Queued ;
}
void Maemo::Timed::EventAction::whenDue()
{
  flags |= ActionFlags::State_Due ;
}
void Maemo::Timed::EventAction::whenMissed()
{
  flags |= ActionFlags::State_Missed ;
}
void Maemo::Timed::EventAction::whenTriggered()
{
  flags |= ActionFlags::State_Triggered ;
}
void Maemo::Timed::EventAction::whenSnoozed()
{
  flags |= ActionFlags::State_Snoozed ;
}
void Maemo::Timed::EventAction::whenServed()
{
  flags |= ActionFlags::State_Served ;
}
void Maemo::Timed::EventAction::whenAborted()
{
  flags |= ActionFlags::State_Aborted ;
}
void Maemo::Timed::EventAction::whenFailed()
{
  flags |= ActionFlags::State_Failed ;
}
void Maemo::Timed::EventAction::whenFinalized()
{
  flags |= ActionFlags::State_Finalized ;
}
void Maemo::Timed::EventAction::whenButton(const EventButton &x)
{
  int xn = owner->which_button(x) ;
  if(xn<0)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  else
    flags |= ActionFlags::State_App_Button_1 << xn ;
}
void Maemo::Timed::EventAction::whenSysButton(int x)
{
  if(x<0 || x>Maemo::Timed::Number_of_Sys_Buttons)
    throw Exception(__PRETTY_FUNCTION__, "invalid argument") ;
  flags |= ActionFlags::State_Sys_Button_0 << x ;
}
