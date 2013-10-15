/***************************************************************************
**                                                                        **
**  Copyright (C) 2013 Jolla Ltd.                                         **
**  Contact: Petri M. Gerdt <petri.gerdt@jollamobile.com>                 **
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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "androidalarmwrapper.h"
#include "android_alarm.h"
#include "../common/log.h"

static const char android_alarm_path[] = "/dev/alarm";

AndroidAlarmWrapper::AndroidAlarmWrapper()
{
    if((m_android_alarm_fd = open(android_alarm_path, O_RDONLY)) == -1 ) {
        /* Using /dev/alarm is optional; do not complain if it is missing */
        if(errno != ENOENT)
            log_warning("Failed to open %s: %m", android_alarm_path);
    }
}

AndroidAlarmWrapper::~AndroidAlarmWrapper()
{
    if (m_android_alarm_fd != -1) {
        close(m_android_alarm_fd);
        m_android_alarm_fd = -1;
    }
}

int AndroidAlarmWrapper::getTime(struct timespec *ts)
{
    if (m_android_alarm_fd == -1)
        return -1;

    int cmd = ANDROID_ALARM_GET_TIME(ANDROID_ALARM_ELAPSED_REALTIME);
    int res = ioctl(m_android_alarm_fd, cmd, ts);
    if (res < 0)
        log_warning("Failed to get ANDROID_ALARM_ELAPSED_REALTIME %m");

    return res;
}
