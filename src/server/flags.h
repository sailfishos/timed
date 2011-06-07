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
#ifndef FLAGS_H
#define FLAGS_H

#include <cassert>
#include <stdint.h>

namespace Maemo
{
  namespace Timed
  {
    enum
    {
      First_Sys_Button_Bit    = 20,
      Number_of_Sys_Buttons   = 2,
      // +1 because of the System Button #0, which is not a real button at all:
      First_App_Button_Bit    = First_Sys_Button_Bit + Number_of_Sys_Buttons + 1,
      Max_Number_of_App_Buttons = (signed)sizeof(uint32_t)*8-First_App_Button_Bit
    } ;
  }
}

namespace WallInfoFlags
{
  enum
  {
    Time_Nitz = 1<<0,
    Time
  } ;
}

namespace WallOpcode
{
  enum
  {
    Op_Set_Time_Nitz             = 1<<0,
    Op_Set_Time_Manual           = 1<<1,
    Op_Set_Time_Manual_Val       = 1<<2,

    Op_Set_Time_Mask = (Op_Set_Time_Manual_Val<<1)-1,

    Op_Set_Offset_Cellular       = 1<<3,
    Op_Set_Offset_Manual         = 1<<4,
    Op_Set_Offset_Manual_Val     = 1<<5,
    Op_Set_Timezone_Cellular     = 1<<6,
    Op_Set_Timezone_Cellular_Fbk = 1<<7,
    Op_Set_Timezone_Manual       = 1<<8,

    Op_Set_Zone_Mask = (Op_Set_Timezone_Manual<<1)-1-Op_Set_Time_Mask,

    Op_Set_24                    = 1<<9,
    Op_Set_12                    = 1<<10,

    Op_Set_Format_12_24_Mask = (Op_Set_12|Op_Set_24),

    Op_Full_Mask = (1<<11)-1
  } ;
}

    namespace EventFlags
    {
      enum
      {
        Alarm                   = 1<<0,
        Need_Connection         = 1<<1,
        Trigger_If_Missed       = 1<<2,
        Postpone_If_Missed      = 1<<3,
        User_Mode               = 1<<4,
        Aligned_Snooze          = 1<<5,
        Boot                    = 1<<6,
        Reminder                = 1<<7,
        Suppress0               = 1<<8,
        Hide1                   = 1<<9,
        Hide2                   = 1<<10,
        Keep_Alive              = 1<<11,
        Single_Shot             = 1<<12,
        Backup                  = 1<<13,
        Trigger_When_Adjusting  = 1<<14,

        Last_Client_Flag        = 1<<14,
        Client_Mask             = (Last_Client_Flag<<1)-1,

        Snoozing                = Last_Client_Flag << 1,
        Recurring               = Last_Client_Flag << 2,
        Empty_Recurring         = Last_Client_Flag << 3,
        In_Dialog               = Last_Client_Flag << 4,
        Missed                  = Last_Client_Flag << 5,

        Last_Internal_Flag      = Last_Client_Flag << 5,

        Cluster_Queue           = Last_Internal_Flag << 1,
        Cluster_Dialog          = Last_Internal_Flag << 2,

        Last_Cluster_Flag       = Last_Internal_Flag << 2,
        Cluster_Mask            = (Last_Cluster_Flag - Last_Internal_Flag)<<1,

        Voland_Mask             = (Suppress0|Hide1|Hide2|Missed)

      } ;
    }

    namespace RecurrenceFlags
    {
      enum
      {
        Fill_Gaps               = 1<<0
      } ;
    }

    namespace ActionFlags
    {
      enum
      {
        Send_Cookie             = 1<<0,
        Send_Event_Attributes   = 1<<1,
        Send_Action_Attributes  = 1<<2,
        Run_Command             = 1<<3,
        DBus_Method             = 1<<4,
        DBus_Signal             = 1<<5,
        DBus_Action             = DBus_Method|DBus_Signal,
        Use_System_Bus          = 1<<6,

        State_Triggered         = 1<<10,
        State_Queued            = 1<<11,
        State_Missed            = 1<<12,
        State_Finalized         = 1<<13,
        State_Due               = 1<<14,
        State_Snoozed           = 1<<15,
        State_Served            = 1<<16,
        State_Aborted           = 1<<17,
        State_Failed            = 1<<18,
        State_Tranquil          = 1<<19,

        State_Sys_Button_0      = 1<<Maemo::Timed::First_Sys_Button_Bit,
        State_App_Button_1      = 1<<Maemo::Timed::First_App_Button_Bit
      } ;

      inline int app_button(int no)
      {
        assert(0<no && no<=Maemo::Timed::Max_Number_of_App_Buttons) ; // TODO: change it to log_assert(...)
        return 1<<(no+Maemo::Timed::First_App_Button_Bit-1) ;
      }

      inline int sys_button(int no)
      {
        assert(0<=no && no<=Maemo::Timed::Number_of_Sys_Buttons) ; // TODO: the same as above
        return 1<<(no+Maemo::Timed::First_Sys_Button_Bit) ;
      }
    }


namespace RenameMeNameSpace
{
    enum
    {
      Missing_Threshold = 59,
      Bootup_Length = 60,
      Dsme_Poweroff_Threshold = 120
    } ;
}

#endif
