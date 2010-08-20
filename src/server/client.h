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
#error unuser stuff, dlelete me

namespace Alarm
{
  class Event
  {
    class Button ;
    class Action ;
    class Recurrence ;

    enum Action_State
    {
      State_NEW = 0,
      State_TRIGGERED,
      State_BUTTON_FIRST,
      State_BUTTON_LAST = State_BUTTON_1 + N_BUTTONS - 1 
    } ;
    class Action
    {
      Event *owner ;
      unsigned state_mask ;
      Action(Event *event) ;
    public:
      bool link(Button *button) ;
      bool link(enum Action_State state) ;
    } ;
  public:
    Event() ;
   ~Event() ;
    Button *add_button(QString label) ;
    int find_button(Button *) ;
    Action *add_action_exec(QString cmd) ;
    Action *add_action_dbus(QString service, QString path, QString interface, QString method) ;
  private:
    QVector <Button*> buttons ;

}
